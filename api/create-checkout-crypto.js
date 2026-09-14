const { createInvoice } = require('../lib/nowpayments');

const SITE_URL = 'https://solverix-nicolastu-devs-projects.vercel.app';
const API_URL = 'https://solverix-api-nicolastu-devs-projects.vercel.app';

module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, plan, ref } = req.body || {};
  if (!email || !plan || (plan !== 'advanced' && plan !== 'complete')) {
    res.status(400).json({ ok: false, error: 'missing_or_invalid_fields' });
    return;
  }

  try {
    const invoice = await createInvoice({
      email: String(email).trim().toLowerCase(),
      plan,
      siteUrl: SITE_URL,
      apiUrl: API_URL,
      refCode: ref ? String(ref).trim().toLowerCase() : null,
    });
    res.status(200).json({ ok: true, checkoutUrl: invoice.invoice_url });
  } catch (err) {
    console.error('create-checkout-crypto error', err);
    res.status(500).json({ ok: false, error: 'nowpayments_error' });
  }
};
