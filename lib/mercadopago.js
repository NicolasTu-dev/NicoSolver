const MP_API = 'https://api.mercadopago.com';

function accessToken(){
  return process.env.MP_ACCESS_TOKEN;
}

// price in ARS pesos (whole number)
const PLAN_PRICES = {
  advanced: { title: 'Solverix Solver Avanzado (1 mes)', price: 30000 },
  complete: { title: 'Solverix Completo (1 mes)', price: 40000 },
};

// affiliate, when given, is { accessToken, commissionRate }. The preference
// is then created using the AFFILIATE's Mercado Pago access token (so MP
// pays them as the collector) with a marketplace_fee equal to our share —
// MP splits the payment automatically at the moment of purchase, nothing
// else has to happen afterwards.
async function createPreference({ email, plan, siteUrl, apiUrl, refCode, affiliate }){
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

  let tokenForRequest = accessToken();
  if(affiliate){
    tokenForRequest = affiliate.accessToken;
    const ourShare = Math.round(planInfo.price * (1 - affiliate.commissionRate) * 100) / 100;
    body.marketplace_fee = ourShare;
  }

  const res = await fetch(MP_API + '/checkout/preferences', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      Authorization: 'Bearer ' + tokenForRequest,
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

// ---------- Affiliate OAuth (Mercado Pago marketplace connect) ----------

function affiliateAuthorizeUrl({ code, redirectUri }){
  const params = new URLSearchParams({
    client_id: process.env.MP_CLIENT_ID,
    response_type: 'code',
    platform_id: 'mp',
    state: code,
    redirect_uri: redirectUri,
  });
  return 'https://auth.mercadopago.com/authorization?' + params.toString();
}

async function exchangeAffiliateCode({ code, redirectUri }){
  const res = await fetch(MP_API + '/oauth/token', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      client_id: process.env.MP_CLIENT_ID,
      client_secret: process.env.MP_CLIENT_SECRET,
      grant_type: 'authorization_code',
      code,
      redirect_uri: redirectUri,
    }),
  });
  const data = await res.json();
  if(!res.ok){
    throw new Error(data.message || 'oauth_exchange_failed');
  }
  return data; // { access_token, refresh_token, user_id, expires_in, ... }
}

module.exports = { createPreference, getPayment, affiliateAuthorizeUrl, exchangeAffiliateCode };
