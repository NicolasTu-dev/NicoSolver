const { getPayment } = require('../lib/mercadopago');
const { sql, ensureSchema } = require('../lib/db');

// Mercado Pago calls this after every payment event. We only act on
// "payment" notifications with status "approved". Always respond 200
// quickly so MP doesn't keep retrying.
module.exports = async (req, res) => {
  try {
    const paymentId =
      (req.body && req.body.data && req.body.data.id) ||
      req.query['data.id'] ||
      req.query.id;
    const topic = (req.body && req.body.type) || req.query.topic || req.query.type;

    if (!paymentId || (topic && topic !== 'payment')) {
      res.status(200).json({ ok: true, ignored: true });
      return;
    }

    const payment = await getPayment(paymentId);
    if (!payment || payment.status !== 'approved') {
      res.status(200).json({ ok: true, notApproved: true });
      return;
    }

    const ref = payment.external_reference || '';
    const [email, plan, refCode] = ref.split('|');
    if (!email || (plan !== 'advanced' && plan !== 'complete')) {
      res.status(200).json({ ok: true, badReference: true });
      return;
    }

    await ensureSchema();
    await sql`
      UPDATE users
      SET plan = ${plan}, expires_at = now() + interval '30 days'
      WHERE email = ${email}
    `;

    // Logs the commission owed to the affiliate as 'pending' the founder
    // pays it out by hand later and marks it 'paid' from the founder panel.
    // ON CONFLICT guards against MP retrying the same notification and
    // double-counting.
    if (refCode && refCode !== 'none') {
      const affiliateRows = await sql`SELECT commission_rate FROM affiliates WHERE code = ${refCode} AND status = 'active'`;
      if (affiliateRows.length > 0) {
        const grossAmount = Number(payment.transaction_amount || 0);
        const commissionAmount = Math.round(grossAmount * Number(affiliateRows[0].commission_rate) * 100) / 100;
        await sql`
          INSERT INTO affiliate_commissions (affiliate_code, buyer_email, plan, gross_amount, commission_amount, currency, source, payment_id)
          VALUES (${refCode}, ${email}, ${plan}, ${grossAmount}, ${commissionAmount}, 'ARS', 'mercadopago', ${String(paymentId)})
          ON CONFLICT (payment_id) DO NOTHING
        `;
      }
    }

    res.status(200).json({ ok: true });
  } catch (err) {
    console.error('mp-webhook error', err);
    res.status(200).json({ ok: false }); // still 200 so MP doesn't hammer retries
  }
};
