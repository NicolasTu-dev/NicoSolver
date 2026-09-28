  // Decorative mockup grids: filled procedurally so it reads as "real" data
  // without needing an actual screenshot.
  function paintGrid(id, cells){
    const el = document.getElementById(id);
    if(!el) return;
    const colors = ['#4fb8ff','#22c55e','#d4af37','#3d2f1a','#1c2f22'];
    const weights = [0.10,0.22,0.30,0.20,0.18];
    for(let i=0;i<cells;i++){
      const r = Math.random();
      let acc=0, chosen=colors[0];
      for(let j=0;j<weights.length;j++){ acc+=weights[j]; if(r<=acc){chosen=colors[j]; break;} }
      const d = document.createElement('div');
      d.style.background = chosen;
      d.style.opacity = (0.55 + Math.random()*0.45).toFixed(2);
      d.style.aspectRatio = '1/1';
      el.appendChild(d);
    }
  }
  paintGrid('grid-mock', 32);

  // ---------- Language switch (EN default, ES optional) ----------
  const ES = {
    'nav.how': 'Cómo funciona', 'nav.tool': 'La herramienta', 'nav.pricing': 'Precios', 'nav.faq': 'Preguntas',
    'nav.account': 'Mi cuenta',
    'hero.eyebrow': '♣ Solver GTO en español ♦',
    'hero.title': 'Dejá de adivinar.<br>Jugá la mano que <em>gana en el largo plazo</em>.',
    'hero.lead': 'Cargá tu mano y el board, y Solverix te muestra en segundos qué hace un jugador matemáticamente equilibrado en esa situación exacta sin conjeturas, sin "sensación".',
    'hero.ctaPrimary': 'Empezar ahora', 'hero.ctaSecondary': 'Ver cómo funciona',
    'hero.note': 'Sin tarjeta para probar la demo · Cancelás cuando quieras',
    'hero.solvedIn': 'Resuelto en', 'hero.recLabel': 'Jugada recomendada', 'hero.recVerdict': 'Apostar 33% del pozo',
    'hero.recWhy': 'Value bet: tu mano le gana a la mayoría del rango del rival y todavía podés mejorar.',
    'benefits.eyebrow': 'Por qué Solverix', 'benefits.title': 'Un solver de verdad, pensado para jugar no para programar',
    'benefits.subtitle': 'La mayoría de los solvers están hechos para gente que ya sabe usar un solver. Este no.',
    'benefits.card1Title': 'Teoría de juegos real',
    'benefits.card1Body': 'Basado en CFR (Minimización de Arrepentimiento Contrafactual), el algoritmo que converge hacia el equilibrio de Nash la estrategia matemáticamente inexplotable corriendo enteramente en tu propia computadora, sin depender de la nube.',
    'benefits.card2Title': 'Multilingüe de fábrica',
    'benefits.card2Body': 'Cada pantalla, explicación y término técnico disponible en inglés, español o portugués cambiás cuando quieras con un click.',
    'benefits.card3Title': 'Respuestas en segundos',
    'benefits.card3Body': 'Elegís la situación y la mano, y el resultado aparece casi al toque no hace falta esperar minutos para una respuesta.',
    'how.eyebrow': 'Cómo funciona', 'how.title': 'Tres pasos, una respuesta clara',
    'how.step1Title': 'Elegís la situación', 'how.step1Body': 'Posición tuya, posición del rival, tu mano y el board. Nada de escribir rangos a mano.',
    'how.step2Title': 'Solverix calcula', 'how.step2Body': 'El motor arma el árbol de decisiones y resuelve la estrategia óptima para esa mano exacta.',
    'how.step3Title': 'Te dice qué hacer', 'how.step3Body': 'Retirarte, pagar o apostar con el porcentaje de frecuencia y el motivo, en una frase que se entiende.',
    'gallery.eyebrow': 'La herramienta', 'gallery.title': 'Así se ve por dentro',
    'gallery.subtitle': 'Vista previa del Solver Avanzado y del explorador de estrategia.',
    'gallery.caption1': 'Árbol de decisiones y EV por combo Solver Avanzado',
    'gallery.caption2': 'Paso 1 Elegís tu asiento', 'gallery.caption3': 'Paso 4 La respuesta, directa',
    'pricing.eyebrow': 'Precios', 'pricing.title': 'Empezá simple. Sumá el Modo Rápido cuando quieras.',
    'pricing.subtitle': 'Dos formas de usar Solverix, según cuánto querés meterte en el detalle.',
    'pricing.plan1Name': 'Solver Avanzado',
    'pricing.plan1Tag': 'Para analizar tus manos con control total del rango, el board y las apuestas ideal si ya jugás con frecuencia.',
    'pricing.plan1Price': '$38.500',
    'pricing.currency': 'ARS / mes',
    'pricing.plan1Feat1': 'Solver GTO completo, sin límite de manos',
    'pricing.plan1Feat2': 'Rangos, tamaños de apuesta y stacks 100% editables',
    'pricing.plan1Feat3': 'Explorador de estrategia con EV y razones de cada jugada',
    'pricing.plan1Feat4': '5 temas visuales, en inglés, español o portugués',
    'pricing.plan1Feat5': 'Modo Rápido (situaciones precargadas)',
    'pricing.plan1Feat6': 'Modo Práctica (entrená adivinando)',
    'pricing.plan1Cta': 'Adquirir Avanzado',
    'pricing.badge': 'Recomendado', 'pricing.plan2Name': 'Completo',
    'pricing.plan2Tag': 'Todo el Avanzado, más el Modo Rápido y el Modo Práctica ideal si estás empezando o jugás con alguien que recién arranca.',
    'pricing.plan2Price': '$49.000',
    'pricing.plan2Feat1': 'Todo lo del Solver Avanzado',
    'pricing.plan2Feat2': 'Modo Rápido: elegís posición y mano, listo',
    'pricing.plan2Feat3': '6-max y Full Ring precargados',
    'pricing.plan2Feat4': 'Modo Práctica para entrenar antes de jugar',
    'pricing.plan2Feat5': 'Glosario de términos en criollo',
    'pricing.plan2Feat6': 'Soporte prioritario',
    'pricing.plan2Cta': 'Adquirir Completo',
    'pricing.note': 'Precios de ejemplo ver más abajo. Cancelás cuando quieras, sin permanencia.',
    'audience.eyebrow': 'Para quién es', 'audience.title': 'Sirve distinto según dónde estés parado',
    'audience.tag1': 'Jugador con experiencia', 'audience.title1': 'Solver Avanzado',
    'audience.body1': 'Armás vos los rangos exactos de cada jugador, ajustás los tamaños de apuesta a como se juega en tu mesa, y revisás manos puntuales después de la sesión.',
    'audience.quote1': '"Lo uso para revisar las manos grandes de la semana, no para jugar en vivo."',
    'audience.quoteLabel': 'uso típico',
    'audience.tag2': 'Recién empezás', 'audience.title2': '+ Modo Rápido',
    'audience.body2': 'Elegís tu posición, tu mano y el board de una lista simple, y Solverix arma los rangos típicos por vos. Sin tener que entender rangos todavía.',
    'audience.quote2': '"Toco 3 botones y me dice qué hacer. Después entiendo el por qué."',
    'faq.eyebrow': 'Preguntas frecuentes', 'faq.title': 'Antes de sumarte',
    'faq.q1': '¿Necesito saber de teoría de juegos para usarlo?',
    'faq.a1': 'No. El Modo Rápido está pensado para que no necesites saber qué es un rango o un GTO para recibir una respuesta clara. El Avanzado sí asume que ya jugás con cierta frecuencia.',
    'faq.q2': '¿Puedo pasar del plan Avanzado al Completo después?',
    'faq.a2': 'Sí, en cualquier momento. Tus manos y configuraciones guardadas se mantienen igual.',
    'faq.q3': '¿Funciona sin conexión a internet?',
    'faq.a3': 'El cálculo corre en tu computadora, así que no depende de un servidor externo para resolver una mano.',
    'faq.q4': '¿Cuánto tarda en resolver una mano?',
    'faq.a4': 'Segundos, no minutos. Podés elegir entre una respuesta rápida y aproximada, o una más precisa si tenés tiempo de sobra.',
    'finalCta.title': 'Andá a la próxima mesa sabiendo qué hacer',
    'finalCta.body': 'Probá Solverix y dejá de jugar de memoria.',
    'finalCta.cta': 'Ver planes',
    'footer.privacy': 'Política de Privacidad',
    'footer.terms': 'Términos de Servicio',
  };

  const PT = {
    'nav.how': 'Como funciona', 'nav.tool': 'A ferramenta', 'nav.pricing': 'Preços', 'nav.faq': 'Perguntas',
    'nav.account': 'Minha conta',
    'hero.eyebrow': '♣ Solver GTO em português ♦',
    'hero.title': 'Pare de chutar.<br>Jogue a mão que <em>ganha no longo prazo</em>.',
    'hero.lead': 'Carregue sua mão e o board, e o Solverix mostra em segundos o que um jogador matematicamente equilibrado faz naquela situação exata sem achismo, sem "feeling".',
    'hero.ctaPrimary': 'Começar agora', 'hero.ctaSecondary': 'Ver como funciona',
    'hero.note': 'Sem cartão para testar a demo · Cancele quando quiser',
    'hero.solvedIn': 'Resolvido em', 'hero.recLabel': 'Jogada recomendada', 'hero.recVerdict': 'Apostar 33% do pote',
    'hero.recWhy': 'Value bet: sua mão vence a maior parte do range do adversário e ainda pode melhorar.',
    'benefits.eyebrow': 'Por que o Solverix', 'benefits.title': 'Um solver de verdade, feito para jogar não para programar',
    'benefits.subtitle': 'A maioria dos solvers é feita para quem já sabe usar um solver. Este não.',
    'benefits.card1Title': 'Teoria dos jogos de verdade',
    'benefits.card1Body': 'Baseado em CFR (Minimização de Arrependimento Contrafactual), o algoritmo que converge para o equilíbrio de Nash a estratégia matematicamente inexplorável rodando inteiramente no seu próprio computador, sem depender da nuvem.',
    'benefits.card2Title': 'Multilíngue de fábrica',
    'benefits.card2Body': 'Cada tela, explicação e termo técnico disponível em inglês, espanhol ou português troque quando quiser com um clique.',
    'benefits.card3Title': 'Respostas em segundos',
    'benefits.card3Body': 'Escolha a situação e a mão, e o resultado aparece quase na hora sem esperar minutos por uma resposta.',
    'how.eyebrow': 'Como funciona', 'how.title': 'Três passos, uma resposta clara',
    'how.step1Title': 'Escolha a situação', 'how.step1Body': 'Sua posição, a posição do adversário, sua mão e o board. Nada de digitar ranges na mão.',
    'how.step2Title': 'O Solverix calcula', 'how.step2Body': 'O motor monta a árvore de decisão e resolve a estratégia ótima para aquela mão exata.',
    'how.step3Title': 'Ele te diz o que fazer', 'how.step3Body': 'Foldar, pagar ou apostar com o percentual de frequência e o motivo, numa frase que faz sentido.',
    'gallery.eyebrow': 'A ferramenta', 'gallery.title': 'Veja o que tem por dentro',
    'gallery.subtitle': 'Uma prévia do Solver Avançado e do explorador de estratégia.',
    'gallery.caption1': 'Árvore de decisão e EV por combo Solver Avançado',
    'gallery.caption2': 'Passo 1 Escolha seu assento', 'gallery.caption3': 'Passo 4 A resposta, direto ao ponto',
    'pricing.eyebrow': 'Preços', 'pricing.title': 'Comece simples. Adicione o Modo Rápido quando quiser.',
    'pricing.subtitle': 'Duas formas de usar o Solverix, dependendo do quanto você quer se aprofundar.',
    'pricing.plan1Name': 'Solver Avançado',
    'pricing.plan1Tag': 'Analise suas mãos com controle total do range, do board e das apostas ideal se você já joga com regularidade.',
    'pricing.plan1Price': '$25',
    'pricing.currency': 'USD / mês',
    'pricing.plan1Feat1': 'Solver GTO completo, sem limite de mãos',
    'pricing.plan1Feat2': 'Ranges, tamanhos de aposta e stacks 100% editáveis',
    'pricing.plan1Feat3': 'Explorador de estratégia com EV e os motivos de cada jogada',
    'pricing.plan1Feat4': '5 temas visuais, em inglês, espanhol ou português',
    'pricing.plan1Feat5': 'Modo Rápido (situações pré-carregadas)',
    'pricing.plan1Feat6': 'Modo Prática (treine adivinhando)',
    'pricing.plan1Cta': 'Adquirir Avançado',
    'pricing.badge': 'Recomendado', 'pricing.plan2Name': 'Completo',
    'pricing.plan2Tag': 'Tudo do Avançado, mais o Modo Rápido e o Modo Prática ideal se você está começando ou joga com alguém que está começando.',
    'pricing.plan2Price': '$32',
    'pricing.plan2Feat1': 'Tudo do Solver Avançado',
    'pricing.plan2Feat2': 'Modo Rápido: escolha posição e mão, pronto',
    'pricing.plan2Feat3': '6-max e Full Ring pré-carregados',
    'pricing.plan2Feat4': 'Modo Prática para treinar antes de jogar',
    'pricing.plan2Feat5': 'Glossário de termos em linguagem simples',
    'pricing.plan2Feat6': 'Suporte prioritário',
    'pricing.plan2Cta': 'Adquirir Completo',
    'pricing.note': 'Preços de exemplo veja mais abaixo. Cancele quando quiser, sem fidelidade.',
    'audience.eyebrow': 'Para quem é', 'audience.title': 'Serve diferente dependendo de onde você está',
    'audience.tag1': 'Jogador experiente', 'audience.title1': 'Solver Avançado',
    'audience.body1': 'Você monta os ranges exatos de cada jogador, ajusta os tamanhos de aposta de acordo com sua mesa e revisa mãos específicas depois da sessão.',
    'audience.quote1': '"Uso para revisar as mãos grandes da semana, não para jogar ao vivo."',
    'audience.quoteLabel': 'uso típico',
    'audience.tag2': 'Começando agora', 'audience.title2': '+ Modo Rápido',
    'audience.body2': 'Escolha sua posição, sua mão e o board de uma lista simples, e o Solverix monta os ranges típicos para você. Sem precisar entender ranges ainda.',
    'audience.quote2': '"Toco 3 botões e ele me diz o que fazer. Depois eu entendo o porquê."',
    'faq.eyebrow': 'Perguntas frequentes', 'faq.title': 'Antes de entrar',
    'faq.q1': 'Preciso saber teoria dos jogos para usar?',
    'faq.a1': 'Não. O Modo Rápido foi pensado para que você não precise saber o que é um range ou um GTO para ter uma resposta clara. O Avançado já assume que você joga com alguma regularidade.',
    'faq.q2': 'Posso passar do plano Avançado para o Completo depois?',
    'faq.a2': 'Sim, a qualquer momento. Suas mãos e configurações salvas continuam as mesmas.',
    'faq.q3': 'Funciona sem conexão com a internet?',
    'faq.a3': 'O cálculo roda no seu computador, então não depende de um servidor externo para resolver uma mão.',
    'faq.q4': 'Quanto tempo leva para resolver uma mão?',
    'faq.a4': 'Segundos, não minutos. Você pode escolher entre uma resposta rápida e aproximada, ou uma mais precisa se tiver tempo sobrando.',
    'finalCta.title': 'Chegue na próxima mesa sabendo o que fazer',
    'finalCta.body': 'Experimente o Solverix e pare de jogar de memória.',
    'finalCta.cta': 'Ver planos',
    'footer.privacy': 'Política de Privacidade',
    'footer.terms': 'Termos de Serviço',
  };

  const EN = {};
  document.querySelectorAll('[data-i18n]').forEach(el => {
    EN[el.getAttribute('data-i18n')] = el.innerHTML;
  });

  const DICTS = { en: EN, es: ES, pt: PT };

  function applyLang(lang){
    const dict = DICTS[lang] || EN;
    document.querySelectorAll('[data-i18n]').forEach(el => {
      const key = el.getAttribute('data-i18n');
      if(dict[key] !== undefined) el.innerHTML = dict[key];
    });
    document.documentElement.lang = lang;
    document.getElementById('langSwitch').value = lang;
    localStorage.setItem('solverix_lang', lang);
  }

  const savedLang = localStorage.getItem('solverix_lang') || 'en';
  if(savedLang !== 'en') applyLang(savedLang);
  else document.getElementById('langSwitch').value = 'en';
  document.getElementById('langSwitch').addEventListener('change', (e) => {
    applyLang(e.target.value);
  });

  // ---------- Cookie consent banner ----------
  (function(){
    const COOKIE_TEXT = {
      en: { text: 'This site uses cookies and local storage to remember your language and your session. See our Privacy Policy.', btn: 'Accept' },
      es: { text: 'Este sitio usa cookies y almacenamiento local para recordar tu idioma y tu sesión. Mirá nuestra Política de Privacidad.', btn: 'Aceptar' },
      pt: { text: 'Este site usa cookies e armazenamento local para lembrar seu idioma e sua sessão. Veja nossa Política de Privacidade.', btn: 'Aceitar' },
    };
    if(localStorage.getItem('solverix_cookie_consent')) return;
    const banner = document.getElementById('cookieBanner');
    const lang = localStorage.getItem('solverix_lang') || 'en';
    const set = COOKIE_TEXT[lang] || COOKIE_TEXT.en;
    document.getElementById('cookieBannerText').textContent = set.text;
    document.getElementById('cookieAcceptBtn').textContent = set.btn;
    banner.classList.add('show');
    document.getElementById('cookieAcceptBtn').addEventListener('click', () => {
      localStorage.setItem('solverix_cookie_consent', '1');
      banner.classList.remove('show');
    });
  })();
