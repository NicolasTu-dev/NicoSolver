# Modo Rápido — Design Spec

## Summary

Solverix's existing 6-step wizard ("Modo Avanzado") requires the user to configure both players' full ranges, board, bet sizes, pot, and stack — a workflow built for someone who already understands GTO solving. A real test with a beginner (the developer's father) showed the actual expectation: *"quiero cargar mis cartas y el flop, y que me diga qué hacer"* — enter my hand and the board, get a direct recommendation.

This spec adds **Modo Rápido**, a new primary entry point that collects only what a player actually knows at the table (their two hole cards, the board, and a plain-language description of the preflop action) and auto-fills everything else — opponent range, bet sizes, pot, stack — from a lookup table of common preflop situations. It reuses the existing solver pipeline and range-based engine untouched; it only changes how those inputs get populated.

## Goals

- A player who has never seen a solver can get "what do I do with this hand" in under 1 minute, without ever seeing the word "rango" or configuring a bet size.
- Modo Avanzado (existing 6-step wizard) is untouched and remains available for full manual control.
- The welcome screen presents Modo Rápido as the primary/first choice; Modo Avanzado as the secondary "for power users" choice.
- Villain ranges are approximated from commonly-taught standard opening/defending/3-betting guidelines — good enough as a starting assumption, not represented as verified professional charts. Expanding coverage later means adding rows to a data table, not touching UI or solver code.

## Non-goals

- Not a real-time / live-play assistant. This is the same "configure a spot, solve it, read the answer" offline tool Solverix already is — just with a faster on-ramp. (This distinction matters: real-time table assistance is out of scope for this product per earlier project decisions.)
- Not attempting full preflop-action coverage (every stack depth, every 3-bet/4-bet line, multiway pots). V1 covers 6 common heads-up preflop situations; more can be added later as pure data.
- Not changing anything about how Modo Avanzado or the underlying solver engine work.

## Architecture

Modo Rápido is a **thin front-end layer** that programmatically drives the exact same `QSolverJob` fields the existing Modo Avanzado wizard already sets from its widgets (`range_ip`, `range_oop`, `board`, per-street `StreetSetting`s, `ip_commit`/`oop_commit`, `stack`, etc. — see `mainwindow.cpp::on_buildTreeButtom_clicked()`), then calls the same build-tree and solve functions already wired up. No changes to `QSolverJob`, `GameTree`, or any solving code.

```
Welcome dialog
 ├─ "🚀 Modo Rápido" ──► Quick-mode step flow (3 steps, new)
 │                         1. Situación preflop (matchup picker)
 │                         2. Tus cartas (2-card picker, adapted from boardselector)
 │                         3. Cartas de la mesa (existing boardselector, reused as-is)
 │                        ──► auto-fill ip/oop range text, board text, bet sizes,
 │                            pot, stack from the chosen matchup's data row
 │                        ──► call existing on_buildTreeButtom_clicked() +
 │                            on_buttomSolve_clicked() (unchanged)
 │                        ──► Results screen: NEW "your hand" banner on top
 │                            (direct answer) + existing strategy grid below
 │
 └─ "🔧 Modo Avanzado" ──► existing 6-step wizard, unchanged
```

## Component 1: Matchup data table

A new header-only data file, `include/data/quickmoderanges.h`, defines:

```cpp
struct QuickModeMatchup {
    QString id;                  // stable key, e.g. "btn_open_bb_call"
    QString descriptionAsOpener; // shown when the user is the raiser
    QString descriptionAsCaller; // shown when the user is the caller/3-bettor's opponent
    QString openerPosition;      // e.g. "BTN"
    QString callerPosition;      // e.g. "BB"
    bool openerIsIP;             // true if the opener acts last postflop
    QString openerRange;
    QString callerRange;
    QString board;               // left empty; user fills this via step 3
    float pot;
    float effectiveStack;
};

QVector<QuickModeMatchup> getQuickModeMatchups();
```

`openerIsIP` matters because postflop position isn't always "whoever raised": when SB opens and BB calls, **BB acts last postflop** (SB is out of position despite being the preflop aggressor). Every matchup row must set this explicitly rather than assuming the opener is always IP.

### V1 matchup rows (6 situations, 12 user-facing options)

Ranges below are simplified, commonly-taught 100bb 6-max approximations (not verified solver-perfect charts) — a reasonable starting assumption for the solver to work from, exactly like a human solver-user would type in an estimated villain range. Two of them (BTN open vs BB call) reuse the exact strings already shipped as the app's default example range, for consistency.

1. **UTG open, BB calls** — `openerIsIP = true`
   - opener (UTG): `22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,AJo,AQo,AKo,A5s,A9s,ATs,AJs,AQs,AKs,KTs,KJs,KQs,KQo,QTs,QJs,QJo,JTs,T9s,98s,87s,76s,65s`
   - caller (BB): `22,33,44,55,66,77,88,99,TT,A9s,ATs,AJs,AQs,AQo:0.5,KQs,KJs,KQo:0.5,QJs,JTs,T9s,98s,87s,76s,65s`
   - pot: 50, stack: 200

2. **CO open, BB calls** — `openerIsIP = true`
   - opener (CO): `22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K9s,KTs,KJs,KQs,KTo:0.5,KQo,Q9s,QTs,QJs,QJo,J9s,JTs,T9s,T8s:0.5,98s,97s:0.5,87s,86s:0.5,76s,65s,54s:0.5`
   - caller (BB): `22,33,44,55,66,77,88,99,TT,JJ:0.5,A8s,A9s,ATs,AJs,AQs,AQo:0.5,K9s,KTs,KJs,KQs,KQo:0.5,Q9s,QTs,QJs,J9s,JTs,T9s,T8s,98s,87s,86s:0.5,76s,65s,54s:0.5`
   - pot: 50, stack: 200

3. **BTN open, BB calls** — `openerIsIP = true`
   - opener (BTN): `AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5`
   - caller (BB): `QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s`
   - pot: 50, stack: 200

4. **SB open, BB calls** — `openerIsIP = false` (BB is IP)
   - opener (SB): `22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A2o:0.5,A3o:0.5,A4o:0.5,A5o:0.5,A6o:0.5,A7o,A8o,A9o,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K5s:0.5,K6s,K7s,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo,Q7s:0.5,Q8s,Q9s,QTs,QJs,QTo:0.5,QJo,J8s,J9s,JTs,JTo:0.5,T8s,T9s,97s:0.5,98s,87s,76s,65s,54s,43s:0.5`
   - caller (BB): `22,33,44,55,66,77,88,99,TT,JJ,QQ:0.5,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs:0.5,A8o:0.5,A9o,ATo,AJo,AQo:0.5,K7s:0.5,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo:0.5,Q9s,QTs,QJs,QTo:0.5,J9s,JTs,JTo:0.5,T8s,T9s,98s,97s:0.5,87s,86s:0.5,76s,65s,54s,43s:0.5`
   - pot: 60, stack: 195

5. **BTN opens, BB 3-bets, BTN calls** (3-bet pot) — `openerIsIP = true`
   - opener/caller-of-3bet (BTN): `QQ,KK,AA:0.5,JJ:0.5,TT:0.5,AQs,AKs,AKo:0.5,AJs:0.5,KQs:0.5,ATs:0.3`
   - 3-bettor (BB): `AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,TT:0.3,KQs:0.3,A2s:0.3`
   - pot: 130, stack: 175

6. **CO opens, BTN 3-bets, CO calls** (3-bet pot) — `openerIsIP = false` (BTN is IP as 3-bettor)
   - 3-bettor (BTN): `AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,KQs:0.3,A2s:0.3`
   - opener/caller-of-3bet (CO): `QQ,KK,AA:0.5,JJ:0.5,TT:0.4,AQs,AKs,AKo:0.5,AJs:0.3`
   - pot: 130, stack: 175

All 6 rows use the app's existing default bet/raise sizing (50% bet, 60% raise, all streets, allin threshold 0.67, isomorphism on) — the same values already shipped as Modo Avanzado's defaults — so no new bet-sizing content is needed.

### The 12 user-facing options (step 1 of Modo Rápido)

Presented as a single scrollable list of plain-language buttons, grouped visually into "Bote simple" (rows 1-4) and "Con 3-bet" (rows 5-6):

- "Abriste desde UTG y el rival pagó desde la ciega grande" → row 1, user = opener
- "El rival abrió desde UTG y vos pagaste desde la ciega grande" → row 1, user = caller
- "Abriste desde el Cutoff (CO) y el rival pagó desde la ciega grande" → row 2, user = opener
- "El rival abrió desde el Cutoff y vos pagaste desde la ciega grande" → row 2, user = caller
- "Abriste desde el Botón (BTN) y el rival pagó desde la ciega grande" → row 3, user = opener
- "El rival abrió desde el Botón y vos pagaste desde la ciega grande" → row 3, user = caller
- "Abriste desde la ciega chica (SB) y el rival pagó desde la ciega grande" → row 4, user = opener
- "El rival abrió desde la ciega chica y vos pagaste desde la ciega grande" → row 4, user = caller
- "Abriste desde el Botón, el rival subió (3-bet) y vos pagaste" → row 5, user = opener/caller-of-3bet
- "El rival abrió desde el Botón, subiste (3-bet) y el rival pagó" → row 5, user = 3-bettor
- "Abriste desde el Cutoff, el rival subió (3-bet) desde el Botón y vos pagaste" → row 6, user = opener/caller-of-3bet
- "El rival abrió desde el Cutoff, subiste (3-bet) desde el Botón y el rival pagó" → row 6, user = 3-bettor

Selecting an option stores: the matchup row, and whether the user's hand goes into the "opener/caller-of-3bet" range slot or the "caller/3-bettor" range slot (this determines whether the user's exact hand — picked in step 2 — is looked up in `range_ip` or `range_oop` once solved, and which side of the board `ipRangeText`/`oopRangeText` gets the user's typical range vs their exact hand doesn't need injecting into the range text at all — see Component 3).

