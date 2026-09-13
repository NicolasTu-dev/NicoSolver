const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');

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

    res.status(200).json({
      ok: true,
      active,
      plan: active ? user.plan : 'none',
      expiresAt: expiresAt ? expiresAt.toISOString() : null,
    });
  } catch (err) {
    console.error('login error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
