# Theme System (Dark Modern + Light) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a QSS-based theming system to TexasSolverGui with a Dark Modern theme (default) and a Light theme, selectable from Settings and persisted across restarts.

**Architecture:** Two static `.qss` stylesheet files (`theme_dark.qss`, `theme_light.qss`) bundled via Qt's resource system, loaded and applied with `qApp->setStyleSheet(...)` at startup based on a `QSettings` value. No C++ widget classes change — this is pure styling of existing Qt Widgets. A new combo box in the existing Settings dialog lets the user switch themes (takes effect on restart, matching the existing language-switch UX).

**Tech Stack:** Qt 5/6 Widgets, QSS (Qt Style Sheets), QSettings, qmake `.qrc` resources.

## Global Constraints

- Do not modify any file under `src/solver/`, `src/GameTree.cpp`, `src/nodes/`, `src/ranges/`, `src/compairer/` — solver logic is out of scope for this entire redesign project.
- Every new user-visible string must be wrapped in `tr()`.
- Default theme is Dark Modern.
- Dark Modern palette (exact values, reused across every step that touches color):
  - Background base: `#12141c`
  - Background panel/nav: `#0d0f16`
  - Card/panel surface: `#1a1d29`
  - Border: `#2a2d3a`
  - Text primary: `#e8e8f0`
  - Text secondary: `#8b8fa3`
  - Accent primary: `#5b6ef5`
  - Accent primary hover: `#7280f7`
  - Accent primary pressed: `#4757d1`
  - Action call/positive: `#3ddc84`
  - Action raise/warning: `#ffb44f`
  - Action fold/danger: `#ff5c5c`
- Light theme palette (exact values):
  - Background base: `#f7f8fa`
  - Card/panel surface: `#ffffff`
  - Border: `#e2e4ea`
  - Text primary: `#1a1d29`
  - Text secondary: `#5a5f73`
  - Accent primary: `#3d5afe`
  - Accent primary hover: `#5570ff`
  - Accent primary pressed: `#2c44d6`
  - Action call/positive: `#2fb872`
  - Action raise/warning: `#f5a623`
  - Action fold/danger: `#e5484d`
- Build/verify command for every task in this plan (run from repo root):
  ```
  powershell -Command "$env:PATH='D:\QT\6.11.2\mingw_64\bin;D:\QT\Tools\mingw1310_64\bin;' + $env:PATH; if (!(Test-Path build-theme)) { New-Item -ItemType Directory build-theme | Out-Null }; cd build-theme; & 'D:\QT\6.11.2\mingw_64\bin\qmake.exe' ..\TexasSolverGui.pro; mingw32-make -j4"
  ```
  Expected on success: no lines containing `error:` in the output, and `build-theme\release\TexasSolverGui.exe` exists (or `build-theme\debug\` depending on configured build type — check whichever the qmake run produced).
- This project has no existing automated test suite for the GUI (confirmed: no test framework wired into `TexasSolverGui.pro`). Verification for every task in this plan is: (1) it compiles clean per the command above, (2) the app launches without crashing, (3) a manual visual check against the specific criteria listed in the step. This is a deliberate, documented deviation from unit-test-per-step — do not invent a test framework for this.

---

## File Structure

- Create: `resources/themes/theme_dark.qss` — Dark Modern stylesheet
- Create: `resources/themes/theme_light.qss` — Light stylesheet
- Create: `themes.qrc` — Qt resource file bundling both `.qss` files (same pattern as existing `compairer.qrc`, `translations.qrc`)
- Modify: `TexasSolverGui.pro` — add `themes.qrc` to `RESOURCES`
- Modify: `main.cpp` — load theme preference from `QSettings`, apply stylesheet before `MainWindow w; w.show();`
- Modify: `settingeditor.ui` — add `themeBox` (`QComboBox`) with a `QLabel` next to the existing `languageBox` row
- Modify: `settingeditor.h` — declare `on_themeBox_currentIndexChanged(int index)` slot
- Modify: `settingeditor.cpp` — read/write theme setting, show restart-needed message on change (mirrors existing language logic)

---

### Task 1: Dark Modern stylesheet file

**Files:**
- Create: `resources/themes/theme_dark.qss`

**Interfaces:**
- Produces: a `.qss` file consumable by `qApp->setStyleSheet(QString)` — no code interface, just the file's existence and content at this path.

- [ ] **Step 1: Create the directory and file**

Create `resources/themes/theme_dark.qss` with this content:

```css
/* TexasSolver - Dark Modern theme */

QWidget {
    background-color: #12141c;
    color: #e8e8f0;
    font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
    font-size: 13px;
}

QMainWindow, QDialog {
    background-color: #12141c;
}

QLabel {
    background: transparent;
    color: #e8e8f0;
}

QLabel:disabled {
    color: #8b8fa3;
}

QPushButton {
    background-color: #5b6ef5;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 600;
}

QPushButton:hover {
    background-color: #7280f7;
}

QPushButton:pressed {
    background-color: #4757d1;
}

QPushButton:disabled {
    background-color: #2a2d3a;
    color: #8b8fa3;
}

QPushButton[flat="true"], QToolButton {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    padding: 4px 10px;
}

QToolButton:hover {
    background-color: #2a2d3a;
}

QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    padding: 4px 8px;
    selection-background-color: #5b6ef5;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
    border: 1px solid #5b6ef5;
}

