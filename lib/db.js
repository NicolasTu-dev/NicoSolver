const { neon } = require('@neondatabase/serverless');

// The Neon/Postgres Marketplace integration on Vercel injects the
// connection string as DATABASE_URL (falls back to POSTGRES_URL for
// older-style setups).
const connectionString = process.env.DATABASE_URL || process.env.POSTGRES_URL;
const sql = neon(connectionString);

// Creates the users table if it doesn't exist yet. Safe to call on every
// request — CREATE TABLE IF NOT EXISTS is a no-op once the table exists.
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
  // link (?ref=code). access_token/refresh_token are only present once the
  // affiliate has completed the Mercado Pago OAuth connect flow — until
  // then status stays 'pending' and the code can't be used for checkout.
  await sql`
    CREATE TABLE IF NOT EXISTS affiliates (
      id SERIAL PRIMARY KEY,
      code TEXT UNIQUE NOT NULL,
      name TEXT NOT NULL,
      commission_rate NUMERIC NOT NULL DEFAULT 0.20,
      status TEXT NOT NULL DEFAULT 'pending',
      mp_user_id TEXT,
      access_token TEXT,
      refresh_token TEXT,
      token_expires_at TIMESTAMPTZ,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    );
  `;

  // Ledger of commissions paid out automatically by Mercado Pago's
  // marketplace split at the moment of each referred purchase. This table
  // is only for reporting — the money movement itself already happened
  // inside the payment, nothing here triggers a transfer.
  await sql`
    CREATE TABLE IF NOT EXISTS affiliate_commissions (
      id SERIAL PRIMARY KEY,
      affiliate_code TEXT NOT NULL REFERENCES affiliates(code),
      buyer_email TEXT NOT NULL,
      plan TEXT NOT NULL,
      gross_amount NUMERIC NOT NULL,
      commission_amount NUMERIC NOT NULL,
      payment_id TEXT NOT NULL UNIQUE,
      created_at TIMESTAMPTZ NOT NULL DEFAULT now()
    );
  `;
}

module.exports = { sql, ensureSchema };
