const { sql, ensureSchema } = require('../lib/db');

// Admin-only report: /api/affiliate-stats?secret=... how much each
// affiliate has generated and how much is still owed, for keeping track
// of manual payouts outside the founder panel in the account page.
module.exports = async (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');
  if (req.method === 'OPTIONS') { res.status(204).end(); return; }

  if (req.query.secret !== process.env.ADMIN_SECRET) {
    res.status(403).json({ ok: false, error: 'forbidden' });
    return;
  }

  try {
    await ensureSchema();
    const rows = await sql`
      SELECT
        a.code,
        a.name,
        a.status,
        a.commission_rate,
        COUNT(c.id) AS sales_count,
        COALESCE(SUM(CASE WHEN c.currency = 'ARS' AND c.status = 'pending' THEN c.commission_amount ELSE 0 END), 0) AS pending_ars,
        COALESCE(SUM(CASE WHEN c.currency = 'ARS' AND c.status = 'paid' THEN c.commission_amount ELSE 0 END), 0) AS paid_ars,
        COALESCE(SUM(CASE WHEN c.currency = 'USD' AND c.status = 'pending' THEN c.commission_amount ELSE 0 END), 0) AS pending_usd,
        COALESCE(SUM(CASE WHEN c.currency = 'USD' AND c.status = 'paid' THEN c.commission_amount ELSE 0 END), 0) AS paid_usd
      FROM affiliates a
      LEFT JOIN affiliate_commissions c ON c.affiliate_code = a.code
      GROUP BY a.code, a.name, a.status, a.commission_rate
      ORDER BY (pending_ars + pending_usd) DESC
    `;
    res.status(200).json({ ok: true, affiliates: rows });
  } catch (err) {
    console.error('affiliate-stats error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