QLineEdit:disabled, QTextEdit:disabled, QSpinBox:disabled {
    color: #8b8fa3;
    background-color: #12141c;
}

QComboBox {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    padding: 4px 8px;
    min-height: 20px;
}

QComboBox:hover {
    border: 1px solid #5b6ef5;
}

QComboBox::drop-down {
    border: none;
    width: 20px;
}

QComboBox QAbstractItemView {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    selection-background-color: #5b6ef5;
    selection-color: #ffffff;
    outline: none;
}

QGroupBox {
    background-color: #1a1d29;
    border: 1px solid #2a2d3a;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 14px;
    font-weight: 600;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: #8b8fa3;
}

QCheckBox, QRadioButton {
    background: transparent;
    color: #e8e8f0;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #2a2d3a;
    background-color: #1a1d29;
}

QCheckBox::indicator {
    border-radius: 3px;
}

QRadioButton::indicator {
    border-radius: 8px;
}

QCheckBox::indicator:checked, QRadioButton::indicator:checked {
    background-color: #5b6ef5;
    border: 1px solid #5b6ef5;
}

QTabWidget::pane {
    background-color: #12141c;
    border: 1px solid #2a2d3a;
    border-radius: 8px;
}

QTabBar::tab {
    background-color: #0d0f16;
    color: #8b8fa3;
    border: 1px solid #2a2d3a;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 6px 16px;
    margin-right: 2px;
}

QTabBar::tab:selected {
    background-color: #1a1d29;
    color: #e8e8f0;
}

QTabBar::tab:hover:!selected {
    color: #e8e8f0;
}

QTableView, QTreeView, QListView {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    gridline-color: #2a2d3a;
    selection-background-color: #5b6ef5;
    selection-color: #ffffff;
}

QHeaderView::section {
    background-color: #0d0f16;
    color: #8b8fa3;
    border: none;
    border-right: 1px solid #2a2d3a;
    border-bottom: 1px solid #2a2d3a;
    padding: 4px 8px;
}

