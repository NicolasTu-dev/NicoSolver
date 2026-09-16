const crypto = require('crypto');
const { sql, ensureSchema } = require('../lib/db');

// NOWPayments signs the IPN body with HMAC-SHA512 over the JSON-sorted
// payload, using the separate IPN secret key (not the API key) from
// Settings > IPN in the NOWPayments dashboard.
function sortObject(obj){
  if (Array.isArray(obj)) return obj.map(sortObject);
  if (obj && typeof obj === 'object') {
    return Object.keys(obj).sort().reduce((acc, key) => {
      acc[key] = sortObject(obj[key]);
      return acc;
    }, {});
  }
  return obj;
}

// NOWPayments calls this after every status change on a payment. We only
// act once the payment is "finished" (fully confirmed on-chain and
// credited). Always respond 200 so NOWPayments doesn't keep retrying.
module.exports = async (req, res) => {
  try {
    const signature = req.headers['x-nowpayments-sig'];
    const sortedBody = JSON.stringify(sortObject(req.body || {}));
    const expectedSig = crypto
      .createHmac('sha512', process.env.NOWPAYMENTS_IPN_SECRET)
      .update(sortedBody)
      .digest('hex');

    if (!signature || signature !== expectedSig) {
      res.status(401).json({ ok: false, error: 'invalid_signature' });
      return;
    }

    const status = req.body.payment_status;
    if (status !== 'finished' && status !== 'confirmed') {
      res.status(200).json({ ok: true, notFinished: true });
      return;
    }

    const orderId = req.body.order_id || '';
    const [email, plan, refCode] = orderId.split('|');
    if (!email || (plan !== 'advanced' && plan !== 'complete')) {
      res.status(200).json({ ok: true, badOrder: true });
      return;
    }

    await ensureSchema();
    await sql`
      UPDATE users
      SET plan = ${plan}, expires_at = now() + interval '30 days'
      WHERE email = ${email}
    `;

    // Logs the commission owed to the affiliate as 'pending', in USD the
    // founder pays it out by hand later and marks it 'paid' from the
    // founder panel.
    if (refCode && refCode !== 'none') {
      const affiliateRows = await sql`SELECT commission_rate FROM affiliates WHERE code = ${refCode} AND status = 'active'`;
      if (affiliateRows.length > 0) {
        const grossAmount = Number(req.body.price_amount || 0);
        const commissionAmount = Math.round(grossAmount * Number(affiliateRows[0].commission_rate) * 100) / 100;
        await sql`
          INSERT INTO affiliate_commissions (affiliate_code, buyer_email, plan, gross_amount, commission_amount, currency, source, payment_id)
          VALUES (${refCode}, ${email}, ${plan}, ${grossAmount}, ${commissionAmount}, 'USD', 'crypto', ${'np_' + String(req.body.payment_id || orderId)})
          ON CONFLICT (payment_id) DO NOTHING
        `;
      }
    }

    res.status(200).json({ ok: true });
  } catch (err) {
    console.error('nowpayments-webhook error', err);
    res.status(200).json({ ok: false });
  }
};
