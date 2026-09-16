const { sql, ensureSchema } = require('../lib/db');

// Lightweight re-check used while the app is already running (no password
// needed the user already authenticated at login; this just confirms
// the subscription hasn't been cancelled/expired since then).
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email } = req.body || {};
  if (!email) {
    res.status(400).json({ ok: false, error: 'missing_fields' });
    return;
  }

  const normalizedEmail = String(email).trim().toLowerCase();

  try {
    await ensureSchema();

    const result = await sql`
      SELECT plan, expires_at FROM users WHERE email = ${normalizedEmail}
    `;
    if (result.length === 0) {
      res.status(404).json({ ok: false, error: 'user_not_found' });
      return;
    }

    const user = result[0];
    const expiresAt = user.expires_at ? new Date(user.expires_at) : null;
    const active = user.plan !== 'none' && expiresAt !== null && expiresAt.getTime() > Date.now();

    res.status(200).json({
      ok: true,
      active,
      plan: active ? user.plan : 'none',
      expiresAt: expiresAt ? expiresAt.toISOString() : null,
    });
  } catch (err) {
    console.error('status error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
