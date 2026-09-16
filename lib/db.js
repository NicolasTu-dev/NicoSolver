const { neon } = require('@neondatabase/serverless');

// The Neon/Postgres Marketplace integration on Vercel injects the
// connection string as DATABASE_URL (falls back to POSTGRES_URL for
// older-style setups).
const connectionString = process.env.DATABASE_URL || process.env.POSTGRES_URL;
const sql = neon(connectionString);

// Creates the users table if it doesn't exist yet. Safe to call on every
// request CREATE TABLE IF NOT EXISTS is a no-op once the table exists.
async function ensureSchema() {
  await sql`
    CREATE TABLE IF NOT EXISTS users (
      id SERIAL PRIMARY KEY,
      email TEXT UNIQUE NOT NULL,
      password_hash TEXT NOT NULL,
      plan TEXT NOT NULL DEFAULT 'none',
      expires_at TIMESTAMPTZ,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    );
  `;

  // One row per affiliate (streamer). code is what goes in the referral
  // link (?ref=code). owner_email links the affiliate to a login account
  // (users.email) so that account can show a "streamer" badge, its own
  // pending/paid commission totals, and let the code be looked up without
  // the admin secret. There's no payment-processor connection here on
  // purpose payouts are done by hand (see affiliate_commissions below).
  await sql`
    CREATE TABLE IF NOT EXISTS affiliates (
      id SERIAL PRIMARY KEY,
      code TEXT UNIQUE NOT NULL,
      name TEXT NOT NULL,
      commission_rate NUMERIC NOT NULL DEFAULT 0.20,
      status TEXT NOT NULL DEFAULT 'pending',
      owner_email TEXT,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    );
  `;
  await sql`ALTER TABLE affiliates ADD COLUMN IF NOT EXISTS owner_email TEXT`;
  await sql`ALTER TABLE affiliates DROP COLUMN IF EXISTS mp_user_id`;
  await sql`ALTER TABLE affiliates DROP COLUMN IF EXISTS access_token`;
  await sql`ALTER TABLE affiliates DROP COLUMN IF EXISTS refresh_token`;
  await sql`ALTER TABLE affiliates DROP COLUMN IF EXISTS token_expires_at`;

  // Ledger of commissions owed to each affiliate for a referred purchase.
  // Nothing here moves money automatically the founder reviews this list
  // in the account page's founder panel and transfers funds by hand, then
  // marks the row 'paid'. currency/source tell apart Mercado Pago (ARS)
  // sales from crypto (USD) sales, since they're paid out separately.
  await sql`
    CREATE TABLE IF NOT EXISTS affiliate_commissions (
      id SERIAL PRIMARY KEY,
      affiliate_code TEXT NOT NULL REFERENCES affiliates(code),
      buyer_email TEXT NOT NULL,
      plan TEXT NOT NULL,
      gross_amount NUMERIC NOT NULL,
      commission_amount NUMERIC NOT NULL,
      currency TEXT NOT NULL DEFAULT 'ARS',
      source TEXT NOT NULL DEFAULT 'mercadopago',
      status TEXT NOT NULL DEFAULT 'pending',
      payment_id TEXT NOT NULL UNIQUE,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
      paid_at TIMESTAMPTZ
    );
  `;
  await sql`ALTER TABLE affiliate_commissions ADD COLUMN IF NOT EXISTS currency TEXT NOT NULL DEFAULT 'ARS'`;
  await sql`ALTER TABLE affiliate_commissions ADD COLUMN IF NOT EXISTS source TEXT NOT NULL DEFAULT 'mercadopago'`;
  await sql`ALTER TABLE affiliate_commissions ADD COLUMN IF NOT EXISTS status TEXT NOT NULL DEFAULT 'pending'`;
  await sql`ALTER TABLE affiliate_commissions ADD COLUMN IF NOT EXISTS paid_at TIMESTAMPTZ`;
}

module.exports = { sql, ensureSchema };