QScrollBar:vertical {
    background-color: #12141c;
    width: 12px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background-color: #2a2d3a;
    border-radius: 5px;
    min-height: 24px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background-color: #5b6ef5;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background-color: #12141c;
    height: 12px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background-color: #2a2d3a;
    border-radius: 5px;
    min-width: 24px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #5b6ef5;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

QProgressBar {
    background-color: #1a1d29;
    border: 1px solid #2a2d3a;
    border-radius: 6px;
    text-align: center;
    color: #e8e8f0;
}

QProgressBar::chunk {
    background-color: #5b6ef5;
    border-radius: 5px;
}

QToolTip {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
    padding: 4px 8px;
    border-radius: 4px;
}

QMenuBar {
    background-color: #0d0f16;
    color: #e8e8f0;
}

QMenuBar::item:selected {
    background-color: #2a2d3a;
}

QMenu {
    background-color: #1a1d29;
    color: #e8e8f0;
    border: 1px solid #2a2d3a;
}

QMenu::item:selected {
    background-color: #5b6ef5;
    color: #ffffff;
}

QStatusBar {
    background-color: #0d0f16;
    color: #8b8fa3;
}

QSplitter::handle {
    background-color: #2a2d3a;
}
```

- [ ] **Step 2: Verify the file is valid UTF-8 plain text with no BOM issues**

Run: `powershell -Command "Get-Content -Raw 'resources/themes/theme_dark.qss' | Measure-Object -Character"`
Expected: prints a character count > 0, no errors.

- [ ] **Step 3: Commit**

```bash
git add resources/themes/theme_dark.qss
git commit -m "Add Dark Modern QSS stylesheet"
```

---

### Task 2: Light theme stylesheet file

**Files:**
- Create: `resources/themes/theme_light.qss`

**Interfaces:**
- Produces: a `.qss` file, same role as Task 1's output but with the light palette from Global Constraints.

- [ ] **Step 1: Create the file**

Create `resources/themes/theme_light.qss` with this content (same structure as `theme_dark.qss`, light palette substituted):

```css
/* TexasSolver - Light theme */

QWidget {
    background-color: #f7f8fa;
    color: #1a1d29;
    font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
    font-size: 13px;
}

QMainWindow, QDialog {
    background-color: #f7f8fa;
}

QLabel {
    background: transparent;
    color: #1a1d29;
}

QLabel:disabled {
    color: #5a5f73;
}

QPushButton {
    background-color: #3d5afe;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 600;
}

QPushButton:hover {
    background-color: #5570ff;
}

QPushButton:pressed {
    background-color: #2c44d6;
}

QPushButton:disabled {
    background-color: #e2e4ea;
    color: #5a5f73;
}

QPushButton[flat="true"], QToolButton {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    padding: 4px 10px;
}

QToolButton:hover {
    background-color: #eef0f5;
}

QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    padding: 4px 8px;
    selection-background-color: #3d5afe;
    selection-color: #ffffff;
}

QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
    border: 1px solid #3d5afe;
}

QLineEdit:disabled, QTextEdit:disabled, QSpinBox:disabled {
    color: #5a5f73;
    background-color: #f7f8fa;
}

QComboBox {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    padding: 4px 8px;
    min-height: 20px;
}

QComboBox:hover {
    border: 1px solid #3d5afe;
}

QComboBox::drop-down {
    border: none;
    width: 20px;
}

QComboBox QAbstractItemView {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    selection-background-color: #3d5afe;
    selection-color: #ffffff;
    outline: none;
}

QGroupBox {
    background-color: #ffffff;
    border: 1px solid #e2e4ea;
    border-radius: 8px;
    margin-top: 12px;
    padding-top: 14px;
    font-weight: 600;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 10px;
    padding: 0 4px;
    color: #5a5f73;
}

QCheckBox, QRadioButton {
    background: transparent;
    color: #1a1d29;
    spacing: 8px;
}

QCheckBox::indicator, QRadioButton::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #e2e4ea;
    background-color: #ffffff;
}

QCheckBox::indicator {
    border-radius: 3px;
}

QRadioButton::indicator {
    border-radius: 8px;
}

QCheckBox::indicator:checked, QRadioButton::indicator:checked {
    background-color: #3d5afe;
    border: 1px solid #3d5afe;
}

QTabWidget::pane {
    background-color: #f7f8fa;
    border: 1px solid #e2e4ea;
    border-radius: 8px;
}

