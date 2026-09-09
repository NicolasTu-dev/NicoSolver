# Results Screen Contextual Explanations Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the `StrategyExplorer` results screen self-explanatory: add a fixed color legend, a real hover tooltip (combo/%/EV/equity), and a toggle to show/hide EV-level detail — without touching solver logic or restructuring the existing tree/matrix/detail-panel architecture, which already works.

**Architecture:** This app already has almost all the data plumbing this feature needs — `TableStrategyModel` is the single source of truth the matrix, the rough-strategy bar, and the detail panel all read from, and a custom hover-detection mechanism (`HtmlTableView::itemMouseChange(int,int)`) already exists and drives the existing `detailView` side panel. This plan adds a real `QToolTip` popup on the same hover signal (additive, doesn't remove the existing side panel), a new legend widget, and a new checkbox that gates whether EV/equity numbers render. No new data-access pattern, no solver-file changes, no rewrite of the existing 5-button view-mode system (`ipRangeButtom`/`oopRangeButtom`/`strategyModeButtom`/`evModeButtom`/`evOnlyModeButtom` stay exactly as they are).

**Tech Stack:** Qt 6 Widgets, QToolTip, QSS (extends the theme system from `feature/theme-system`).

## Global Constraints

- Do not modify any file under `src/solver/`, `src/GameTree.cpp`, `src/nodes/`, `src/ranges/`, `src/compairer/`. This explicitly rules out plumbing `BestResponse::printExploitability`'s result (computed inside `src/solver/PCfrSolver.cpp`) through to the GUI — **the exploitability indicator from the original design spec is deliberately deferred, not built in this plan.** See Self-Review Notes for the reasoning.
- Do not change the behavior of the existing 5 view-mode buttons, `TableStrategyModel`, `RoughStrategyViewerModel`, `DetailViewerModel`, or any existing delegate's *current* painting logic for the states it already handles — this plan only *adds* a tooltip, a legend, and an EV-visibility flag that existing code checks, it doesn't rewrite existing paint methods' core behavior.
- Every new user-visible string wrapped in `tr()`.
- This branch must be created from `feature/config-wizard` (which itself sits on `feature/theme-system`) so the QSS theme system and wizard are both present — `git checkout -b feature/results-screen feature/config-wizard`.
- Build/verify command (adapt to Bash if preferred):
  ```
  powershell -Command "$env:PATH='D:\QT\6.11.2\mingw_64\bin;D:\QT\Tools\mingw1310_64\bin;' + $env:PATH; if (!(Test-Path build-results)) { New-Item -ItemType Directory build-results | Out-Null }; cd build-results; & 'D:\QT\6.11.2\mingw_64\bin\qmake.exe' ..\TexasSolverGui.pro; mingw32-make -j4"
  ```
  Expected: no `error:` lines.
- Launch/verify: copy `Qt6Core.dll`/`Qt6Gui.dll`/`Qt6Widgets.dll` from `D:\QT\6.11.2\mingw_64\bin`, `libgomp-1.dll`/`libstdc++-6.dll`/`libwinpthread-1.dll`/`libgcc_s_seh-1.dll` from `D:\QT\Tools\mingw1310_64\bin`, and `platforms\qwindows.dll` from `D:\QT\6.11.2\mingw_64\plugins\platforms\` into the exe's folder if launch fails with a missing-DLL error.
- No automated test suite exists. Verification is build success + manual/UI-automation interaction check, including actually running a solve (via the wizard, using a trivial valid range/board) and opening `StrategyExplorer` to see real data, not just launching to an empty dialog.

---

## File Structure

- Modify: `strategyexplorer.ui` — add a legend widget (`resultsLegend`) and a `QCheckBox` (`advancedModeCheck`)
- Modify: `strategyexplorer.h` — declare hover-tooltip handling additions and the new checkbox's slot
- Modify: `strategyexplorer.cpp` — wire the tooltip into the existing `onMouseMoveEvent` handler, wire the checkbox
- Modify: `include/ui/detailwindowsetting.h`, `src/ui/detailitemdelegate.cpp`, `src/ui/roughstrategyitemdelegate.cpp` — thread an `advanced_mode` bool through so EV/equity text is conditionally rendered (only if these files actually need it — confirm during Task 3, see that task's note)
- Modify: `resources/themes/theme_dark.qss`, `resources/themes/theme_light.qss` — style the new legend and checkbox

---

### Task 1: Color legend widget

**Files:**
- Modify: `strategyexplorer.ui`

**Interfaces:**
- Produces: a `QWidget` named `resultsLegend` containing 3 labeled color swatches, placed visibly near `strategyTableView` (top of the left or center column — read the current `.ui` to decide the least disruptive insertion point, e.g. directly above `strategyTableView` inside `verticalLayout`, per the structure found in research: `verticalLayout` at `strategyexplorer.ui:18` holds the "Game Tree" groupbox then the turn/river combo row then `strategyTableView` — insert the legend between the combo row and `strategyTableView`).

**Before you begin:** Read the current `strategyexplorer.ui` in full to confirm the exact current structure around `strategyTableView` before inserting anything — the plan gives you the target (a legend widget placed immediately above the matrix) and the known real color values to use (from prior codebase research, `src/ui/strategyitemdelegate.cpp`): fold color is `QColor(0,191,255)` (a cyan), call/check color is a green-dominant gradient (`green=255` fixed, base swatch color can just show pure `rgb(55,255,55)` as the representative "call" swatch), bet/raise color is a red-dominant gradient (representative swatch `rgb(255,128,128)`). These are the ACTUAL hardcoded colors already used in the matrix — the legend must match them exactly, not invent new ones, or it will mislabel the real colors on screen.

- [ ] **Step 1: Add the legend widget**

Add a `QWidget` named `resultsLegend` to `strategyexplorer.ui`, positioned as described above. Structure: a `QHBoxLayout` containing three `QWidget`s (or `QFrame`s), each pairing a small fixed-size color swatch with a `QLabel`:
- Swatch color `rgb(0,191,255)` + label text `Fold` (wrapped in `tr()`)
- Swatch color `rgb(55,255,55)` + label text `Call / Check` (wrapped in `tr()`)
- Swatch color `rgb(255,128,128)` + label text `Bet / Raise` (wrapped in `tr()`)

Give each swatch widget a distinct objectName (`legendSwatchFold`, `legendSwatchCall`, `legendSwatchBet`) so Task 4 can style their fixed background color directly (via `.ui`-set `styleSheet` property on each swatch is acceptable here since these are fixed reference colors that must NOT change with the app theme — they must always show the real matrix colors regardless of dark/light mode, so this is one of the rare legitimate uses of an inline per-widget stylesheet rather than the shared QSS files).

- [ ] **Step 2: Build**

Run the build command from Global Constraints. Expected: no `error:` lines.

- [ ] **Step 3: Manual verification**

Launch the app, run a trivial solve via the wizard (e.g. range "AA" both players, a 3-card board), open the results screen (`showResultButton`). Confirm the legend appears above the matrix with 3 correctly colored swatches and readable labels in both the dark and light theme (switch via Settings, relaunch, re-check — the swatches should NOT change color between themes, only their surrounding label text/background should follow the theme).

- [ ] **Step 4: Commit**

```bash
git add strategyexplorer.ui
git commit -m "Add fixed color legend to results screen"
```

---

### Task 2: Real hover tooltip on the strategy matrix

**Files:**
- Modify: `strategyexplorer.cpp` — extend the existing `onMouseMoveEvent(int,int)` slot (per prior research, at `strategyexplorer.cpp:225-230`, connected from `HtmlTableView`'s `itemMouseChange(int,int)` signal at `strategyexplorer.cpp:87`)

**Interfaces:**
- Consumes: `TableStrategyModel`'s existing public data-access methods (per prior research, declared in `include/ui/tablestrategymodel.h:37-52` — includes `get_strategy`, `get_strategies_evs`, `get_ev_grid`, plus `p1_range`/`p2_range` and `current_strategy`/`current_evs` members). **Read the actual header to get exact signatures (parameter types, return types) before writing the tooltip-formatting code — do not guess these signatures.** Also read `src/ui/detailitemdelegate.cpp`'s `paint_strategy`/`paint_evs` methods (per research, around lines 117-138 and 275-343) to see the exact existing pattern for turning a `(row,col)` cell into combo/%/EV text — the tooltip should format equivalent information, reusing the same underlying calls, not inventing a new data path.
- Produces: a `QToolTip::showText(...)` call triggered on hover, shown alongside (not replacing) the existing `detailView` panel update that already happens in this slot.

- [ ] **Step 1: Read the real signatures and existing formatting logic**

Read `include/ui/tablestrategymodel.h` in full and `src/ui/detailitemdelegate.cpp`'s strategy/EV text-building code. Identify exactly how an existing method turns `(grid_i, grid_j)` into: the hand's combo notation (e.g. "AKs"), the per-action percentages, the EV number, and the equity number (if a separate equity value exists — if `DetailItemDelegate` only shows EV and not a distinct "equity" figure, use what's actually available rather than fabricating an equity number that doesn't exist in this codebase; note this in your report).

- [ ] **Step 2: Implement the tooltip**

In `strategyexplorer.cpp`'s `onMouseMoveEvent(int i, int j)` (or wherever the hover slot actually is — confirm exact name/location first), after the existing detail-panel-refresh logic, add a call to build a plain-text or rich-text tooltip string using the data access confirmed in Step 1, and call `QToolTip::showText(QCursor::pos(), tooltipText, this->ui->strategyTableView)`. Guard against invalid/out-of-range `(i,j)` the same way the existing detail-panel code already guards (reuse its bounds-checking pattern if any exists — read it first).

- [ ] **Step 3: Build**

Run the build command from Global Constraints. Expected: no `error:` lines.

- [ ] **Step 4: Manual verification**

Launch, solve, open results. Hover over several different cells in `strategyTableView` (a hand with a clear strategy, a folded hand, an empty/diagonal cell) and confirm a tooltip appears near the cursor with sensible combo/%/EV text that matches what the existing `detailView` panel shows for the same cell (cross-check the two are consistent — if the tooltip's numbers disagree with the panel's numbers, something in Step 2 read the wrong data).

- [ ] **Step 5: Commit**

```bash
git add strategyexplorer.cpp
git commit -m "Add hover tooltip with combo/EV detail on the strategy matrix"
```

---

### Task 3: Simple/Advanced EV visibility toggle

**Files:**
- Modify: `strategyexplorer.ui` — add `QCheckBox advancedModeCheck` (text "Modo avanzado (mostrar EV)", wrapped in `tr()`), placed near the 5 existing mode buttons
- Modify: `strategyexplorer.h`, `strategyexplorer.cpp` — add `bool advancedMode = false;` state and a slot `on_advancedModeCheck_toggled(bool checked)` that stores it and triggers a repaint (`ui->strategyTableView->update(); ui->detailView->update();` or whatever the existing repaint pattern is, per `item_clicked`'s existing `updateStrategyData()`-then-`update()` sequence)
- Modify: Task 2's tooltip code in `strategyexplorer.cpp` — gate the EV/equity portion of the tooltip text behind this flag (read via a getter you add, or by passing `StrategyExplorer*` context through — keep it simple: this can live as a `bool` member on `StrategyExplorer` that the tooltip-building code in Task 2 already has access to via `this`)

**Interfaces:**
- Consumes: nothing new beyond Task 2's tooltip-building code and the existing repaint pattern.
- Produces: `StrategyExplorer::advancedMode` (bool) — if a future phase wants other panels to also respect this flag, this is the field to read.

**Note on scope:** the design spec originally envisioned this as gating the *entire* results screen's numeric detail (including inside `detailView`/`DetailItemDelegate`'s permanent side panel). Threading a flag all the way into `DetailItemDelegate`'s paint methods is a larger, more invasive change (touches a delegate class used elsewhere). For this task, **only gate the new hover tooltip's EV/equity line** — leave `detailView`'s existing always-on EV display untouched. This keeps the change additive and low-risk, matching "no rewrite of existing paint methods" in Global Constraints. If, after using it, the app owner wants the toggle to also affect `detailView`, that's a follow-up task, not part of this one — do not attempt it here even if it seems like a small extension; keep this task's diff scoped to what's listed above.

- [ ] **Step 1: Add the checkbox to `strategyexplorer.ui`**

Add `QCheckBox advancedModeCheck`, unchecked by default, positioned near the existing mode buttons (`ipRangeButtom`/`oopRangeButtom`/`strategyModeButtom`/`evModeButtom`/`evOnlyModeButtom` — read their current container in the `.ui` file and add the checkbox as a sibling in the same row/area).

- [ ] **Step 2: Declare and implement the slot**

In `strategyexplorer.h`, add `bool advancedMode = false;` (private member) and declare `void on_advancedModeCheck_toggled(bool checked);` in the slots section. In `strategyexplorer.cpp`, implement:

```cpp
void StrategyExplorer::on_advancedModeCheck_toggled(bool checked)
{
    this->advancedMode = checked;
    // trigger the same kind of repaint the existing mode-button slots use —
    // read one of ipRangeButtom/evModeButtom's existing slot implementation
    // first and reuse its exact repaint call(s), don't invent a new one.
}
```

- [ ] **Step 3: Gate the tooltip's EV/equity line**

In Task 2's tooltip-building code, wrap the EV/equity portion of the tooltip string in `if(this->advancedMode) { ... }` so the basic tooltip (combo + action %) always shows, but EV/equity numbers only show when the checkbox is checked.

- [ ] **Step 4: Build**

Run the build command from Global Constraints. Expected: no `error:` lines.

- [ ] **Step 5: Manual verification**

Launch, solve, open results. Confirm the checkbox is unchecked by default and the tooltip shows combo+% only. Check it, hover again, confirm EV/equity now appears in the tooltip. Uncheck, confirm it disappears again. Confirm `detailView`'s permanent panel is unaffected either way (still always shows its full detail, per the scope note above).

- [ ] **Step 6: Commit**

```bash
git add strategyexplorer.ui strategyexplorer.h strategyexplorer.cpp
git commit -m "Add simple/advanced toggle gating EV detail in the hover tooltip"
```

---

### Task 4: Theme styling for the legend and checkbox

**Files:**
- Modify: `resources/themes/theme_dark.qss`
- Modify: `resources/themes/theme_light.qss`

**Interfaces:**
- Consumes: `resultsLegend` and its child labels, `advancedModeCheck` objectNames from Tasks 1 and 3. Does NOT restyle `legendSwatchFold`/`legendSwatchCall`/`legendSwatchBet` (those keep their fixed inline colors from Task 1, deliberately theme-independent).

- [ ] **Step 1: Add dark-theme rules**

Append to `resources/themes/theme_dark.qss`:

```css
#resultsLegend {
    background-color: #1a1d29;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    padding: 6px 10px;
}

