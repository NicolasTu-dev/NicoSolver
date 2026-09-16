const { sql, ensureSchema } = require('../lib/db');
const { affiliateAuthorizeUrl } = require('../lib/mercadopago');

const REDIRECT_URI = 'https://solverix-api-nicolastu-devs-projects.vercel.app/api/affiliate-oauth-callback';

// Admin-only (the founder account holds ADMIN_SECRET): run once per new
// affiliate/streamer, e.g.
// /api/affiliate-connect?secret=...&code=fulano&name=Fulano&email=fulano@example.com
// Creates the affiliate row if it doesn't exist yet (status stays 'pending'
// until they finish the Mercado Pago OAuth flow) and returns the link to
// send them. The optional email links this affiliate to a login account
// (users.email) so that account shows a "streamer" badge pass it again
// later to re-link an existing affiliate to a different or corrected email.
module.exports = async (req, res) => {
  if (req.query.secret !== process.env.ADMIN_SECRET) {
    res.status(403).json({ ok: false, error: 'forbidden' });
    return;
  }

  const code = String(req.query.code || '').trim().toLowerCase();
  const name = String(req.query.name || '').trim();
  const ownerEmail = String(req.query.email || '').trim().toLowerCase() || null;
  if (!code || !name) {
    res.status(400).json({ ok: false, error: 'missing_code_or_name' });
    return;
  }

  try {
    await ensureSchema();
    await sql`
      INSERT INTO affiliates (code, name, owner_email)
      VALUES (${code}, ${name}, ${ownerEmail})
      ON CONFLICT (code) DO UPDATE SET
        name = EXCLUDED.name,
        owner_email = COALESCE(EXCLUDED.owner_email, affiliates.owner_email)
    `;

    const authorizeUrl = affiliateAuthorizeUrl({ code, redirectUri: REDIRECT_URI });
    res.status(200).json({ ok: true, code, ownerEmail, authorizeUrl });
  } catch (err) {
    console.error('affiliate-connect error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
