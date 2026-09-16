const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');
const { affiliateAuthorizeUrl } = require('../lib/mercadopago');

const FOUNDER_EMAIL = 'nicolastu98@hotmail.com';
const REDIRECT_URI = 'https://solverix-api-nicolastu-devs-projects.vercel.app/api/affiliate-oauth-callback';

// Single endpoint for every founder-only action (search users, assign a
// streamer code, grant a subscription) merged into one Serverless
// Function instead of three, since the Vercel Hobby plan caps deployments
// at 12 functions and each admin action used to be its own file.
// Body: { email, password, action, ...action-specific fields }.
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, password, action } = req.body || {};
  if (!email || !password || !action) {
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

    if (action === 'search') {
      const { query } = req.body || {};
      const searchTerm = '%' + String(query || '').trim().toLowerCase() + '%';
      // Matches on the account email OR on the streamer code/name tied to
      // that account, since the founder often only remembers the
      // streamer's alias, not the email they registered with.
      const rows = await sql`
        SELECT u.email, u.plan,
          (SELECT code FROM affiliates a WHERE a.owner_email = u.email) AS streamer_code
        FROM users u
        WHERE u.email ILIKE ${searchTerm}
          OR EXISTS (
            SELECT 1 FROM affiliates a
            WHERE a.owner_email = u.email
              AND (a.code ILIKE ${searchTerm} OR a.name ILIKE ${searchTerm})
          )
        ORDER BY u.email
        LIMIT 20
      `;
      res.status(200).json({ ok: true, users: rows });
      return;
    }

    if (action === 'assign-streamer') {
      const { targetEmail, code, name } = req.body || {};
      if (!targetEmail || !code || !name) {
        res.status(400).json({ ok: false, error: 'missing_fields' });
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
      return;
    }

    if (action === 'grant-plan') {
      const { targetEmail, plan, days } = req.body || {};
      if (!targetEmail || !plan) {
        res.status(400).json({ ok: false, error: 'missing_fields' });
        return;
      }
      if (plan !== 'advanced' && plan !== 'complete') {
        res.status(400).json({ ok: false, error: 'invalid_plan' });
        return;
      }
      const durationDays = Number(days) > 0 ? Math.floor(Number(days)) : 30;
      const normalizedTarget = String(targetEmail).trim().toLowerCase();
      const intervalText = durationDays + ' days';
      const result = await sql`
        UPDATE users
        SET plan = ${plan}, expires_at = now() + ${intervalText}::interval
        WHERE email = ${normalizedTarget}
        RETURNING email, plan, expires_at
      `;
      if (result.length === 0) {
        res.status(404).json({ ok: false, error: 'target_not_found' });
        return;
      }
      res.status(200).json({ ok: true, plan: result[0].plan, expiresAt: result[0].expires_at });
      return;
    }

    res.status(400).json({ ok: false, error: 'unknown_action' });
  } catch (err) {
    console.error('admin error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
