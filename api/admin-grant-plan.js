const bcrypt = require('bcryptjs');
const { sql, ensureSchema } = require('../lib/db');

const FOUNDER_EMAIL = 'nicolastu98@hotmail.com';

// The founder grants a subscription to an existing login account
// (targetEmail) directly from the web UI — same use case as manually
// activating a friend's account without going through a real payment.
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, password, targetEmail, plan, days } = req.body || {};
  if (!email || !password || !targetEmail || !plan) {
    res.status(400).json({ ok: false, error: 'missing_fields' });
    return;
  }
  if (plan !== 'advanced' && plan !== 'complete') {
    res.status(400).json({ ok: false, error: 'invalid_plan' });
    return;
  }
  const durationDays = Number(days) > 0 ? Math.floor(Number(days)) : 30;

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
    const result = await sql`
      UPDATE users
      SET plan = ${plan}, expires_at = now() + (${durationDays} || ' days')::interval
      WHERE email = ${normalizedTarget}
      RETURNING email, plan, expires_at
    `;

    if (result.length === 0) {
      res.status(404).json({ ok: false, error: 'target_not_found' });
      return;
    }

    res.status(200).json({ ok: true, plan: result[0].plan, expiresAt: result[0].expires_at });
  } catch (err) {
    console.error('admin-grant-plan error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
