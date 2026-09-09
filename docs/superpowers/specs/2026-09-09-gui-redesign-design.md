# Rediseño de GUI + Español — TexasSolver

Fecha: 2026-09-09
Estado: Aprobado por usuario, pendiente de plan de implementación

## Contexto

TexasSolver es un solver GTO de poker open source (AGPL-3.0, `bupticybee/TexasSolver`). El motor de cálculo (CFR, árbol de juego, evaluación de manos) vive en `src/solver`, `src/GameTree.cpp`, `src/nodes/`, `src/ranges/`, `src/compairer/` y **no se modifica** en este proyecto.

El uso previsto es privado (el usuario y su padre, para testear), por lo que no aplican las obligaciones de publicación de código de la AGPL (esas solo se activan al distribuir a terceros u ofrecer el software como servicio de red a otros usuarios).

Problemas actuales identificados:
- La GUI (`mainwindow.ui`) apila toda la configuración inicial (rangos IP/OOP, board, bet sizes de 6 secciones, stacks, pot, solver options) en una sola pantalla larga, sin guía.
- La pantalla de resultados (`strategyexplorer.cpp`, modelos en `include/ui/`) es difícil de interpretar sin conocimiento previo.
- Solo hay traducciones a inglés y chino (`lang_en.ts`, `lang_cn.ts`, cargadas vía `QTranslator` en `main.cpp`).
- La apariencia visual es la de Qt Widgets sin estilizar ("pantalla blanca fea").

## Objetivo

Rediseñar la experiencia de la app de escritorio (Qt Widgets) en tres frentes — tema visual, flujo de configuración, pantalla de resultados — y agregar español como tercer idioma. Todo esto sin tocar la lógica de cálculo del solver.

## Diseño

### 1. Sistema de temas (QSS)

- Dos hojas de estilo Qt (QSS): `theme_dark.qss` (Dark Modern) y `theme_light.qss` (Clean Light), cubriendo paleta, tipografía, radios de borde y estados hover/pressed/disabled/focus para todos los widgets usados en la app (botones, inputs, tablas/vistas de rango, tabs, scrollbars, tooltips).
- Paleta Dark Modern (referencia, ajustable en implementación):
  - Fondos: `#12141c` (base), `#0d0f16` (paneles/nav)
  - Tarjetas/paneles: `#1a1d29`, bordes `#2a2d3a`
  - Texto: `#e8e8f0` (primario), `#8b8fa3` (secundario)
  - Acento primario (botones, progreso): `#5b6ef5`
  - Semáforo de acciones de poker: call `#3ddc84`, raise `#ffb44f`, fold `#ff5c5c`
- Set de iconos SVG monocromos para toolbar, navegación del wizard y settings; se recolorean vía QSS según el tema activo, sin duplicar assets.
- Selección de tema persistida en `QSettings`; se aplica con `qApp->setStyleSheet(...)` al arrancar y al cambiar desde la configuración. Patrón de carga vía Qt resource system (`.qrc`), igual que las traducciones existentes.
- Tema por defecto: Dark Modern.

### 2. Wizard de configuración inicial

- Reemplaza la pantalla única actual por un flujo guiado paso a paso con barra de progreso. Pasos:
  1. Posiciones
  2. Rangos (IP/OOP)
  3. Board
  4. Stacks / Pot
  5. Bet sizings
  6. Confirmar y resolver
- Cada paso incluye: título, subtítulo explicativo en lenguaje simple, tip contextual cuando aplique, navegación "Atrás / Siguiente" fija abajo.
- Los datos se acumulan en la estructura de configuración ya existente en el backend (`GameTreeBuildingSettings`) — el wizard la va completando incrementalmente en vez de requerir todo junto.
- Atajo "cargar configuración guardada" disponible desde el primer paso, para que usuarios recurrentes no repitan el flujo completo cada vez.
- Implementación técnica (QWizard nativo de Qt vs QStackedWidget + barra de progreso custom) se decide en la fase de plan, evaluando cuál se integra mejor con los widgets de selección de rango/board ya existentes (`rangeselector.cpp`, `boardselector.cpp`).

### 3. Pantalla de resultados

- Layout de 3 columnas dentro de la ventana principal (post-resolución):
  - **Izquierda**: árbol de decisión navegable (calles y acciones), reemplazando/reorganizando la navegación actual de `strategyexplorer.cpp`.
  - **Centro**: matriz de manos 13x13 con leyenda de colores fija (call/raise/fold) siempre visible.
  - **Derecha**: toggle Simple ⇄ Avanzado, e indicador de explotabilidad (% del pot).
- Tooltip al pasar el mouse sobre una mano de la matriz: combo, % de cada acción, EV, equity.
- Modo "Simple" (default): solo frecuencias de acción por color, sin números de EV/equity.
- Modo "Avanzado": toda la información numérica disponible hoy (EV, equity, combos exactos).
- Los modelos de datos existentes (`tablestrategymodel`, `roughstrategyviewermodel`, `detailviewermodel`, etc. en `include/ui/`) se reutilizan/adaptan; no se reescribe el cálculo de estrategias, solo su presentación.

### 4. Español

- Nuevo archivo fuente `lang_es.ts`, compilado a `lang_es.qm`.
- Se agrega a `translations.qrc` (junto a `lang_cn.qm`, `lang_en.qm`) y a la lista `TRANSLATIONS` en `TexasSolverGui.pro`.
- Se sigue el patrón exacto ya usado en `main.cpp` para `lang_en`/`lang_cn` al cargar el `QTranslator`.
- Todo string nuevo introducido por el wizard y la pantalla de resultados rediseñada se escribe envuelto en `tr()` desde el inicio.
- El selector de idioma ya existente en Settings gana una tercera opción: Español.

## Fuera de alcance

- Cualquier cambio a `src/solver/`, `src/GameTree.cpp`, `src/nodes/`, `src/ranges/`, `src/compairer/` — la lógica de cálculo no se toca.
- Asistencia en tiempo real durante manos reales o cualquier integración con GGPoker u otro sitio de poker online. Este proyecto es exclusivamente la app de escritorio para análisis offline/pre-sesión.
- Empaquetado, licenciamiento comercial o mecanismo de venta/distribución — decisión futura, fuera de este spec.
- Uso multiplataforma (Linux/Mac) más allá de lo que ya soporta el proyecto — el foco es la build de Windows que el usuario ya tiene.

## Testing / validación

- Validación visual manual: correr la app con cada tema y confirmar legibilidad/contraste en todos los pasos del wizard y en la pantalla de resultados.
- Confirmar que cambiar de tema en caliente (sin reiniciar) no rompe ningún widget.
- Confirmar que el wizard produce exactamente la misma estructura de configuración que hoy genera la pantalla única (mismo `GameTreeBuildingSettings`), corriendo un caso de prueba conocido y comparando el árbol de juego resultante contra la versión actual sin rediseñar.
- Cambiar el idioma a Español y verificar que no queden strings sin traducir (barrido visual de cada pantalla).
