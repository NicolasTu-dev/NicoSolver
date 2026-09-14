const NP_API = 'https://api.nowpayments.io/v1';

// price in USD (whole number)
const PLAN_PRICES_USD = {
  advanced: { title: 'Solverix — Advanced Solver (1 month)', price: 25 },
  complete: { title: 'Solverix — Complete (1 month)', price: 32 },
};

// order_id doubles as our record of who's paying, for what, and via which
// affiliate — the webhook parses it back apart once NOWPayments confirms
// the payment. Timestamp keeps it unique across repeat attempts.
async function createInvoice({ email, plan, siteUrl, apiUrl, refCode }){
  const planInfo = PLAN_PRICES_USD[plan];
  if(!planInfo) throw new Error('invalid_plan');

  const orderId = [email, plan, refCode || 'none', Date.now()].join('|');

  const res = await fetch(NP_API + '/invoice', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'x-api-key': process.env.NOWPAYMENTS_API_KEY,
    },
    body: JSON.stringify({
      price_amount: planInfo.price,
      price_currency: 'usd',
      order_id: orderId,
      order_description: planInfo.title,
      ipn_callback_url: apiUrl + '/api/nowpayments-webhook',
      success_url: siteUrl + '/cuenta.html?np=success',
      cancel_url: siteUrl + '/cuenta.html?np=cancel',
    }),
  });
  const data = await res.json();
  if(!res.ok){
    throw new Error(data.message || 'nowpayments_invoice_failed');
  }
  return data; // has .invoice_url, .id
}

module.exports = { createInvoice, PLAN_PRICES_USD };