## Component 2: Two-card hand picker

Reuses `BoardSelectorTableModel`/`BoardSelectorTableDelegate`/`HtmlTableView` **directly** (no new class) — the same trio `boardselector` already uses for its popup grid, just embedded inline as a Modo Rápido step widget instead of inside a popup dialog, with a click handler that enforces "exactly 2 selected" (rejecting a 3rd click with a message instead of the popup's unlimited board-card selection). Outputs a `QString` like "As,Kh" via the model's existing `getBoardText()` — exactly the format the hand-lookup code already expects.

## Component 3: Auto-fill and solve

New `MainWindow::startQuickModeSolve(const QuickModeSelection& sel)`:

1. Look up the chosen `QuickModeMatchup` row.
2. Set `ui->ipRangeText` / `ui->oopRangeText` text to `openerRange`/`callerRange` (mapped by `openerIsIP`) — exactly like typing them into Modo Avanzado's step 1, just done in code.
3. Set `ui->boardText` to the board the user picked in step 3.
4. Set `ui->potText`, `ui->effectiveStackText`, and all per-street bet/raise fields to the matchup row's values (same defaults across all 6 rows, see above).
5. Call the existing `on_buildTreeButtom_clicked()` then `on_buttomSolve_clicked()` — unchanged.
6. On solve completion (reusing the existing `onSolverJobFinished` signal), open the results screen and pass it the user's exact 2 cards.

## Component 4: "Your hand" results banner

New widget added to the top of `strategyexplorer.ui`, above the existing grid — a `QLabel` styled like the existing `exampleBanner`/legend, populated once via `TableStrategyModel::get_strategy(row, col)` for the row/column matching the user's two cards (card→grid index mapping already exists in `RangeSelectorTableModel`/`TableStrategyModel` — same lookup the mouse-hover tooltip already does, just triggered once at load instead of on hover).

Text format: *"Con A♠K♥ en esta situación: Apostar 75% del pozo (68% de las veces) · Chequear (32%)"* — listing every action with non-zero frequency, sorted by frequency descending. Only shown when Modo Rápido launched the results screen (Modo Avanzado's results screen is unchanged — no forced hand selection there, since the user never picked "their exact hand" in that flow).

## Component 5: Welcome dialog changes

`WelcomeDialog` (`welcomedialog.ui`/`.cpp`) buttons change from `[Empezar de cero] [Ver ejemplo →]` to `[🔧 Modo Avanzado] [🚀 Modo Rápido]`, with Modo Rápido visually primary (filled/accent button, matching the existing `wizardNextButton` style) and Modo Avanzado secondary (outline style, matching `wizardBackButton`). The body text is rewritten to describe both paths instead of just explaining "ver ejemplo".

`WelcomeDialog::Choice` gains a third value `QuickMode` alongside the existing `ViewExample`/`StartFresh` (rename `ViewExample` role to `QuickMode`, drop the standalone "ver ejemplo" concept — Modo Rápido's guided nature replaces what "ver ejemplo" was trying to do). `MainWindow::on_helpButton_clicked()` branches to `startQuickModeFlow()` instead of `resetToExampleDefaults()`.

## Testing

No unit test infrastructure exists in this codebase (it's a Qt Widgets desktop app tested manually throughout this project). Verification is manual: build, launch, walk through all 12 Modo Rápido options end-to-end, confirm the solve completes and the hand banner shows a sane recommendation, confirm Modo Avanzado is unaffected.
