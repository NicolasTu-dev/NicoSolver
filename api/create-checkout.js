const { createPreference } = require('../lib/mercadopago');
const { sql, ensureSchema } = require('../lib/db');

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
    let affiliate = null;
    let refCode = null;
    if (ref) {
      await ensureSchema();
      const rows = await sql`
        SELECT code, commission_rate, access_token FROM affiliates
        WHERE code = ${String(ref).trim().toLowerCase()} AND status = 'active'
      `;
      if (rows.length > 0 && rows[0].access_token) {
        refCode = rows[0].code;
        affiliate = { accessToken: rows[0].access_token, commissionRate: Number(rows[0].commission_rate) };
      }
    }

    const preference = await createPreference({
      email: String(email).trim().toLowerCase(),
      plan,
      siteUrl: SITE_URL,
      apiUrl: API_URL,
      refCode,
      affiliate,
    });
    res.status(200).json({
      ok: true,
      checkoutUrl: preference.init_point || preference.sandbox_init_point,
    });
  } catch (err) {
    console.error('create-checkout error', err);
    res.status(500).json({ ok: false, error: 'mp_error' });
  }
};
