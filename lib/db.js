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
}

module.exports = { sql, ensureSchema };
