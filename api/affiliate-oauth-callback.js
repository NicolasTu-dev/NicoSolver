const { sql, ensureSchema } = require('../lib/db');
const { exchangeAffiliateCode } = require('../lib/mercadopago');

const REDIRECT_URI = 'https://solverix-api-nicolastu-devs-projects.vercel.app/api/affiliate-oauth-callback';

// Mercado Pago redirects the affiliate here after they approve the OAuth
// connection, with ?code=...&state=<our affiliate code>. We exchange that
// code for their access/refresh tokens and mark the affiliate 'active'
// from then on their link works for checkout.
module.exports = async (req, res) => {
  const { code: oauthCode, state: affiliateCode } = req.query;
  if (!oauthCode || !affiliateCode) {
    res.status(400).send('Falta el código de autorización.');
    return;
  }

  try {
    const tokenData = await exchangeAffiliateCode({ code: oauthCode, redirectUri: REDIRECT_URI });
    const expiresAt = new Date(Date.now() + Number(tokenData.expires_in || 0) * 1000).toISOString();

    await ensureSchema();
    const result = await sql`
      UPDATE affiliates
      SET status = 'active',
          mp_user_id = ${String(tokenData.user_id || '')},
          access_token = ${tokenData.access_token},
          refresh_token = ${tokenData.refresh_token},
          token_expires_at = ${expiresAt}
      WHERE code = ${affiliateCode}
      RETURNING code
    `;

    if (result.length === 0) {
      res.status(404).send('Ese código de afiliado no existe.');
      return;
    }

    res.status(200).send('¡Listo! Tu cuenta de Mercado Pago quedó conectada a Solverix. Ya podés compartir tu link.');
  } catch (err) {
    console.error('affiliate-oauth-callback error', err);
    res.status(500).send('No se pudo completar la conexión. Avisale a Solverix.');
  }
};