QTabBar::tab {
    background-color: #eef0f5;
    color: #5a5f73;
    border: 1px solid #e2e4ea;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 6px 16px;
    margin-right: 2px;
}

QTabBar::tab:selected {
    background-color: #ffffff;
    color: #1a1d29;
}

QTabBar::tab:hover:!selected {
    color: #1a1d29;
}

QTableView, QTreeView, QListView {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    gridline-color: #e2e4ea;
    selection-background-color: #3d5afe;
    selection-color: #ffffff;
}

QHeaderView::section {
    background-color: #eef0f5;
    color: #5a5f73;
    border: none;
    border-right: 1px solid #e2e4ea;
    border-bottom: 1px solid #e2e4ea;
    padding: 4px 8px;
}

QScrollBar:vertical {
    background-color: #f7f8fa;
    width: 12px;
    margin: 0;
}

QScrollBar::handle:vertical {
    background-color: #e2e4ea;
    border-radius: 5px;
    min-height: 24px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background-color: #3d5afe;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QScrollBar:horizontal {
    background-color: #f7f8fa;
    height: 12px;
    margin: 0;
}

QScrollBar::handle:horizontal {
    background-color: #e2e4ea;
    border-radius: 5px;
    min-width: 24px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background-color: #3d5afe;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0;
}

QProgressBar {
    background-color: #ffffff;
    border: 1px solid #e2e4ea;
    border-radius: 6px;
    text-align: center;
    color: #1a1d29;
}

QProgressBar::chunk {
    background-color: #3d5afe;
    border-radius: 5px;
}

QToolTip {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
    padding: 4px 8px;
    border-radius: 4px;
}

QMenuBar {
    background-color: #ffffff;
    color: #1a1d29;
}

QMenuBar::item:selected {
    background-color: #eef0f5;
}

QMenu {
    background-color: #ffffff;
    color: #1a1d29;
    border: 1px solid #e2e4ea;
}

QMenu::item:selected {
    background-color: #3d5afe;
    color: #ffffff;
}

QStatusBar {
    background-color: #ffffff;
    color: #5a5f73;
}

QSplitter::handle {
    background-color: #e2e4ea;
}
```

- [ ] **Step 2: Verify the file is valid UTF-8 plain text**

Run: `powershell -Command "Get-Content -Raw 'resources/themes/theme_light.qss' | Measure-Object -Character"`
Expected: prints a character count > 0, no errors.

- [ ] **Step 3: Commit**

```bash
git add resources/themes/theme_light.qss
git commit -m "Add Light QSS stylesheet"
```

---

### Task 3: Bundle stylesheets as Qt resources

**Files:**
- Create: `themes.qrc`
- Modify: `TexasSolverGui.pro:1-30` (the `RESOURCES` line, currently `RESOURCES += \    translations.qrc \    compairer.qrc`)

**Interfaces:**
- Consumes: `resources/themes/theme_dark.qss`, `resources/themes/theme_light.qss` from Tasks 1-2.
- Produces: Qt resource paths `:/resources/themes/theme_dark.qss` and `:/resources/themes/theme_light.qss`, loadable via `QFile`.

- [ ] **Step 1: Create `themes.qrc`**

```xml
<RCC>
    <qresource prefix="/">
        <file>resources/themes/theme_dark.qss</file>
        <file>resources/themes/theme_light.qss</file>
    </qresource>
</RCC>
```

- [ ] **Step 2: Add it to the qmake project**

In `TexasSolverGui.pro`, find:

```
RESOURCES += \
    translations.qrc \
    compairer.qrc
```

Replace with:

```
RESOURCES += \
    translations.qrc \
    compairer.qrc \
    themes.qrc
