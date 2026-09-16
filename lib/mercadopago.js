const MP_API = 'https://api.mercadopago.com';

function accessToken(){
  return process.env.MP_ACCESS_TOKEN;
}

// price in ARS pesos (whole number)
const PLAN_PRICES = {
  advanced: { title: 'Solverix Solver Avanzado (1 mes)', price: 30000 },
  complete: { title: 'Solverix Completo (1 mes)', price: 40000 },
};

// refCode (when given) just rides along in external_reference so the
// webhook can log a commission for that affiliate once the payment is
// approved. The payment itself always goes through OUR OWN Mercado Pago
// account there's no automatic split; payouts to streamers are done by
// hand from the founder panel.
async function createPreference({ email, plan, siteUrl, apiUrl, refCode }){
  const planInfo = PLAN_PRICES[plan];
  if(!planInfo) throw new Error('invalid_plan');

  const externalReference = [email, plan, refCode || 'none'].join('|');

  const body = {
    items: [
      {
        title: planInfo.title,
        quantity: 1,
        currency_id: 'ARS',
        unit_price: planInfo.price,
      },
    ],
    payer: { email },
    external_reference: externalReference,
    back_urls: {
      success: siteUrl + '/?mp=success',
      failure: siteUrl + '/?mp=failure',
      pending: siteUrl + '/?mp=pending',
    },
    auto_return: 'approved',
    notification_url: apiUrl + '/api/mp-webhook',
  };

  const res = await fetch(MP_API + '/checkout/preferences', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      Authorization: 'Bearer ' + accessToken(),
    },
    body: JSON.stringify(body),
  });
  const data = await res.json();
  if(!res.ok){
    throw new Error(data.message || 'mp_preference_failed');
  }
  return data; // has .init_point (production) / .sandbox_init_point (test)
}

async function getPayment(paymentId){
  const res = await fetch(MP_API + '/v1/payments/' + paymentId, {
    headers: { Authorization: 'Bearer ' + accessToken() },
  });
  if(!res.ok) return null;
  return res.json();
}

module.exports = { createPreference, getPayment };
