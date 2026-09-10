# Modo Rápido Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add "Modo Rápido" — a 3-step flow where a beginner picks a plain-language preflop situation, their two hole cards, and the board, and gets a direct recommendation for their exact hand, with zero range/bet-size configuration.

**Architecture:** A thin front-end layer that populates the exact same `QSolverJob`/UI-widget fields the existing 6-step "Modo Avanzado" wizard already sets (via `mainwindow.cpp`'s existing `on_buildTreeButtom_clicked()`/`on_buttomSolve_clicked()`), then auto-launches `StrategyExplorer` with the user's hand highlighted. No changes to the solver engine.

**Tech Stack:** Qt6 Widgets, C++17, existing `QSolverJob`/`TableStrategyModel`/`BoardSelectorTableModel` classes.

## Global Constraints

- All new user-facing strings are Spanish source text via `tr()`, matching the rest of the app (see `docs/superpowers/specs/2026-09-10-modo-rapido-design.md`).
- Reuse `BoardSelectorTableModel`/`BoardSelectorTableDelegate`/`HtmlTableView` for card selection — do not create a new card-grid widget class.
- Reuse `ui->wizardStepBoard` (the existing Modo Avanzado board step) as Modo Rápido's board step — do not duplicate it.
- Modo Avanzado's existing 6-step flow (`showWizardStep`, `wizardSteps[6]`) must not change behavior.
- Default bet/raise sizing for every quick-mode matchup: 50% bet, 60% raise, all streets, `Add Allin` off, `raiseLimitText`=4, `allinThresholdText`=0.67, `useIsoCheck` checked, `useHalfFloats_box` index 0, `mode_box` index 0 (holdem), `iterationText`=200, `exploitabilityText`=0.5, `logIntervalText`=10, `threadsText`=8 — these are the same values already shipped as the `.ui` file's defaults for those widgets.

---

### Task 1: Quick-mode range data table

**Files:**
- Create: `include/data/quickmoderanges.h`
- Create: `src/data/quickmoderanges.cpp`
- Modify: `TexasSolverGui.pro:129` (add header to `HEADERS +=` list, after `settingeditor.h`) and `TexasSolverGui.pro:96` (add .cpp to `SOURCES +=` list, after `settingeditor.cpp`)

**Interfaces:**
- Produces: `struct QuickModeMatchup` with fields `id` (QString), `descriptionAsOpener` (QString), `descriptionAsCaller` (QString), `openerRange` (QString), `callerRange` (QString), `openerIsIP` (bool), `pot` (float), `effectiveStack` (float).
- Produces: `QVector<QuickModeMatchup> getQuickModeMatchups()` — free function, 6 entries in the exact order below (index order matters, later tasks reference these by index).

- [ ] **Step 1: Create the header**

```cpp
// include/data/quickmoderanges.h
#ifndef QUICKMODERANGES_H
#define QUICKMODERANGES_H

#include <QString>
#include <QVector>

struct QuickModeMatchup {
    QString id;
    QString descriptionAsOpener;  // shown when the user opened the pot
    QString descriptionAsCaller;  // shown when the user called (or called a 3-bet)
    QString openerRange;
    QString callerRange;
    bool openerIsIP;              // false only for SB-opens-BB-calls (BB acts last postflop)
    float pot;
    float effectiveStack;
};

QVector<QuickModeMatchup> getQuickModeMatchups();

#endif // QUICKMODERANGES_H
```

- [ ] **Step 2: Create the implementation with all 6 matchups**

```cpp
// src/data/quickmoderanges.cpp
#include "include/data/quickmoderanges.h"

QVector<QuickModeMatchup> getQuickModeMatchups() {
    QVector<QuickModeMatchup> matchups;

    matchups.append({
        "utg_open_bb_call",
        QObject::tr("Abriste desde UTG y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde UTG y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,AJo,AQo,AKo,A5s,A9s,ATs,AJs,AQs,AKs,KTs,KJs,KQs,KQo,QTs,QJs,QJo,JTs,T9s,98s,87s,76s,65s",
        "22,33,44,55,66,77,88,99,TT,A9s,ATs,AJs,AQs,AQo:0.5,KQs,KJs,KQo:0.5,QJs,JTs,T9s,98s,87s,76s,65s",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "co_open_bb_call",
        QObject::tr("Abriste desde el Cutoff (CO) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde el Cutoff (CO) y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K9s,KTs,KJs,KQs,KTo:0.5,KQo,Q9s,QTs,QJs,QJo,J9s,JTs,T9s,T8s:0.5,98s,97s:0.5,87s,86s:0.5,76s,65s,54s:0.5",
        "22,33,44,55,66,77,88,99,TT,JJ:0.5,A8s,A9s,ATs,AJs,AQs,AQo:0.5,K9s,KTs,KJs,KQs,KQo:0.5,Q9s,QTs,QJs,J9s,JTs,T9s,T8s,98s,87s,86s:0.5,76s,65s,54s:0.5",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "btn_open_bb_call",
        QObject::tr("Abriste desde el Botón (BTN) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde el Botón (BTN) y vos pagaste desde la ciega grande (BB)"),
        "AA,KK,QQ,JJ,TT,99:0.75,88:0.75,77:0.5,66:0.25,55:0.25,AK,AQs,AQo:0.75,AJs,AJo:0.5,ATs:0.75,A6s:0.25,A5s:0.75,A4s:0.75,A3s:0.5,A2s:0.5,KQs,KQo:0.5,KJs,KTs:0.75,K5s:0.25,K4s:0.25,QJs:0.75,QTs:0.75,Q9s:0.5,JTs:0.75,J9s:0.75,J8s:0.75,T9s:0.75,T8s:0.75,T7s:0.75,98s:0.75,97s:0.75,96s:0.5,87s:0.75,86s:0.5,85s:0.5,76s:0.75,75s:0.5,65s:0.75,64s:0.5,54s:0.75,53s:0.5,43s:0.5",
        "QQ:0.5,JJ:0.75,TT,99,88,77,66,55,44,33,22,AKo:0.25,AQs,AQo:0.75,AJs,AJo:0.75,ATs,ATo:0.75,A9s,A8s,A7s,A6s,A5s,A4s,A3s,A2s,KQ,KJ,KTs,KTo:0.5,K9s,K8s,K7s,K6s,K5s,K4s:0.5,K3s:0.5,K2s:0.5,QJ,QTs,Q9s,Q8s,Q7s,JTs,JTo:0.5,J9s,J8s,T9s,T8s,T7s,98s,97s,96s,87s,86s,76s,75s,65s,64s,54s,53s,43s",
        true, 50.0f, 200.0f
    });

    matchups.append({
        "sb_open_bb_call",
        QObject::tr("Abriste desde la ciega chica (SB) y el rival pagó desde la ciega grande (BB)"),
        QObject::tr("El rival abrió desde la ciega chica (SB) y vos pagaste desde la ciega grande (BB)"),
        "22,33,44,55,66,77,88,99,TT,JJ,QQ,KK,AA,A2o:0.5,A3o:0.5,A4o:0.5,A5o:0.5,A6o:0.5,A7o,A8o,A9o,ATo,AJo,AQo,AKo,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs,AKs,K5s:0.5,K6s,K7s,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo,Q7s:0.5,Q8s,Q9s,QTs,QJs,QTo:0.5,QJo,J8s,J9s,JTs,JTo:0.5,T8s,T9s,97s:0.5,98s,87s,76s,65s,54s,43s:0.5",
        "22,33,44,55,66,77,88,99,TT,JJ,QQ:0.5,A2s,A3s,A4s,A5s,A6s,A7s,A8s,A9s,ATs,AJs,AQs:0.5,A8o:0.5,A9o,ATo,AJo,AQo:0.5,K7s:0.5,K8s,K9s,KTs,KJs,KQs,KTo:0.5,KJo,KQo:0.5,Q9s,QTs,QJs,QTo:0.5,J9s,JTs,JTo:0.5,T8s,T9s,98s,97s:0.5,87s,86s:0.5,76s,65s,54s,43s:0.5",
        false, 60.0f, 195.0f
    });

    matchups.append({
        "btn_open_bb_3bet_btn_call",
        QObject::tr("Abriste desde el Botón (BTN), el rival subió (3-bet) desde la ciega grande y vos pagaste"),
        QObject::tr("El rival abrió desde el Botón (BTN), subiste (3-bet) desde la ciega grande y el rival pagó"),
        "QQ,KK,AA:0.5,JJ:0.5,TT:0.5,AQs,AKs,AKo:0.5,AJs:0.5,KQs:0.5,ATs:0.3",
        "AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,TT:0.3,KQs:0.3,A2s:0.3",
        true, 130.0f, 175.0f
    });

    matchups.append({
        "co_open_btn_3bet_co_call",
        QObject::tr("Abriste desde el Cutoff (CO), el rival subió (3-bet) desde el Botón y vos pagaste"),
        QObject::tr("El rival abrió desde el Cutoff (CO), subiste (3-bet) desde el Botón y el rival pagó"),
        "QQ,KK,AA:0.5,JJ:0.5,TT:0.4,AQs,AKs,AKo:0.5,AJs:0.3",
        "AA,KK,QQ,AKs,AKo,A5s,A4s,JJ:0.5,KQs:0.3,A2s:0.3",
        false, 130.0f, 175.0f
    });

    return matchups;
}
```

Note: for rows 5-6 (3-bet pots), `openerRange` holds the **original opener/caller-of-the-3bet**'s range and `callerRange` holds the **3-bettor**'s range — the struct field names describe *who acted first preflop*, not literal "who called". `openerIsIP` still tells you which of the two is IP. Task 5 uses this exact mapping.

- [ ] **Step 3: Add the new files to the project**

Edit `TexasSolverGui.pro`. Find this block (existing `HEADERS +=` list):
```
    include/ui/boardselectortabledelegate.h \
    settingeditor.h \
    welcomedialog.h
```
Change to:
```
    include/ui/boardselectortabledelegate.h \
    settingeditor.h \
    welcomedialog.h \
    include/data/quickmoderanges.h
```

Find this block (existing `SOURCES +=` list):
```
    src/ui/boardselectortabledelegate.cpp \
    settingeditor.cpp \
    welcomedialog.cpp
```
Change to:
```
    src/ui/boardselectortabledelegate.cpp \
    settingeditor.cpp \
    welcomedialog.cpp \
    src/data/quickmoderanges.cpp
```

- [ ] **Step 4: Build to verify it compiles**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: no output (no errors). This task only adds a data file with no callers yet, so a clean compile is the only verification possible.

- [ ] **Step 5: Commit**

```bash
git add include/data/quickmoderanges.h src/data/quickmoderanges.cpp TexasSolverGui.pro
git commit -m "Add quick-mode preflop matchup range data table"
```

---

### Task 2: Welcome dialog — Modo Rápido / Modo Avanzado choice

**Files:**
- Modify: `welcomedialog.h` (whole file — small, shown in full)
- Modify: `welcomedialog.cpp` (whole file — small, shown in full)
- Modify: `welcomedialog.ui:24-79` (button labels/ids, body text)

**Interfaces:**
- Consumes: nothing new.
- Produces: `WelcomeDialog::Choice` enum now has 3 values: `Cancelled`, `QuickMode`, `Advanced` (renamed from `ViewExample`/`StartFresh` — Task 5 depends on these exact names).

- [ ] **Step 1: Update the header**

Replace the full contents of `welcomedialog.h`:

```cpp
#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

namespace Ui {
class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    enum Choice { Cancelled, QuickMode, Advanced };

    explicit WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();
    Choice choice();

private slots:
    void on_quickModeButton_clicked();
    void on_advancedButton_clicked();

private:
    Ui::WelcomeDialog *ui;
    Choice result = Cancelled;
};

#endif // WELCOMEDIALOG_H
```

- [ ] **Step 2: Update the implementation**

Replace the full contents of `welcomedialog.cpp`:

```cpp
#include "welcomedialog.h"
#include "ui_welcomedialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Bienvenido a Solverix"));
}

WelcomeDialog::~WelcomeDialog()
{
    delete ui;
}

WelcomeDialog::Choice WelcomeDialog::choice()
{
    return this->result;
}

void WelcomeDialog::on_quickModeButton_clicked()
{
    this->result = QuickMode;
    accept();
}

void WelcomeDialog::on_advancedButton_clicked()
{
    this->result = Advanced;
    accept();
}
```

- [ ] **Step 3: Update the .ui file**

Read `welcomedialog.ui` first, then apply these changes:
- Rename the `exampleButton` object to `quickModeButton`, change its text to `"🚀 Modo Rápido"`.
- Rename the `freshButton` object to `advancedButton`, change its text to `"🔧 Modo Avanzado"`.
- Reorder the button layout so `quickModeButton` comes first (it's the primary/recommended path).
- Replace `bodyLabel`'s text with:

```
Solverix calcula, para una situación concreta de una mano, cuál es la forma matemáticamente más equilibrada de jugarla.

Tenés dos formas de usarlo:

🚀 Modo Rápido: cargás tus 2 cartas, el board, y elegís de una lista qué pasó antes del flop (por ejemplo "abriste desde el Botón y te pagaron"). Nosotros armamos automáticamente los rangos típicos de esa situación y te mostramos directamente qué conviene hacer con tu mano.

🔧 Modo Avanzado: configurás vos mismo los rangos exactos de cada jugador, el tamaño de las apuestas, y todos los demás parámetros — para cuando quieras control total.

Si es tu primera vez, te recomendamos Modo Rápido.
```

- [ ] **Step 4: Build to verify it compiles**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: errors referencing `mainwindow.cpp`'s uses of the old `WelcomeDialog::ViewExample`/`StartFresh` — this is expected and gets fixed in Task 5. Confirm the errors are *only* in `mainwindow.cpp`, not in `welcomedialog.cpp` itself.

- [ ] **Step 5: Commit**

```bash
git add welcomedialog.h welcomedialog.cpp welcomedialog.ui
git commit -m "Replace welcome dialog's example/fresh-start choice with Modo Rápido/Avanzado"
```

---

### Task 3: Situation-picker step widget

**Files:**
- Modify: `mainwindow.ui` (insert a new step widget; see exact insertion point below)

**Interfaces:**
- Produces: a `QWidget` named `quickStepSituation`, containing 12 `QPushButton`s named `situationBtn0` through `situationBtn11` (index matches the flattened opener/caller pairs of the 6 matchups in Task 1's array order: button `2*i` = matchup `i` "as opener", button `2*i+1` = matchup `i` "as caller"), plus a `QLabel` named `situationIntroLabel`.

- [ ] **Step 1: Read the insertion point**

Read `mainwindow.ui` and find `<widget class="QWidget" name="wizardStepRanges">` (this is the first child widget inside `inputLayout`, right after the `wizardNavStrip` item closes). The new widget goes as a **sibling**, immediately before this line, i.e. as a new `<item>` inside `inputLayout` positioned before `wizardStepRanges`'s `<item>`.

- [ ] **Step 2: Insert the situation-picker widget**

Insert this new `<item>` block immediately before the `<item>` that wraps `wizardStepRanges`:

```xml
      <item>
       <widget class="QWidget" name="quickStepSituation">
        <layout class="QVBoxLayout" name="quickStepSituationLayout">
         <item>
          <widget class="QLabel" name="situationIntroLabel">
           <property name="wordWrap">
            <bool>true</bool>
           </property>
           <property name="text">
            <string>Elegí la opción que más se parezca a lo que pasó antes del flop en tu mano.</string>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QLabel" name="situationGroupLabel1">
           <property name="text">
            <string>▸ Bote simple</string>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn0">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn1">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn2">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn3">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn4">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn5">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn6">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn7">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QLabel" name="situationGroupLabel2">
           <property name="text">
            <string>▸ Con 3-bet (re-subida antes del flop)</string>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn8">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn9">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn10">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QPushButton" name="situationBtn11">
           <property name="text">
            <string/>
           </property>
          </widget>
         </item>
        </layout>
       </widget>
      </item>
```

Button text is left empty here — Task 5 sets it programmatically from `getQuickModeMatchups()`'s description strings so the two files can't drift out of sync.

- [ ] **Step 3: Add one to `inputLayout`'s stretch count**

Find `<layout class="QVBoxLayout" name="inputLayout" stretch="0,0,0,0,0,0,0,1">` and add one more `0` for the new item: `stretch="0,0,0,0,0,0,0,0,1"`.

- [ ] **Step 4: Build to verify it compiles**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: no errors (the widget exists but nothing references `ui->quickStepSituation` yet, which is fine — unused-but-present widgets don't error).

- [ ] **Step 5: Commit**

```bash
git add mainwindow.ui
git commit -m "Add Modo Rápido situation-picker step widget"
```

---

### Task 4: Hand-picker step widget

**Files:**
- Modify: `mainwindow.ui` (insert a new step widget, sibling of `quickStepSituation`)

**Interfaces:**
- Consumes: `BoardSelectorTableModel`, `BoardSelectorTableDelegate` (existing classes, unchanged).
- Produces: a `QWidget` named `quickStepHand`, containing an `HtmlTableView` named `handSelectorTable` and a `QLabel` named `handSelectedLabel`.

- [ ] **Step 1: Insert the hand-picker widget**

Insert this new `<item>` immediately after `quickStepSituation`'s closing `</item>` (i.e. also before `wizardStepRanges`'s `<item>`, right after the one added in Task 3):

```xml
      <item>
       <widget class="QWidget" name="quickStepHand">
        <layout class="QVBoxLayout" name="quickStepHandLayout">
         <item>
          <widget class="QLabel" name="handIntroLabel">
           <property name="wordWrap">
            <bool>true</bool>
           </property>
           <property name="text">
            <string>Tocá exactamente 2 cartas: las que tenías en la mano.</string>
           </property>
          </widget>
         </item>
         <item>
          <widget class="QLabel" name="handSelectedLabel">
           <property name="text">
            <string>Seleccionadas: (ninguna)</string>
           </property>
          </widget>
         </item>
         <item>
          <widget class="HtmlTableView" name="handSelectorTable">
           <attribute name="horizontalHeaderVisible">
            <bool>false</bool>
           </attribute>
           <attribute name="verticalHeaderVisible">
            <bool>false</bool>
           </attribute>
          </widget>
         </item>
        </layout>
       </widget>
      </item>
```

- [ ] **Step 2: Update `inputLayout`'s stretch count again**

Change `stretch="0,0,0,0,0,0,0,0,1"` (from Task 3) to `stretch="0,0,0,0,0,0,0,0,0,1"` (one more `0`, for this new second item).

- [ ] **Step 3: Build to verify it compiles**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: no errors.

- [ ] **Step 4: Commit**

```bash
git add mainwindow.ui
git commit -m "Add Modo Rápido hand-picker step widget"
```

---

### Task 5: Quick-mode state machine and wiring

**Files:**
- Modify: `mainwindow.h` (add members/slots — exact diff below)
- Modify: `mainwindow.cpp` (constructor, `on_helpButton_clicked`, `on_wizardNextButton_clicked`, `on_wizardBackButton_clicked`, `onSolverJobFinished` — exact diffs below; new methods appended)

**Interfaces:**
- Consumes: `getQuickModeMatchups()` (Task 1), `WelcomeDialog::Choice::QuickMode`/`Advanced` (Task 2), `ui->quickStepSituation`/`situationBtn0..11` (Task 3), `ui->quickStepHand`/`handSelectorTable`/`handSelectedLabel` (Task 4), `BoardSelectorTableModel`/`BoardSelectorTableDelegate` (existing), `StrategyExplorer::setHighlightedHand()`/`selectRootNode()` (Task 6 — declare the calls here, Task 6 implements them; build will fail until Task 6 lands, that's expected and called out below).
- Produces: `MainWindow::startQuickMode()` — entry point called from `on_helpButton_clicked()`.

- [ ] **Step 1: Add new members and slot declarations to `mainwindow.h`**

Add this include near the top, after `#include "include/ui/rangeselectortabledelegate.h"`:

```cpp
#include "include/data/quickmoderanges.h"
#include "include/ui/boardselectortablemodel.h"
#include "include/ui/boardselectortabledelegate.h"
```

Add these slot declarations inside the existing `private slots:` block (anywhere in that block):

```cpp
    void onQuickSituationChosen();
    void onHandSelectorClicked(const QModelIndex &index);
```

Add these members inside the existing `private:` block, after `bool solvingInProgress = false;`:

```cpp
    enum class QuickModeStage { None, WaitingForBuildTree, WaitingForSolve };
    QuickModeStage quickModePendingStage = QuickModeStage::None;
    bool quickMode = false;
    int currentQuickStep = 0;
    QWidget* quickModeSteps[3];
    int chosenMatchupIndex = -1;
    bool userIsOpener = true;
    QString quickModeCard1;
    QString quickModeCard2;
    BoardSelectorTableModel* handSelectorModel = NULL;
    BoardSelectorTableDelegate* handSelectorDelegate = NULL;
    void startQuickMode();
    void showQuickModeStep(int index);
    void startQuickModeSolve();
```

- [ ] **Step 2: Wire up the hand-selector model in the constructor**

Read `mainwindow.cpp`'s constructor. Find this block (already present, sets up the IP/OOP range grids):

```cpp
    this->oop_model = new RangeSelectorTableModel(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","),this->ui->oopRangeText->toPlainText(),this,true);
```

Immediately after the full block that ends with `this->ui->oopRangeTableView->horizontalHeader()->setMinimumSectionSize(1);`, add:

```cpp
    // Quick-mode hand picker — reuses the existing board-card grid widget/model,
    // just pointed at a separate (initially empty) text buffer for "my 2 cards".
    this->handSelectorModel = new BoardSelectorTableModel(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","), "", this);
    this->ui->handSelectorTable->setModel(this->handSelectorModel);
    this->handSelectorDelegate = new BoardSelectorTableDelegate(QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(","), this->handSelectorModel, this);
    this->ui->handSelectorTable->setItemDelegate(this->handSelectorDelegate);
    connect(this->ui->handSelectorTable, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onHandSelectorClicked(const QModelIndex&)));

    this->quickModeSteps[0] = this->ui->quickStepSituation;
    this->quickModeSteps[1] = this->ui->quickStepHand;
    this->quickModeSteps[2] = this->ui->wizardStepBoard;
    this->ui->quickStepSituation->setVisible(false);
    this->ui->quickStepHand->setVisible(false);

    QVector<QuickModeMatchup> matchups = getQuickModeMatchups();
    QPushButton* situationButtons[12] = {
        this->ui->situationBtn0, this->ui->situationBtn1, this->ui->situationBtn2, this->ui->situationBtn3,
        this->ui->situationBtn4, this->ui->situationBtn5, this->ui->situationBtn6, this->ui->situationBtn7,
        this->ui->situationBtn8, this->ui->situationBtn9, this->ui->situationBtn10, this->ui->situationBtn11
    };
    for(int m = 0; m < matchups.size(); m++){
        situationButtons[2*m]->setText(matchups[m].descriptionAsOpener);
        situationButtons[2*m+1]->setText(matchups[m].descriptionAsCaller);
        connect(situationButtons[2*m], &QPushButton::clicked, this, [this, m](){
            this->chosenMatchupIndex = m;
            this->userIsOpener = true;
            this->onQuickSituationChosen();
        });
        connect(situationButtons[2*m+1], &QPushButton::clicked, this, [this, m](){
            this->chosenMatchupIndex = m;
            this->userIsOpener = false;
            this->onQuickSituationChosen();
        });
    }
```

`BoardSelectorTableDelegate`'s constructor signature and `QPushButton` are already used/included elsewhere in this file (`ui->flop_ip_allin` etc. use `QPushButton` implicitly via the `.ui`-generated header) — no new includes needed beyond Step 1's three lines.

- [ ] **Step 3: Implement `onQuickSituationChosen()` and `onHandSelectorClicked()`**

Add these new method bodies near the end of `mainwindow.cpp` (after `onSolverJobFinished`, before `on_ipRangeText_textChanged`):

```cpp
void MainWindow::onQuickSituationChosen(){
    this->handSelectorModel->clear_board();
    this->ui->handSelectorTable->update();
    this->ui->handSelectedLabel->setText(tr("Seleccionadas: (ninguna)"));
    this->showQuickModeStep(1);
}

void MainWindow::onHandSelectorClicked(const QModelIndex &index){
    int row = index.row();
    int col = index.column();
    bool wasSelected = this->handSelectorModel->getBoardAt(row, col) > 0;
    if(!wasSelected){
        int selectedCount = 0;
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 13; j++){
                if(this->handSelectorModel->getBoardAt(i, j) > 0) selectedCount++;
            }
        }
        if(selectedCount >= 2){
            QMessageBox::information(this, tr("Ya elegiste 2 cartas"),
                tr("Solo podés elegir 2 cartas para tu mano. Tocá una de las ya seleccionadas para sacarla."));
            return;
        }
    }
    this->handSelectorModel->setBoardAt(row, col, wasSelected ? 0 : 1);
    this->ui->handSelectorTable->update();
    QString text = this->handSelectorModel->getBoardText();
    this->ui->handSelectedLabel->setText(text.isEmpty() ? tr("Seleccionadas: (ninguna)") : tr("Seleccionadas: %1").arg(text));
}
```

- [ ] **Step 4: Implement `startQuickMode()` and `showQuickModeStep()`**

Add these new methods in the same location:

```cpp
void MainWindow::startQuickMode(){
    this->quickMode = true;
    this->exampleMode = false;
    for(int i = 0; i < 6; i++){
        this->wizardSteps[i]->setVisible(false);
    }
    this->handSelectorModel->clear_board();
    this->ui->handSelectedLabel->setText(tr("Seleccionadas: (ninguna)"));
    this->showQuickModeStep(0);
}

void MainWindow::showQuickModeStep(int index){
    if(index < 0 || index > 2) return;
    this->currentQuickStep = index;
    for(int i = 0; i < 3; i++){
        this->quickModeSteps[i]->setVisible(i == index);
    }
    QStringList stepNames;
    stepNames << tr("Situación") << tr("Tus cartas") << tr("Board");
    this->ui->wizardStepLabel->setText(tr("Modo Rápido — Paso %1 de 3: %2").arg(index + 1).arg(stepNames[index]));
    this->ui->wizardStepSubtitle->setText("");
    this->ui->exampleBanner->setVisible(false);
    this->ui->wizardBackButton->setEnabled(index > 0);
    this->ui->wizardNextButton->setText(index == 2 ? tr("Resolver →") : tr("Siguiente →"));
}
```

- [ ] **Step 5: Implement `startQuickModeSolve()`**

```cpp
void MainWindow::startQuickModeSolve(){
    QVector<QuickModeMatchup> matchups = getQuickModeMatchups();
    QuickModeMatchup matchup = matchups[this->chosenMatchupIndex];

    QString myRange = this->userIsOpener ? matchup.openerRange : matchup.callerRange;
    QString villainRange = this->userIsOpener ? matchup.callerRange : matchup.openerRange;
    bool myRangeIsIP = this->userIsOpener ? matchup.openerIsIP : !matchup.openerIsIP;

    if(myRangeIsIP){
        this->ui->ipRangeText->setPlainText(myRange);
        this->ui->oopRangeText->setPlainText(villainRange);
    }else{
        this->ui->ipRangeText->setPlainText(villainRange);
        this->ui->oopRangeText->setPlainText(myRange);
    }

    this->ui->potText->setText(QString::number(matchup.pot, 'f', 1));
    this->ui->effectiveStackText->setText(QString::number(matchup.effectiveStack, 'f', 1));
    this->ui->raiseLimitText->setText("4");
    this->ui->allinThresholdText->setText("0.67");
    this->ui->useIsoCheck->setChecked(true);
    this->ui->useHalfFloats_box->setCurrentIndex(0);
    this->ui->mode_box->setCurrentIndex(0);
    this->ui->iterationText->setText("200");
    this->ui->exploitabilityText->setText("0.5");
    this->ui->logIntervalText->setText("10");
    this->ui->threadsText->setText("8");

    QStringList betSizeFields = {"flop_ip_bet","turn_ip_bet","river_ip_bet","flop_oop_bet","turn_oop_bet","river_oop_bet"};
    QStringList raiseSizeFields = {"flop_ip_raise","turn_ip_raise","river_ip_raise","flop_oop_raise","turn_oop_raise","river_oop_raise"};
    for(const QString& name : betSizeFields){
        QLineEdit* field = this->findChild<QLineEdit*>(name);
        if(field != NULL) field->setText("50");
    }
    for(const QString& name : raiseSizeFields){
        QLineEdit* field = this->findChild<QLineEdit*>(name);
        if(field != NULL) field->setText("60");
    }
    QStringList allinChecks = {"flop_ip_allin","turn_ip_allin","river_ip_allin","flop_oop_allin","turn_oop_allin","river_oop_allin"};
    for(const QString& name : allinChecks){
        QCheckBox* field = this->findChild<QCheckBox*>(name);
        if(field != NULL) field->setChecked(false);
    }
    QLineEdit* turnDonk = this->findChild<QLineEdit*>("turn_oop_donk");
    if(turnDonk != NULL) turnDonk->setText("");
    QLineEdit* riverDonk = this->findChild<QLineEdit*>("river_oop_donk");
    if(riverDonk != NULL) riverDonk->setText("");

    this->quickModePendingStage = QuickModeStage::WaitingForBuildTree;
    this->on_buildTreeButtom_clicked();
}
```

Uses `findChild<QLineEdit*>`/`findChild<QCheckBox*>` by object name rather than 12 individual `ui->flop_ip_bet` etc. references, since the list is long and mechanical — this matches how `sizes_convert` already treats these fields as plain text uniformly elsewhere in this file. `#include <QLineEdit>` and `#include <QCheckBox>` — check whether `mainwindow.cpp` already transitively includes these (it uses `ui->flop_ip_bet->text()` already, so `QLineEdit` is already available via the generated `ui_mainwindow.h`; add explicit includes only if the build in Step 8 reports them missing).

- [ ] **Step 6: Wire `on_helpButton_clicked()` to the new choice values**

Find:
```cpp
void MainWindow::on_helpButton_clicked()
{
    WelcomeDialog dialog(this);
    dialog.exec();
    if(dialog.choice() == WelcomeDialog::ViewExample){
        this->resetToExampleDefaults();
    }else if(dialog.choice() == WelcomeDialog::StartFresh){
        this->exampleMode = false;
        this->showWizardStep(this->currentWizardStep);
    }
}
```

Replace with:
```cpp
void MainWindow::on_helpButton_clicked()
{
    WelcomeDialog dialog(this);
    dialog.exec();
    if(dialog.choice() == WelcomeDialog::QuickMode){
        this->startQuickMode();
    }else if(dialog.choice() == WelcomeDialog::Advanced){
        this->quickMode = false;
        this->exampleMode = false;
        for(int i = 0; i < 3; i++){
            this->quickModeSteps[i]->setVisible(false);
        }
        this->showWizardStep(this->currentWizardStep);
    }
}
```

- [ ] **Step 7: Branch `on_wizardNextButton_clicked()`/`on_wizardBackButton_clicked()` on `quickMode`**

Find:
```cpp
void MainWindow::on_wizardBackButton_clicked()
{
    if(this->currentWizardStep > 0){
        this->showWizardStep(this->currentWizardStep - 1);
    }
}

void MainWindow::on_wizardNextButton_clicked()
{
    if(this->currentWizardStep == 3){
        // Leaving the tree-params step: build the tree automatically,
        // exactly what clicking the pre-existing "Build Tree" button already does.
        this->on_buildTreeButtom_clicked();
    }
    if(this->currentWizardStep < 5){
        this->showWizardStep(this->currentWizardStep + 1);
    }
```

Replace the two function signatures/bodies (keep whatever closing brace/lines follow `on_wizardNextButton_clicked`'s shown body untouched — only the parts shown here change) with:

```cpp
void MainWindow::on_wizardBackButton_clicked()
{
    if(this->quickMode){
        if(this->currentQuickStep > 0){
            this->showQuickModeStep(this->currentQuickStep - 1);
        }
        return;
    }
    if(this->currentWizardStep > 0){
        this->showWizardStep(this->currentWizardStep - 1);
    }
}

void MainWindow::on_wizardNextButton_clicked()
{
    if(this->quickMode){
        if(this->currentQuickStep == 1){
            QString handText = this->handSelectorModel->getBoardText();
            QStringList cards = handText.split(",", Qt::SkipEmptyParts);
            if(cards.size() != 2){
                QMessageBox::information(this, tr("Elegí 2 cartas"), tr("Tenés que tocar exactamente 2 cartas para tu mano antes de seguir."));
                return;
            }
            this->quickModeCard1 = cards[0];
            this->quickModeCard2 = cards[1];
        }
        if(this->currentQuickStep == 2){
            this->startQuickModeSolve();
            return;
        }
        if(this->currentQuickStep < 2){
            this->showQuickModeStep(this->currentQuickStep + 1);
        }
        return;
    }
    if(this->currentWizardStep == 3){
        // Leaving the tree-params step: build the tree automatically,
        // exactly what clicking the pre-existing "Build Tree" button already does.
        this->on_buildTreeButtom_clicked();
    }
    if(this->currentWizardStep < 5){
        this->showWizardStep(this->currentWizardStep + 1);
    }
```

- [ ] **Step 8: Chain build-tree → solve → results in `onSolverJobFinished()`**

Find:
```cpp
void MainWindow::onSolverJobFinished()
{
    if(this->solvingInProgress){
        this->solvingInProgress = false;
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Listo"));
        msgBox.setText(tr("El solver terminó de calcular la estrategia. Tocá \"ShowResult\" para verla."));
        msgBox.exec();
    }
}
```

Replace with:
```cpp
void MainWindow::onSolverJobFinished()
{
    if(this->quickModePendingStage == QuickModeStage::WaitingForBuildTree){
        this->quickModePendingStage = QuickModeStage::WaitingForSolve;
        this->on_buttomSolve_clicked();
        return;
    }
    if(this->quickModePendingStage == QuickModeStage::WaitingForSolve){
        this->quickModePendingStage = QuickModeStage::None;
        this->solvingInProgress = false;
        this->strategyExplorer = new StrategyExplorer(this, this->qSolverJob);
        this->strategyExplorer->setAttribute(Qt::WA_DeleteOnClose);
        this->strategyExplorer->selectRootNode();
        this->strategyExplorer->setHighlightedHand(this->quickModeCard1, this->quickModeCard2);
        this->strategyExplorer->show();
        return;
    }
    if(this->solvingInProgress){
        this->solvingInProgress = false;
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Listo"));
        msgBox.setText(tr("El solver terminó de calcular la estrategia. Tocá \"ShowResult\" para verla."));
        msgBox.exec();
    }
}
```

- [ ] **Step 9: Build to verify**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: errors only about `StrategyExplorer::selectRootNode`/`setHighlightedHand` not existing — Task 6 adds those. Confirm no *other* errors (typos, missing includes, wrong widget names).

- [ ] **Step 10: Commit**

```bash
git add mainwindow.h mainwindow.cpp
git commit -m "Wire Modo Rápido state machine: situation -> hand -> board -> auto-solve"
```

---

### Task 6: "Your hand" results banner in StrategyExplorer

**Files:**
- Modify: `strategyexplorer.h` (add method declarations)
- Modify: `strategyexplorer.cpp` (add method implementations)
- Modify: `strategyexplorer.ui` (add banner `QLabel`)

**Interfaces:**
- Consumes: `TableStrategyModel::get_strategy(int i, int j)` returning `const vector<pair<GameActions,float>>` (existing, `include/ui/tablestrategymodel.h:37`), `GameActions::getAction()`/`getAmount()` (existing, `include/nodes/GameActions.h`).
- Produces: `void StrategyExplorer::selectRootNode()` and `void StrategyExplorer::setHighlightedHand(QString card1, QString card2)` — both called from `mainwindow.cpp` (Task 5, already written against these exact names).

- [ ] **Step 1: Add the banner widget to the .ui**

Read `strategyexplorer.ui`. Find the `explorerIntroLabel` widget (the intro text added earlier this session, first item inside `verticalLayout` on the left column). Insert a new item **immediately after** `explorerIntroLabel`'s closing `</item>`, still inside the same `verticalLayout`:

```xml
      <item>
       <widget class="QLabel" name="handBannerLabel">
        <property name="visible">
         <bool>false</bool>
        </property>
        <property name="wordWrap">
         <bool>true</bool>
        </property>
        <property name="text">
         <string/>
        </property>
       </widget>
      </item>
```

Update `verticalLayout`'s `stretch` attribute: it currently reads `stretch="0,3,0,0,8"` (5 items after the intro-label task added one `0`) — add one more `0` for this new item, giving `stretch="0,0,3,0,0,8"`. (Read the actual current attribute value first — this plan assumes the intro-label task already added the leading `0`; if the file shows a different count, add exactly one more `0` in the same position, right after the first `0`.)

- [ ] **Step 2: Declare the two new public methods**

In `strategyexplorer.h`, add to the `public:` section (after the constructor/destructor declarations):

```cpp
    void selectRootNode();
    void setHighlightedHand(QString card1, QString card2);
```

- [ ] **Step 3: Implement `selectRootNode()`**

In `strategyexplorer.cpp`, add near the end of the file:

```cpp
void StrategyExplorer::selectRootNode(){
    QModelIndex rootIndex = this->ui->gameTreeView->model()->index(0, 0);
    if(rootIndex.isValid()){
        this->item_clicked(rootIndex);
    }
}
```

- [ ] **Step 4: Implement `setHighlightedHand()`**

Add directly below `selectRootNode()`:

```cpp
static QString actionLabelSpanish(GameTreeNode::PokerActions action, double amount){
    switch(action){
        case GameTreeNode::PokerActions::FOLD: return QObject::tr("Retirarse");
        case GameTreeNode::PokerActions::CHECK: return QObject::tr("Chequear");
        case GameTreeNode::PokerActions::CALL: return QObject::tr("Pagar");
        case GameTreeNode::PokerActions::BET: return QObject::tr("Apostar %1% del pozo").arg((int)amount);
        case GameTreeNode::PokerActions::RAISE: return QObject::tr("Subir a %1% del pozo").arg((int)amount);
        default: return QObject::tr("Otra acción");
    }
}

void StrategyExplorer::setHighlightedHand(QString card1, QString card2){
    QStringList ranks = QString("A,K,Q,J,T,9,8,7,6,5,4,3,2").split(",");
    QChar rank1 = card1.at(0).toUpper();
    QChar rank2 = card2.at(0).toUpper();
    QChar suit1 = card1.at(1).toLower();
    QChar suit2 = card2.at(1).toLower();
    int idx1 = ranks.indexOf(QString(rank1));
    int idx2 = ranks.indexOf(QString(rank2));
    if(idx1 < 0 || idx2 < 0){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    int i, j;
    if(idx1 == idx2){
        i = idx1; j = idx1;
    }else if(suit1 == suit2){
        i = qMin(idx1, idx2); j = qMax(idx1, idx2);
    }else{
        i = qMax(idx1, idx2); j = qMin(idx1, idx2);
    }
    if(this->tableStrategyModel->treeItem == NULL ||
       this->tableStrategyModel->treeItem->m_treedata.lock()->getType() != GameTreeNode::GameTreeNode::ACTION){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    vector<pair<GameActions,float>> strategy = this->tableStrategyModel->get_strategy(i, j);
    if(strategy.empty()){
        this->ui->handBannerLabel->setVisible(false);
        return;
    }
    std::sort(strategy.begin(), strategy.end(), [](const pair<GameActions,float>& a, const pair<GameActions,float>& b){
        return a.second > b.second;
    });
    QString handLabel = QString("%1%2").arg(card1.at(0).toUpper()).arg(card2.at(0).toUpper());
    if(idx1 != idx2) handLabel += (suit1 == suit2) ? "s" : "o";
    QStringList parts;
    for(const pair<GameActions,float>& entry : strategy){
        if(entry.second < 0.01f) continue;
        int pct = (int)(entry.second * 100 + 0.5f);
        parts << QString("%1 (%2%)").arg(actionLabelSpanish(entry.first.getAction(), entry.first.getAmount())).arg(pct);
    }
    this->ui->handBannerLabel->setText(tr("Con %1: %2").arg(handLabel).arg(parts.join(" · ")));
    this->ui->handBannerLabel->setVisible(true);
}
```

`#include <algorithm>` for `std::sort` — check if already present at the top of `strategyexplorer.cpp`; add it if the build in Step 5 reports `std::sort` as undeclared.

- [ ] **Step 5: Build to verify**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
```
Expected: no errors. This is the first point where the whole feature should compile cleanly end to end.

- [ ] **Step 6: Commit**

```bash
git add strategyexplorer.h strategyexplorer.cpp strategyexplorer.ui
git commit -m "Add 'your hand' recommendation banner to results screen for Modo Rápido"
```

---

### Task 7: End-to-end manual verification

**Files:** none (verification only)

- [ ] **Step 1: Full rebuild**

```bash
cd build-i18n && qmake ../TexasSolverGui.pro && mingw32-make -j4 2>&1 | grep -E "error:"
ls -la release/SolverixGui.exe
```
Expected: exe exists, fresh timestamp, zero errors.

- [ ] **Step 2: Launch and walk through Modo Rápido**

Launch `release/SolverixGui.exe`. On the welcome dialog, click "🚀 Modo Rápido". Pick "Abriste desde el Botón (BTN) y el rival pagó desde la ciega grande (BB)". Pick any 2 cards (e.g. click As, then Kh). Confirm the "Seleccionadas" label updates to show both. Click "Siguiente →". On the board step, pick 3 board cards via the existing board-selector button. Click "Resolver →". Confirm the app doesn't freeze (build-tree then solve run in sequence) and the results screen opens automatically with a visible "Con [hand]: ..." banner showing at least one action with a percentage.

- [ ] **Step 3: Confirm Modo Avanzado still works unchanged**

Close the app, relaunch, click "🔧 Modo Avanzado" on the welcome dialog. Confirm the existing 6-step wizard behaves exactly as before (this was working prior to this plan — the check here is regression, not new behavior).

- [ ] **Step 4: Fix anything broken, then final commit**

If Step 2 or 3 surfaced a bug, fix it directly, rebuild, re-verify, then:

```bash
git add -A
git commit -m "Fix issues found in Modo Rápido end-to-end verification"
```

(Skip this commit if nothing needed fixing.)
