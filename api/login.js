const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');

const FOUNDER_EMAIL = 'nicolastu98@hotmail.com';

module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, password } = req.body || {};
  if (!email || !password) {
    res.status(400).json({ ok: false, error: 'missing_fields' });
    return;
  }

  const normalizedEmail = String(email).trim().toLowerCase();

  try {
    await ensureSchema();

    const result = await sql`
      SELECT password_hash, plan, expires_at FROM users WHERE email = ${normalizedEmail}
    `;
    if (result.length === 0) {
      res.status(401).json({ ok: false, error: 'invalid_credentials' });
      return;
    }

    const user = result[0];
    const passwordMatches = await bcrypt.compare(password, user.password_hash);
    if (!passwordMatches) {
      res.status(401).json({ ok: false, error: 'invalid_credentials' });
      return;
    }

    const expiresAt = user.expires_at ? new Date(user.expires_at) : null;
    const active = user.plan !== 'none' && expiresAt !== null && expiresAt.getTime() > Date.now();

    const affiliateRows = await sql`
      SELECT
        a.code, a.name,
        COALESCE(SUM(CASE WHEN c.currency = 'ARS' AND c.status = 'pending' THEN c.commission_amount ELSE 0 END), 0) AS pending_ars,
        COALESCE(SUM(CASE WHEN c.currency = 'ARS' AND c.status = 'paid' THEN c.commission_amount ELSE 0 END), 0) AS paid_ars,
        COALESCE(SUM(CASE WHEN c.currency = 'USD' AND c.status = 'pending' THEN c.commission_amount ELSE 0 END), 0) AS pending_usd,
        COALESCE(SUM(CASE WHEN c.currency = 'USD' AND c.status = 'paid' THEN c.commission_amount ELSE 0 END), 0) AS paid_usd
      FROM affiliates a
      LEFT JOIN affiliate_commissions c ON c.affiliate_code = a.code
      WHERE a.owner_email = ${normalizedEmail}
      GROUP BY a.code, a.name
    `;
    const streamer = affiliateRows.length > 0 ? {
      code: affiliateRows[0].code,
      name: affiliateRows[0].name,
      pendingArs: Number(affiliateRows[0].pending_ars),
      paidArs: Number(affiliateRows[0].paid_ars),
      pendingUsd: Number(affiliateRows[0].pending_usd),
      paidUsd: Number(affiliateRows[0].paid_usd),
    } : null;

    res.status(200).json({
      ok: true,
      active,
      plan: active ? user.plan : 'none',
      expiresAt: expiresAt ? expiresAt.toISOString() : null,
      streamer,
      isFounder: normalizedEmail === FOUNDER_EMAIL,
    });
  } catch (err) {
    console.error('login error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
