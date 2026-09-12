const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');

module.exports = async (req, res) => {
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, password } = req.body || {};
  if (!email || !password) {
    res.status(400).json({ ok: false, error: 'missing_fields' });
    return;
  }
  if (password.length < 4) {
    res.status(400).json({ ok: false, error: 'password_too_short' });
    return;
  }

  const normalizedEmail = String(email).trim().toLowerCase();

  try {
    await ensureSchema();

    const existing = await sql`SELECT id FROM users WHERE email = ${normalizedEmail}`;
    if (existing.length > 0) {
      res.status(409).json({ ok: false, error: 'email_already_registered' });
      return;
    }

    const passwordHash = await bcrypt.hash(password, 10);
    await sql`
      INSERT INTO users (email, password_hash, plan, expires_at)
      VALUES (${normalizedEmail}, ${passwordHash}, 'none', NULL)
    `;

    res.status(201).json({ ok: true });
  } catch (err) {
    console.error('register error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