#resultsLegend QLabel {
    color: #e8e8f0;
    font-size: 11px;
}
```

- [ ] **Step 2: Add light-theme rules**

Append to `resources/themes/theme_light.qss`:

```css
#resultsLegend {
    background-color: #ffffff;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    padding: 6px 10px;
}

#resultsLegend QLabel {
    color: #1a1d29;
    font-size: 11px;
}
```

(`advancedModeCheck` needs no new rules — the existing generic `QCheckBox` rules already in both theme files, from the theme-system plan, apply to it automatically since it has no special objectName-specific override requested here.)

- [ ] **Step 3: Build**

Run the build command from Global Constraints. Expected: no `error:` lines.

- [ ] **Step 4: Manual verification**

Launch in dark theme, open results, confirm the legend has a distinct dark card background with light text, swatches still show their true fixed colors. Switch to light theme, relaunch, confirm the equivalent light look.

- [ ] **Step 5: Commit**

```bash
git add resources/themes/theme_dark.qss resources/themes/theme_light.qss
git commit -m "Style results-screen legend for both themes"
```

---

## Self-Review Notes

- **Spec coverage vs. deliberate deferral:** The design spec's section 3 asked for a 3-column layout, fixed legend, tooltip, simple/advanced toggle, and exploitability indicator. This plan delivers the legend, tooltip, and toggle. It does **not** restructure the existing working 2-column layout into 3 columns (the existing `gameTreeView` + `strategyTableView` + `roughStrategyView`/`detailView`/mode-buttons arrangement already covers the same functional areas the spec's 3-column mockup wanted — tree nav, matrix+legend, detail/mode controls — just arranged differently; reflowing a working, non-broken layout for cosmetic column-count parity with an early mockup is not worth the regression risk). It does **not** build the exploitability indicator, because the only source of that number (`BestResponse::printExploitability`, called from `src/solver/PCfrSolver.cpp`) sits inside the explicitly protected `src/solver/` directory, and exposing it to the GUI would require modifying that file to persist the value somewhere the GUI can read (it currently only `qDebug()`-prints it). This is a real conflict between two spec goals (show exploitability / never touch solver code) — flagging it for the human to decide rather than silently either breaking the constraint or silently dropping the feature without explanation.
- **Theme-decoupling of the matrix itself, left alone:** Codebase research found that `StrategyItemDelegate`/`RoughStrategyItemDelegate`/`DetailItemDelegate` paint hardcoded colors via `QTextDocument`s with no theme awareness at all — this predates this plan and works fine visually in both themes today (cell backgrounds are always bright/pastel, never theme-dark, so text contrast isn't actually broken in practice despite the theoretical risk flagged during the theme-system phase's final review). This plan does not touch those delegates' core paint logic — only adds new, separate, theme-aware widgets (the legend, the checkbox) alongside them. Making the existing matrix cells themselves theme-aware is out of scope here; it's cosmetic polish on already-working code, not a defect this plan needs to fix.
- **No placeholders:** every step has concrete code or an explicit, scoped instruction to read specific real files before finalizing exact signatures (Task 2 Step 1) — same pattern used successfully in the wizard plan for the one section where hand-transcribing exact current code would risk being wrong.