```

- [ ] **Step 3: Build to confirm the resource compiles**

Run the build command from Global Constraints.
Expected: no `error:` lines; build succeeds (this only proves the `.qrc` is well-formed and linked — the stylesheet isn't applied to anything yet).

- [ ] **Step 4: Commit**

```bash
git add themes.qrc TexasSolverGui.pro
git commit -m "Bundle theme stylesheets as Qt resources"
```

---

### Task 4: Load and apply the theme at startup

**Files:**
- Modify: `main.cpp:40-82` (the `main()` function)

**Interfaces:**
- Consumes: `QSettings("TexasSolver", "Setting")` group `"solver"`, existing pattern already used for `language` and `dump_round` keys in this same function.
- Produces: a `"theme"` key in that same `QSettings` group, with value `"dark"` or `"light"`. This is the contract `settingeditor.cpp` (Task 5) reads and writes.

- [ ] **Step 1: Add theme loading logic**

In `main.cpp`, after the existing language-loading block (after `a.installTranslator(&trans);` in the `else` branch, i.e. right before `int dump_round = setting.value("dump_round").toInt();`), insert:

```cpp
    QString theme_str = setting.value("theme").toString();
    if(theme_str != "dark" && theme_str != "light"){
        theme_str = "dark";
        setting.setValue("theme", theme_str);
    }
    QFile themeFile(theme_str == "dark" ? ":/resources/themes/theme_dark.qss" : ":/resources/themes/theme_light.qss");
    if(themeFile.open(QFile::ReadOnly | QFile::Text)){
        QTextStream themeStream(&themeFile);
        a.setStyleSheet(themeStream.readAll());
        themeFile.close();
    }
```

Add the required includes at the top of `main.cpp` (alongside the existing `#include <QSettings>`):

```cpp
#include <QFile>
#include <QTextStream>
```

- [ ] **Step 2: Build**

Run the build command from Global Constraints.
Expected: no `error:` lines.

- [ ] **Step 3: Manual visual verification**

