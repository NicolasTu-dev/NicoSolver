const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');
const { affiliateAuthorizeUrl } = require('../lib/mercadopago');

const FOUNDER_EMAIL = 'nicolastu98@hotmail.com';
const REDIRECT_URI = 'https://solverix-api-nicolastu-devs-projects.vercel.app/api/affiliate-oauth-callback';

// The founder account assigns a streamer code to an existing login account
// (targetEmail) directly from the web UI, instead of building an
// affiliate-connect link by hand. Creates the affiliate row if it doesn't
// exist yet (status stays 'pending' until the streamer finishes the
// Mercado Pago OAuth flow) and links it to targetEmail so that account
// shows the "streamer" badge right away.
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, password, targetEmail, code, name } = req.body || {};
  if (!email || !password || !targetEmail || !code || !name) {
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

    const normalizedTarget = String(targetEmail).trim().toLowerCase();
    const targetRow = await sql`SELECT id FROM users WHERE email = ${normalizedTarget}`;
    if (targetRow.length === 0) {
      res.status(404).json({ ok: false, error: 'target_not_found' });
      return;
    }

    const normalizedCode = String(code).trim().toLowerCase();
    const trimmedName = String(name).trim();
    await sql`
      INSERT INTO affiliates (code, name, owner_email)
      VALUES (${normalizedCode}, ${trimmedName}, ${normalizedTarget})
      ON CONFLICT (code) DO UPDATE SET
        name = EXCLUDED.name,
        owner_email = EXCLUDED.owner_email
    `;

    const authorizeUrl = affiliateAuthorizeUrl({ code: normalizedCode, redirectUri: REDIRECT_URI });
    res.status(200).json({ ok: true, code: normalizedCode, authorizeUrl });
  } catch (err) {
    console.error('admin-assign-streamer error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
