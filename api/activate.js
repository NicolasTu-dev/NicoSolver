const { sql, ensureSchema } = require('../lib/db');

// Demo-only: simulates subscribing. There is no real payment here — this
// endpoint just sets the plan and pushes expires_at 30 days out. Wiring a
// real charge (e.g. Mercado Pago) later means calling this only after a
// successful payment webhook, instead of directly from the client.
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }
  if (req.method !== 'POST') {
    res.status(405).json({ ok: false, error: 'method_not_allowed' });
    return;
  }

  const { email, plan } = req.body || {};
  if (!email || !plan || (plan !== 'advanced' && plan !== 'complete')) {
    res.status(400).json({ ok: false, error: 'missing_or_invalid_fields' });
    return;
  }

  const normalizedEmail = String(email).trim().toLowerCase();

  try {
    await ensureSchema();

    const result = await sql`
      UPDATE users
      SET plan = ${plan}, expires_at = now() + interval '30 days'
      WHERE email = ${normalizedEmail}
      RETURNING plan, expires_at
    `;
    if (result.length === 0) {
      res.status(404).json({ ok: false, error: 'user_not_found' });
      return;
    }

    const user = result[0];
    res.status(200).json({
      ok: true,
      plan: user.plan,
      expiresAt: new Date(user.expires_at).toISOString(),
    });
  } catch (err) {
    console.error('activate error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
