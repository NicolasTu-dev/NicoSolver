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
    const [email, plan] = ref.split('|');
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

    res.status(200).json({ ok: true });
  } catch (err) {
    console.error('mp-webhook error', err);
    res.status(200).json({ ok: false }); // still 200 so MP doesn't hammer retries
  }
};
