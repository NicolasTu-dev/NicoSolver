const { sql, ensureSchema } = require('../lib/db');
const { affiliateAuthorizeUrl } = require('../lib/mercadopago');

const REDIRECT_URI = 'https://solverix-api-nicolastu-devs-projects.vercel.app/api/affiliate-oauth-callback';

// Admin-only: run once per new affiliate (e.g.
// /api/affiliate-connect?secret=...&code=fulano&name=Fulano). Creates the
// affiliate row if it doesn't exist yet (status stays 'pending' until they
// finish the Mercado Pago OAuth flow) and returns the link to send them.
module.exports = async (req, res) => {
  if (req.query.secret !== process.env.ADMIN_SECRET) {
    res.status(403).json({ ok: false, error: 'forbidden' });
    return;
  }

  const code = String(req.query.code || '').trim().toLowerCase();
  const name = String(req.query.name || '').trim();
  if (!code || !name) {
    res.status(400).json({ ok: false, error: 'missing_code_or_name' });
    return;
  }

  try {
    await ensureSchema();
    await sql`
      INSERT INTO affiliates (code, name)
      VALUES (${code}, ${name})
      ON CONFLICT (code) DO NOTHING
    `;

    const authorizeUrl = affiliateAuthorizeUrl({ code, redirectUri: REDIRECT_URI });
    res.status(200).json({ ok: true, code, authorizeUrl });
  } catch (err) {
    console.error('affiliate-connect error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
