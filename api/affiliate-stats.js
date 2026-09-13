const { sql, ensureSchema } = require('../lib/db');

// Admin-only report: /api/affiliate-stats?secret=... — how much each
// affiliate has generated, for keeping track of the marketplace split.
module.exports = async (req, res) => {
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
        COALESCE(SUM(c.commission_amount), 0) AS total_commission
      FROM affiliates a
      LEFT JOIN affiliate_commissions c ON c.affiliate_code = a.code
      GROUP BY a.code, a.name, a.status, a.commission_rate
      ORDER BY total_commission DESC
    `;
    res.status(200).json({ ok: true, affiliates: rows });
  } catch (err) {
    console.error('affiliate-stats error', err);
    res.status(500).json({ ok: false, error: 'server_error' });
  }
};
