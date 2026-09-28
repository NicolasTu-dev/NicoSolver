  const API_BASE = 'https://solverix-api-nicolastu-devs-projects.vercel.app';
  const DOWNLOAD_URL = 'https://github.com/NicolasTu-dev/NicoSolver/releases/download/solverix-v1/SolverixSetup.exe';
  const PLAN_NAMES_EN = { none: 'No subscription', advanced: 'Advanced Solver', complete: 'Complete' };
  const PLAN_NAMES_ES = { none: 'Sin suscripción', advanced: 'Solver Avanzado', complete: 'Completo' };
  const PLAN_NAMES_PT = { none: 'Sem assinatura', advanced: 'Solver Avançado', complete: 'Completo' };

  // Affiliate link tracking handled in js/shared.js

  // ---------- Language switch (EN default, ES optional) ----------
  const ES_TEXT = {
    backLink: '← Volver al inicio',
    pageTitle: '🔑 Mi cuenta',
    pageSubtitle: 'Iniciá sesión o registrate para adquirir un plan y descargar Solverix.',
    labelEmail: 'Email',
    labelPassword: 'Contraseña',
    btnLogin: 'Iniciar sesión',
    btnRegister: 'Registrarme',
    logoutLink: 'Cerrar sesión',
    founderTitle: 'Gestionar cuentas',
    founderSearchPlaceholder: 'Buscar por email...',
    founderGrantTitle: 'Otorgar suscripción',
    founderPlanLabel: 'Plan',
    founderPlanAdvanced: 'Solver Avanzado',
    founderPlanComplete: 'Completo',
    founderDaysLabel: 'Días',
    founderGrantBtn: 'Otorgar suscripción',
    founderStreamerTitle: 'Asignar rol de streamer',
    founderCodeLabel: 'Código de streamer',
    founderNameLabel: 'Nombre para mostrar',
    founderAssignBtn: 'Asignar rol de streamer',
    founderPayoutsTitle: 'Pagos a streamers',
    founderPayoutsBtn: 'Ver pagos a streamers',
    footerPrivacy: 'Política de Privacidad',
    footerTerms: 'Términos de Servicio',
  };
  const PT_TEXT = {
    backLink: '← Voltar para o início',
    pageTitle: '🔑 Minha conta',
    pageSubtitle: 'Faça login ou cadastre-se para adquirir um plano e baixar o Solverix.',
    labelEmail: 'Email',
    labelPassword: 'Senha',
    btnLogin: 'Entrar',
    btnRegister: 'Cadastrar',
    logoutLink: 'Sair',
    founderTitle: 'Gerenciar contas',
    founderSearchPlaceholder: 'Buscar por email...',
    founderGrantTitle: 'Conceder assinatura',
    founderPlanLabel: 'Plano',
    founderPlanAdvanced: 'Solver Avançado',
    founderPlanComplete: 'Completo',
    founderDaysLabel: 'Dias',
    founderGrantBtn: 'Conceder assinatura',
    founderStreamerTitle: 'Atribuir papel de streamer',
    founderCodeLabel: 'Código de streamer',
    founderNameLabel: 'Nome de exibição',
    founderAssignBtn: 'Atribuir papel de streamer',
    founderPayoutsTitle: 'Pagamentos a streamers',
    founderPayoutsBtn: 'Ver pagamentos a streamers',
    footerPrivacy: 'Política de Privacidade',
    footerTerms: 'Termos de Serviço',
  };
  const EN_TEXT = {};
  document.querySelectorAll('[data-i18n]').forEach(el => { EN_TEXT[el.getAttribute('data-i18n')] = el.innerHTML; });
  const EN_PLACEHOLDERS = {};
  document.querySelectorAll('[data-i18n-placeholder]').forEach(el => { EN_PLACEHOLDERS[el.getAttribute('data-i18n-placeholder')] = el.placeholder; });
  const TEXT_DICTS = { en: EN_TEXT, es: ES_TEXT, pt: PT_TEXT };

  function currentLang(){ return localStorage.getItem('solverix_lang') || 'en'; }

  // Small helper for the many inline en/es/pt string choices scattered
  // through this file, so adding a language doesn't mean rewriting every
  // ternary call as t('English', 'Español', 'Português').
  function t(en, es, pt){
    const lang = currentLang();
    if(lang === 'es') return es;
    if(lang === 'pt') return pt;
    return en;
  }

  function applyLang(lang){
    const dict = TEXT_DICTS[lang] || EN_TEXT;
    document.querySelectorAll('[data-i18n]').forEach(el => {
      const key = el.getAttribute('data-i18n');
      if(dict[key] !== undefined) el.innerHTML = dict[key];
    });
    document.querySelectorAll('[data-i18n-placeholder]').forEach(el => {
      const key = el.getAttribute('data-i18n-placeholder');
      const value = lang === 'en' ? EN_PLACEHOLDERS[key] : dict[key];
      if(value !== undefined) el.placeholder = value;
    });
    document.documentElement.lang = lang;
    document.getElementById('langSwitch').value = lang;
    localStorage.setItem('solverix_lang', lang);
  }
  applyLang(currentLang());
  document.getElementById('langSwitch').addEventListener('change', (e) => {
    applyLang(e.target.value);
  });

  // ---------- Mercado Pago return handling ----------
  (function(){
    const params = new URLSearchParams(window.location.search);
    const mpStatus = params.get('mp');
    if(!mpStatus) return;
    const messages = {
      en: {
        success: '✅ Payment received! Log in below to see your active plan and download Solverix.',
        pending: '⏳ Your payment is pending approval. Log in again in a few minutes to check the status.',
        failure: '❌ The payment wasn\'t completed. You can try again by logging in below.',
      },
      es: {
        success: '✅ ¡Pago recibido! Iniciá sesión abajo para ver tu plan activo y descargar Solverix.',
        pending: '⏳ Tu pago está pendiente de aprobación. Iniciá sesión en unos minutos para ver el estado.',
        failure: '❌ El pago no se completó. Podés intentarlo de nuevo iniciando sesión abajo.',
      },
      pt: {
        success: '✅ Pagamento recebido! Faça login abaixo para ver seu plano ativo e baixar o Solverix.',
        pending: '⏳ Seu pagamento está pendente de aprovação. Faça login novamente em alguns minutos para ver o status.',
        failure: '❌ O pagamento não foi concluído. Você pode tentar de novo fazendo login abaixo.',
      },
    };
    const set = messages[currentLang()] || messages.en;
    if(set[mpStatus]){
      const banner = document.createElement('div');
      banner.className = 'mp-banner';
      banner.textContent = set[mpStatus];
      document.getElementById('mpBannerArea').appendChild(banner);
    }
  })();

  // ---------- Crypto (NOWPayments) return handling ----------
  (function(){
    const params = new URLSearchParams(window.location.search);
    const npStatus = params.get('np');
    if(!npStatus) return;
    const messages = {
      en: {
        success: '✅ Payment sent! It can take a few minutes to confirm on-chain log in below in a bit to see your active plan.',
        cancel: '❌ The payment wasn\'t completed. You can try again by logging in below.',
      },
      es: {
        success: '✅ ¡Pago enviado! Puede tardar unos minutos en confirmarse en la blockchain iniciá sesión en un rato para ver tu plan activo.',
        cancel: '❌ El pago no se completó. Podés intentarlo de nuevo iniciando sesión abajo.',
      },
      pt: {
        success: '✅ Pagamento enviado! Pode levar alguns minutos para confirmar na blockchain faça login daqui a pouco para ver seu plano ativo.',
        cancel: '❌ O pagamento não foi concluído. Você pode tentar de novo fazendo login abaixo.',
      },
    };
    const set = messages[currentLang()] || messages.en;
    if(set[npStatus]){
      const banner = document.createElement('div');
      banner.className = 'mp-banner';
      banner.textContent = set[npStatus];
      document.getElementById('mpBannerArea').appendChild(banner);
    }
  })();

  const loginBlock = document.getElementById('loginBlock');
  const loggedInBlock = document.getElementById('loggedInBlock');
  const accEmail = document.getElementById('accEmail');
  const accPassword = document.getElementById('accPassword');
  const accStatus = document.getElementById('accStatus');
  const accLoginBtn = document.getElementById('accLoginBtn');
  const accRegisterBtn = document.getElementById('accRegisterBtn');
  const accPlanName = document.getElementById('accPlanName');
  const accDays = document.getElementById('accDays');
  const accExpiry = document.getElementById('accExpiry');
  const accDownloadArea = document.getElementById('accDownloadArea');
  const accLogoutLink = document.getElementById('accLogoutLink');

  function setStatus(msgEn, msgEs, msgPt, kind){
    accStatus.textContent = t(msgEn, msgEs, msgPt);
    accStatus.className = 'acc-status' + (kind ? ' ' + kind : '');
  }

  function daysUntil(iso){
    if(!iso) return 0;
    const ms = new Date(iso).getTime() - Date.now();
    return Math.max(0, Math.ceil(ms / 86400000));
  }

  async function apiCall(path, body){
    const res = await fetch(API_BASE + path, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
    });
    return res.json().catch(() => ({ ok: false, error: 'bad_response' }));
  }

  const SUPPORT_EMAIL = 'soporte@solverix.com.ar';

  function renderStreamerBadge(streamer){
    const badgeEl = document.getElementById('streamerBadge');
    if(!streamer){ badgeEl.style.display = 'none'; badgeEl.innerHTML = ''; return; }
    const label = 'Streamer';
    badgeEl.style.display = 'block';
    let html = '<span class="streamer-badge">🎥 ' + label + ' · ' + streamer.code.toUpperCase() + '</span>';

    const hasPending = streamer.pendingArs > 0 || streamer.pendingUsd > 0;
    if(hasPending || streamer.paidArs > 0 || streamer.paidUsd > 0){
      const pendingLabel = t('Pending', 'Pendiente', 'Pendente');
      const paidLabel = t('Already paid', 'Ya cobrado', 'Já recebido');
      let rows = '';
      if(streamer.pendingArs > 0 || streamer.paidArs > 0){
        rows += '<div class="se-row"><span>' + pendingLabel + ' (ARS)</span><span class="se-amount">$' + streamer.pendingArs.toLocaleString(t('en-US','es-AR','pt-BR')) + '</span></div>';
        if(streamer.paidArs > 0) rows += '<div class="se-row"><span>' + paidLabel + ' (ARS)</span><span>$' + streamer.paidArs.toLocaleString(t('en-US','es-AR','pt-BR')) + '</span></div>';
      }
      if(streamer.pendingUsd > 0 || streamer.paidUsd > 0){
        rows += '<div class="se-row"><span>' + pendingLabel + ' (USD)</span><span class="se-amount">$' + streamer.pendingUsd.toLocaleString(t('en-US','es-AR','pt-BR')) + '</span></div>';
        if(streamer.paidUsd > 0) rows += '<div class="se-row"><span>' + paidLabel + ' (USD)</span><span>$' + streamer.paidUsd.toLocaleString(t('en-US','es-AR','pt-BR')) + '</span></div>';
      }
      html += '<div class="streamer-earnings">' + rows;
      if(hasPending){
        const subject = encodeURIComponent(t('Streamer payout request', 'Solicitud de pago de streamer', 'Solicitação de pagamento de streamer'));
        const bodyLines = [
          t('Hi, I\'d like to request my pending streamer payout.', 'Hola, quiero solicitar el pago pendiente de mi comisión de streamer.', 'Olá, gostaria de solicitar o pagamento pendente da minha comissão de streamer.'),
          t('Code: ', 'Código: ', 'Código: ') + streamer.code.toUpperCase(),
        ];
        if(streamer.pendingArs > 0) bodyLines.push(t('Pending in ARS: $', 'Pendiente en ARS: $', 'Pendente em ARS: $') + streamer.pendingArs);
        if(streamer.pendingUsd > 0) bodyLines.push(t('Pending in USD: $', 'Pendiente en USD: $', 'Pendente em USD: $') + streamer.pendingUsd);
        const body = encodeURIComponent(bodyLines.join('\n'));
        const mailLabel = t('✉ Request payout by email', '✉ Solicitar pago por mail', '✉ Solicitar pagamento por email');
        html += '<a class="btn btn-ghost" href="mailto:' + SUPPORT_EMAIL + '?subject=' + subject + '&body=' + body + '">' + mailLabel + '</a>';
      }
      html += '</div>';
    }
    badgeEl.innerHTML = html;
  }

  function showLoggedIn(email, plan, expiresAt, streamer){
    loginBlock.style.display = 'none';
    loggedInBlock.style.display = 'block';
    const lang = currentLang();
    const planNames = lang === 'es' ? PLAN_NAMES_ES : (lang === 'pt' ? PLAN_NAMES_PT : PLAN_NAMES_EN);
    accPlanName.textContent = planNames[plan] || plan;
    accDays.textContent = daysUntil(expiresAt) + t(' days left', ' días restantes', ' dias restantes');
    accExpiry.textContent = t('Expires on ', 'Vence el ', 'Vence em ') +
      new Date(expiresAt).toLocaleDateString(t('en-US', 'es-AR', 'pt-BR'));
    const dlLabel = t('⬇ Download Solverix', '⬇ Descargar Solverix', '⬇ Baixar Solverix');
    const dlNote = t(
      'The app unlocks your plan automatically when you log in inside it.',
      'La app desbloquea tu plan automáticamente al iniciar sesión adentro.',
      'O app desbloqueia seu plano automaticamente ao fazer login dentro dele.'
    );
    document.getElementById('accPlanDot').style.display = '';
    document.getElementById('accPlanStatus').textContent = t(' active', ' activo', ' ativo');
    renderStreamerBadge(streamer);
    accDownloadArea.innerHTML =
      '<a class="btn btn-gold acc-download-btn" href="' + DOWNLOAD_URL + '">' + dlLabel + '</a>' +
      '<div style="font-size:11.5px; color:var(--muted-2); margin-top:8px; text-align:center;">' + dlNote + '</div>';
  }

  function showNoPlan(email, streamer){
    renderStreamerBadge(streamer);
    loginBlock.style.display = 'none';
    loggedInBlock.style.display = 'block';
    const lang = currentLang();
    const planNames = lang === 'es' ? PLAN_NAMES_ES : (lang === 'pt' ? PLAN_NAMES_PT : PLAN_NAMES_EN);
    accPlanName.textContent = planNames.none;
    document.getElementById('accPlanDot').style.display = 'none';
    document.getElementById('accPlanStatus').textContent = '';
    accDays.textContent = '';
    accExpiry.textContent = t('Get a plan to download Solverix.', 'Adquirí un plan para poder descargar Solverix.', 'Adquira um plano para poder baixar o Solverix.');
    const advLabel = t('Get Advanced', 'Adquirir Avanzado', 'Adquirir Avançado');
    const compLabel = t('Get Complete', 'Adquirir Completo', 'Adquirir Completo');
    const codeToggleLabel = t('Have a streamer code?', '¿Tenés un código de streamer?', 'Tem um código de streamer?');
    const codeLabel = t('Streamer code', 'Código de streamer', 'Código de streamer');
    const savedCode = localStorage.getItem('solverix_ref') || '';
    accDownloadArea.innerHTML =
      '<button type="button" class="code-toggle' + (savedCode ? ' open' : '') + '" id="codeToggle"><span class="arrow">▸</span>' + codeToggleLabel + '</button>' +
      '<div class="code-body' + (savedCode ? ' open' : '') + '" id="codeBody">' +
      '<div class="acc-field-label">' + codeLabel + '</div>' +
      '<input class="acc-input" id="affiliateCodeInput" style="text-transform:uppercase;" value="' + savedCode + '">' +
      '</div>' +
      '<div class="acc-plan-pick">' +
      '<button class="btn btn-ghost" id="pickAdvanced">' + advLabel + '</button>' +
      '<button class="btn btn-gold" id="pickComplete">' + compLabel + '</button>' +
      '</div>' +
      '<div id="checkoutStatus" class="acc-status"></div>';
    document.getElementById('codeToggle').addEventListener('click', () => {
      document.getElementById('codeToggle').classList.toggle('open');
      document.getElementById('codeBody').classList.toggle('open');
    });
    document.getElementById('affiliateCodeInput').addEventListener('input', (e) => {
      const value = e.target.value.trim().toLowerCase();
      if(value) localStorage.setItem('solverix_ref', value);
      else localStorage.removeItem('solverix_ref');
    });
    document.getElementById('pickAdvanced').onclick = () => showPaymentOptions(email, 'advanced');
    document.getElementById('pickComplete').onclick = () => showPaymentOptions(email, 'complete');
  }

  function showPaymentOptions(email, plan){
    const backLabel = t('← Choose a different plan', '← Elegir otro plan', '← Escolher outro plano');
    const cryptoRegion = t('Rest of the world', 'Resto del mundo', 'Resto do mundo');
    const cryptoNote = t(
      'USDT from any wallet or exchange (including Binance)',
      'USDT desde cualquier wallet o exchange (incluye Binance)',
      'USDT de qualquer carteira ou exchange (incluindo a Binance)'
    );
    const mpRegion = 'Argentina';
    const mpNote = t('Argentine pesos, card or account balance', 'Pesos argentinos, tarjeta o saldo en cuenta', 'Pesos argentinos, cartão ou saldo em conta');
    accDownloadArea.innerHTML =
      '<button type="button" class="pay-back" id="payBack">' + backLabel + '</button>' +
      '<button type="button" class="pay-option featured" id="payCrypto">' +
      '<div class="pay-region">' + cryptoRegion + '</div>' +
      '<div class="pay-method">Crypto (USDT)</div>' +
      '<div class="pay-note">' + cryptoNote + '</div>' +
      '</button>' +
      '<button type="button" class="pay-option" id="payMp">' +
      '<div class="pay-region">' + mpRegion + '</div>' +
      '<div class="pay-method">Mercado Pago</div>' +
      '<div class="pay-note">' + mpNote + '</div>' +
      '</button>' +
      '<div id="checkoutStatus" class="acc-status"></div>';
    document.getElementById('payBack').onclick = () => showNoPlan(email);
    document.getElementById('payMp').onclick = () => startCheckout(email, plan);
    document.getElementById('payCrypto').onclick = () => startCryptoCheckout(email, plan);
  }

  async function startCheckout(email, plan){
    const statusEl = document.getElementById('checkoutStatus');
    if(statusEl){ statusEl.textContent = t('Opening Mercado Pago...', 'Abriendo Mercado Pago...', 'Abrindo o Mercado Pago...'); statusEl.className = 'acc-status info'; }
    const ref = localStorage.getItem('solverix_ref') || undefined;
    const result = await apiCall('/api/create-checkout', { email, plan, ref });
    if(!result.ok || !result.checkoutUrl){
      if(statusEl){ statusEl.textContent = t('Couldn\'t start the payment. Try again.', 'No se pudo iniciar el pago. Probá de nuevo.', 'Não foi possível iniciar o pagamento. Tente de novo.'); statusEl.className = 'acc-status err'; }
      return;
    }
    window.location.href = result.checkoutUrl;
  }

  async function startCryptoCheckout(email, plan){
    const statusEl = document.getElementById('checkoutStatus');
    if(statusEl){ statusEl.textContent = t('Opening the crypto payment...', 'Abriendo el pago en cripto...', 'Abrindo o pagamento em cripto...'); statusEl.className = 'acc-status info'; }
    const ref = localStorage.getItem('solverix_ref') || undefined;
    const result = await apiCall('/api/create-checkout-crypto', { email, plan, ref });
    if(!result.ok || !result.checkoutUrl){
      if(statusEl){ statusEl.textContent = t('Couldn\'t start the payment. Try again.', 'No se pudo iniciar el pago. Probá de nuevo.', 'Não foi possível iniciar o pagamento. Tente de novo.'); statusEl.className = 'acc-status err'; }
      return;
    }
    window.location.href = result.checkoutUrl;
  }

  accLoginBtn.addEventListener('click', async () => {
    const email = accEmail.value.trim();
    const password = accPassword.value;
    if(!email || !password){ setStatus('Fill in your email and password.', 'Completá email y contraseña.', 'Preencha seu email e senha.', 'err'); return; }
    setStatus('Connecting...', 'Conectando...', 'Conectando...', 'info');
    const result = await apiCall('/api/login', { email, password });
    if(!result.ok){
      const invalid = result.error === 'invalid_credentials';
      setStatus(
        invalid ? 'Incorrect email or password.' : 'Couldn\'t connect.',
        invalid ? 'Email o contraseña incorrectos.' : 'No se pudo conectar.',
        invalid ? 'Email ou senha incorretos.' : 'Não foi possível conectar.',
        'err'
      );
      return;
    }
    if(result.active){
      showLoggedIn(email, result.plan, result.expiresAt, result.streamer);
    }else{
      showNoPlan(email, result.streamer);
    }
    if(result.isFounder){
      founderEmail = email;
      founderPassword = password;
      document.getElementById('founderPanel').style.display = 'block';
    }
  });

  // ---------- Founder panel: assign streamer role to any account ----------
  let founderEmail = null;
  let founderPassword = null;
  let founderSelectedTarget = null;
  let founderSearchTimer = null;

  const founderSearchInput = document.getElementById('founderSearch');
  const founderResults = document.getElementById('founderResults');
  const founderAssign = document.getElementById('founderAssign');
  const founderSelectedEmailEl = document.getElementById('founderSelectedEmail');
  const founderCodeInput = document.getElementById('founderCode');
  const founderNameInput = document.getElementById('founderName');
  const founderPlanInput = document.getElementById('founderPlan');
  const founderDaysInput = document.getElementById('founderDays');
  const founderStatusEl = document.getElementById('founderStatus');

  function setFounderStatus(msgEn, msgEs, msgPt, kind){
    founderStatusEl.textContent = t(msgEn, msgEs, msgPt);
    founderStatusEl.className = 'acc-status' + (kind ? ' ' + kind : '');
  }

  founderSearchInput.addEventListener('input', () => {
    clearTimeout(founderSearchTimer);
    const query = founderSearchInput.value.trim();
    founderSearchTimer = setTimeout(async () => {
      if(!query){ founderResults.innerHTML = ''; return; }
      const result = await apiCall('/api/admin', { email: founderEmail, password: founderPassword, action: 'search', query });
      if(!result.ok){
        founderResults.innerHTML = '<div style="font-size:12.5px; color:#e05252; margin-top:8px;">' +
          t('Search failed, try again.', 'La búsqueda falló, probá de nuevo.', 'A busca falhou, tente de novo.') + '</div>';
        return;
      }
      founderResults.innerHTML = result.users.map(u => {
        const tag = u.streamer_code ? '<span class="fr-tag">🎥 ' + u.streamer_code.toUpperCase() + '</span>' : '';
        return '<div class="founder-result" data-email="' + u.email + '"><span>' + u.email + '</span>' + tag + '</div>';
      }).join('') || '<div style="font-size:12.5px; color:var(--muted-2); margin-top:8px;">' + t('No results.', 'Sin resultados.', 'Nenhum resultado.') + '</div>';
      founderResults.querySelectorAll('.founder-result').forEach(row => {
        row.addEventListener('click', () => {
          founderSelectedTarget = row.getAttribute('data-email');
          founderSelectedEmailEl.textContent = founderSelectedTarget;
          founderCodeInput.value = '';
          founderNameInput.value = '';
          founderPlanInput.value = 'advanced';
          founderDaysInput.value = 30;
          founderAssign.style.display = 'block';
          setFounderStatus('', '', '', '');
        });
      });
    }, 300);
  });

  document.getElementById('founderAssignBtn').addEventListener('click', async () => {
    if(!founderSelectedTarget) return;
    const code = founderCodeInput.value.trim();
    const name = founderNameInput.value.trim();
    if(!code || !name){
      setFounderStatus('Fill in the code and the display name.', 'Completá el código y el nombre.', 'Preencha o código e o nome de exibição.', 'err');
      return;
    }
    setFounderStatus('Assigning...', 'Asignando...', 'Atribuindo...', 'info');
    const result = await apiCall('/api/admin', {
      email: founderEmail, password: founderPassword, action: 'assign-streamer', targetEmail: founderSelectedTarget, code, name,
    });
    if(!result.ok){
      setFounderStatus('Couldn\'t assign the streamer role.', 'No se pudo asignar el rol de streamer.', 'Não foi possível atribuir o papel de streamer.', 'err');
      return;
    }
    setFounderStatus('Streamer role assigned.', 'Rol de streamer asignado.', 'Papel de streamer atribuído.', 'ok');
  });

  document.getElementById('founderGrantBtn').addEventListener('click', async () => {
    if(!founderSelectedTarget) return;
    const plan = founderPlanInput.value;
    const days = founderDaysInput.value;
    setFounderStatus('Granting...', 'Otorgando...', 'Concedendo...', 'info');
    const result = await apiCall('/api/admin', {
      email: founderEmail, password: founderPassword, action: 'grant-plan', targetEmail: founderSelectedTarget, plan, days,
    });
    if(!result.ok){
      setFounderStatus('Couldn\'t grant the subscription.', 'No se pudo otorgar la suscripción.', 'Não foi possível conceder a assinatura.', 'err');
      return;
    }
    setFounderStatus('Subscription granted.', 'Suscripción otorgada.', 'Assinatura concedida.', 'ok');
  });

  // ---------- Founder panel: streamer payouts (view + mark paid) ----------
  const founderPayoutsToggle = document.getElementById('founderPayoutsToggle');
  const founderPayoutsList = document.getElementById('founderPayoutsList');
  const founderPayoutsStatusEl = document.getElementById('founderPayoutsStatus');
  let payoutsLoaded = false;

  function setPayoutsStatus(msgEn, msgEs, msgPt, kind){
    founderPayoutsStatusEl.textContent = t(msgEn, msgEs, msgPt);
    founderPayoutsStatusEl.className = 'acc-status' + (kind ? ' ' + kind : '');
  }

  function money(n, currency){
    return '$' + Number(n).toLocaleString(t('en-US','es-AR','pt-BR')) + ' ' + currency;
  }

  async function loadPayoutsList(){
    founderPayoutsList.innerHTML = '<div style="font-size:12.5px; color:var(--muted-2);">' + t('Loading...', 'Cargando...', 'Carregando...') + '</div>';
    const result = await apiCall('/api/admin', { email: founderEmail, password: founderPassword, action: 'affiliate-summary' });
    if(!result.ok){
      founderPayoutsList.innerHTML = '';
      setPayoutsStatus('Couldn\'t load streamer payouts.', 'No se pudo cargar los pagos a streamers.', 'Não foi possível carregar os pagamentos aos streamers.', 'err');
      return;
    }
    if(result.affiliates.length === 0){
      founderPayoutsList.innerHTML = '<div style="font-size:12.5px; color:var(--muted-2);">' + t('No streamers yet.', 'Todavía no hay streamers.', 'Ainda não há streamers.') + '</div>';
      return;
    }
    founderPayoutsList.innerHTML = result.affiliates.map(a => {
      const pendingArs = Number(a.pending_ars), pendingUsd = Number(a.pending_usd);
      const pendingText = (pendingArs > 0 ? money(pendingArs, 'ARS') : '') + (pendingArs > 0 && pendingUsd > 0 ? ' · ' : '') + (pendingUsd > 0 ? money(pendingUsd, 'USD') : '');
      return '<div class="payout-row" data-code="' + a.code + '">' +
        '<div class="pr-head"><span>🎥 ' + a.code.toUpperCase() + ' (' + a.sales_count + ')</span>' +
        '<span class="se-amount">' + (pendingText || t('Nothing pending','Nada pendiente','Nada pendente')) + '</span></div>' +
        '<div class="pr-amounts" id="payoutDetail_' + a.code + '"></div>' +
        '</div>';
    }).join('');
    founderPayoutsList.querySelectorAll('.payout-row').forEach(row => {
      row.addEventListener('click', () => togglePayoutDetail(row.getAttribute('data-code')));
    });
  }

  async function togglePayoutDetail(code){
    const detailEl = document.getElementById('payoutDetail_' + code);
    if(detailEl.classList.contains('payout-detail')){
      detailEl.innerHTML = '';
      detailEl.classList.remove('payout-detail');
      return;
    }
    detailEl.classList.add('payout-detail');
    detailEl.innerHTML = t('Loading...', 'Cargando...', 'Carregando...');
    const result = await apiCall('/api/admin', { email: founderEmail, password: founderPassword, action: 'affiliate-detail', code });
    if(!result.ok){
      detailEl.innerHTML = t('Couldn\'t load the detail.', 'No se pudo cargar el detalle.', 'Não foi possível carregar o detalhe.');
      return;
    }
    const pendingArsCount = result.sales.filter(s => s.currency === 'ARS' && s.status === 'pending').length;
    const pendingUsdCount = result.sales.filter(s => s.currency === 'USD' && s.status === 'pending').length;
    let html = result.sales.map(s => {
      const cls = s.status === 'paid' ? 'ps-paid' : '';
      return '<div class="payout-sale"><span>' + s.buyer_email + ' · ' + s.plan + '</span>' +
        '<span class="' + cls + '">' + money(s.commission_amount, s.currency) + (s.status === 'paid' ? ' ✓' : '') + '</span></div>';
    }).join('') || t('No sales yet.', 'Todavía no hay ventas.', 'Ainda não há vendas.');
    if(pendingArsCount > 0){
      html += '<button class="btn btn-ghost" data-code="' + code + '" data-currency="ARS">' + t('Mark ARS as paid', 'Marcar ARS como pagado', 'Marcar ARS como pago') + '</button>';
    }
    if(pendingUsdCount > 0){
      html += '<button class="btn btn-ghost" data-code="' + code + '" data-currency="USD">' + t('Mark USD as paid', 'Marcar USD como pagado', 'Marcar USD como pago') + '</button>';
    }
    detailEl.innerHTML = html;
    detailEl.querySelectorAll('button[data-currency]').forEach(btn => {
      btn.addEventListener('click', async () => {
        setPayoutsStatus('Updating...', 'Actualizando...', 'Atualizando...', 'info');
        const result2 = await apiCall('/api/admin', {
          email: founderEmail, password: founderPassword, action: 'mark-commissions-paid',
          code: btn.getAttribute('data-code'), currency: btn.getAttribute('data-currency'),
        });
        if(!result2.ok){
          setPayoutsStatus('Couldn\'t update.', 'No se pudo actualizar.', 'Não foi possível atualizar.', 'err');
          return;
        }
        setPayoutsStatus('Marked as paid.', 'Marcado como pagado.', 'Marcado como pago.', 'ok');
        loadPayoutsList();
      });
    });
  }

  founderPayoutsToggle.addEventListener('click', () => {
    payoutsLoaded = !payoutsLoaded;
    founderPayoutsToggle.textContent = payoutsLoaded
      ? t('Hide streamer payouts', 'Ocultar pagos a streamers', 'Ocultar pagamentos a streamers')
      : t('View streamer payouts', 'Ver pagos a streamers', 'Ver pagamentos a streamers');
    if(payoutsLoaded) loadPayoutsList();
    else founderPayoutsList.innerHTML = '';
  });

  accRegisterBtn.addEventListener('click', async () => {
    const email = accEmail.value.trim();
    const password = accPassword.value;
    if(!email || !password){ setStatus('Fill in your email and password.', 'Completá email y contraseña.', 'Preencha seu email e senha.', 'err'); return; }
    setStatus('Creating account...', 'Creando cuenta...', 'Criando conta...', 'info');
    const result = await apiCall('/api/register', { email, password });
    if(!result.ok){
      const messagesEn = {
        email_already_registered: 'That email is already registered log in instead.',
        password_too_short: 'The password needs at least 4 characters.',
      };
      const messagesEs = {
        email_already_registered: 'Ese email ya está registrado iniciá sesión.',
        password_too_short: 'La contraseña necesita al menos 4 caracteres.',
      };
      const messagesPt = {
        email_already_registered: 'Esse email já está cadastrado faça login.',
        password_too_short: 'A senha precisa ter pelo menos 4 caracteres.',
      };
      setStatus(
        messagesEn[result.error] || 'Couldn\'t register.',
        messagesEs[result.error] || 'No se pudo registrar.',
        messagesPt[result.error] || 'Não foi possível cadastrar.',
        'err'
      );
      return;
    }
    setStatus('Account created! Now tap "Log in".', '¡Cuenta creada! Ahora tocá "Iniciar sesión".', 'Conta criada! Agora toque em "Entrar".', 'ok');
  });

  accLogoutLink.addEventListener('click', (e) => {
    e.preventDefault();
    loggedInBlock.style.display = 'none';
    loginBlock.style.display = 'block';
    accEmail.value = '';
    accPassword.value = '';
    setStatus('', '', '', '');
    founderEmail = null;
    founderPassword = null;
    document.getElementById('founderPanel').style.display = 'none';
  });

  // ---------- Cookie consent banner ----------
  (function(){
    if(localStorage.getItem('solverix_cookie_consent')) return;
    const banner = document.getElementById('cookieBanner');
    document.getElementById('cookieBannerText').textContent = t(
      'This site uses cookies and local storage to remember your language and your session. See our Privacy Policy.',
      'Este sitio usa cookies y almacenamiento local para recordar tu idioma y tu sesión. Mirá nuestra Política de Privacidad.',
      'Este site usa cookies e armazenamento local para lembrar seu idioma e sua sessão. Veja nossa Política de Privacidade.'
    );
    document.getElementById('cookieAcceptBtn').textContent = t('Accept', 'Aceptar', 'Aceitar');
    banner.classList.add('show');
    document.getElementById('cookieAcceptBtn').addEventListener('click', () => {
      localStorage.setItem('solverix_cookie_consent', '1');
      banner.classList.remove('show');
    });
  })();

