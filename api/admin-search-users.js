const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');

// The founder account (see FOUNDER_EMAIL) can search the users table by
// partial email match, so they can pick who to assign a streamer code to
// without leaving the account page.
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

  const { email, password, query } = req.body || {};
  if (!email || !password) {
    res.status(400).json({ ok: false, error: 'missing_fields' });
    return;
  }

  const normalizedEmail = String(email).trim().toLowerCase();
  if (normalizedEmail !== FOUNDER_EMAIL) {
    res.status(403).json({ ok: false, error: 'forbidden' });
    return;
  }

  try {
    await ensureSchema();

    const founderRow = await sql`SELECT password_hash FROM users WHERE email = ${normalizedEmail}`;
    if (founderRow.length === 0 || !(await bcrypt.compare(password, founderRow[0].password_hash))) {
      res.status(403).json({ ok: false, error: 'forbidden' });
      return;
    }

    const searchTerm = '%' + String(query || '').trim().toLowerCase() + '%';
    const rows = await sql`
      SELECT u.email, u.plan,
        (SELECT code FROM affiliates a WHERE a.owner_email = u.email) AS streamer_code
      FROM users u
      WHERE u.email ILIKE ${searchTerm}
      ORDER BY u.email
      LIMIT 20
    `;
    res.status(200).json({ ok: true, users: rows });
  } catch (err) {
    console.error('admin-search-users error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