Run: `& 'build-theme\release\TexasSolverGui.exe'` (or `debug\` if that's what was built — check which directory Step 2 populated). If it fails to launch with a missing-DLL error, copy `Qt6Core.dll`, `Qt6Gui.dll`, `Qt6Widgets.dll` from `D:\QT\6.11.2\mingw_64\bin` and `libgomp-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`, `libgcc_s_seh-1.dll` from `D:\QT\Tools\mingw1310_64\bin` into the same folder as the `.exe`, then retry.

Expected: the main window opens with a dark background (`#12141c`-ish), light text, and buttons rendered in the `#5b6ef5` blue-violet accent color — not the default white/gray Qt look.

- [ ] **Step 4: Commit**

```bash
git add main.cpp
git commit -m "Apply theme stylesheet at application startup"
```

---

### Task 5: Theme selector in Settings dialog

**Files:**
- Modify: `settingeditor.ui:16-56` (add a new row mirroring the existing language row)
- Modify: `settingeditor.h:20-29`
- Modify: `settingeditor.cpp` (constructor, `on_confirmBox_accepted`, new slot)

**Interfaces:**
- Consumes: `QSettings` `"theme"` key contract from Task 4.
- Produces: nothing consumed by later tasks in this plan — this is the last task.

- [ ] **Step 1: Add the UI row**

In `settingeditor.ui`, immediately after the closing `</layout>` of `horizontalLayout` (the language row, ends at line 55) and before the `<item>` that opens `horizontalLayout_2` (line 57), insert a new item containing a new horizontal layout:

```xml
     <item>
      <layout class="QHBoxLayout" name="horizontalLayout_3">
       <item>
        <widget class="QLabel" name="label_3">
         <property name="text">
          <string>Theme</string>
         </property>
        </widget>
       </item>
       <item>
        <widget class="QComboBox" name="themeBox">
         <item>
          <property name="text">
           <string>Dark</string>
          </property>
         </item>
         <item>
          <property name="text">
           <string>Light</string>
          </property>
         </item>
        </widget>
       </item>
       <item>
        <spacer name="horizontalSpacer_3">
         <property name="orientation">
          <enum>Qt::Horizontal</enum>
         </property>
         <property name="sizeHint" stdset="0">
          <size>
           <width>40</width>
           <height>20</height>
          </size>
         </property>
        </spacer>
       </item>
      </layout>
     </item>
```

- [ ] **Step 2: Declare the new slot**

In `settingeditor.h`, change:

```cpp
private slots:
    void on_confirmBox_accepted();

    void on_languageBox_currentIndexChanged(int index);
```

to:

```cpp
private slots:
    void on_confirmBox_accepted();

    void on_languageBox_currentIndexChanged(int index);

    void on_themeBox_currentIndexChanged(int index);
```

- [ ] **Step 3: Initialize the combo box from saved settings**

In `settingeditor.cpp`, in the constructor, after the existing block that sets `languageBox`'s index (after the `qDebug()...` `else` branch closes, before `int dump_round = ...`), add:

```cpp
    QString theme_str = setting.value("theme").toString();
    if(theme_str == "dark"){
        this->ui->themeBox->setCurrentIndex(0);
    }else if(theme_str == "light"){
        this->ui->themeBox->setCurrentIndex(1);
    }else{
        qDebug().noquote() << tr("Unknown theme: ") << theme_str << tr("Setting fail");
    }
```

- [ ] **Step 4: Persist the choice on accept**

In `settingeditor.cpp`, in `on_confirmBox_accepted()`, after the existing `setting.setValue("language",language_str);` line, add:

```cpp
    QString theme = this->ui->themeBox->currentText();
    QString theme_str;
    if(theme == "Dark"){
        theme_str = "dark";
    }else if(theme == "Light"){
        theme_str = "light";
    }else{
        qDebug().noquote() << tr("Unknown theme: ") << theme << tr("Setting fail");
    }
    setting.setValue("theme",theme_str);
```

- [ ] **Step 5: Show restart-needed message on change**

In `settingeditor.cpp`, add the new slot implementation at the end of the file:

```cpp
void SettingEditor::on_themeBox_currentIndexChanged(int index)
{
    if(!initized)return;
    QString message = tr("Restart program to make theme selection effective.");
    qDebug().noquote() << message;
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}
```

- [ ] **Step 6: Build**

Run the build command from Global Constraints.
Expected: no `error:` lines.

- [ ] **Step 7: Manual verification**

Run the built `.exe` (same DLL-copy caveat as Task 4 Step 3 if needed). Open the Settings dialog (menu item that constructs `SettingEditor` — confirm via `grep -n "new SettingEditor" mainwindow.cpp` if unsure where it's triggered from). Confirm:
- A "Theme" row appears with a dropdown showing "Dark" / "Light", defaulted to whichever theme is currently active.
- Switching the dropdown shows the "restart to take effect" message box.
- Closing the app, changing `theme` to `light` manually via the dropdown + OK, and relaunching shows the light palette (`#f7f8fa` background, dark text, `#3d5afe` accent buttons).

- [ ] **Step 8: Commit**

```bash
git add settingeditor.ui settingeditor.h settingeditor.cpp
git commit -m "Add theme selector to Settings dialog"
```

---

## Self-Review Notes

- **Spec coverage:** Section 1 of the design spec ("Sistema de temas") is fully covered: both palettes, QSettings persistence, `.qrc` loading pattern matching existing translations, default-dark behavior. Icon set (mentioned in the spec as future work) is intentionally deferred — it depends on UI elements (wizard nav, toolbar) that don't exist yet and will be added in the wizard/results plans; recoloring rules for it will be added to the QSS at that point.
- **Type/name consistency:** `theme` QSettings key, values `"dark"`/`"light"` (lowercase, matching the existing `"EN"`/`"CN"` convention style but chosen lowercase to avoid confusion with the `language` key's uppercase codes) is used consistently across Task 4 (writer/reader) and Task 5 (writer/reader).
- **No placeholders:** every step has complete, copy-pasteable code or exact commands.
