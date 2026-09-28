# Solverix

Solverix is a GTO (Game Theory Optimal) poker solver for offline study, built as a Windows desktop app on top of the CFR (Counterfactual Regret Minimization) algorithm. It runs entirely on your own machine — no cloud, no internet dependency to solve a hand.

## Features

- **Advanced Solver**: full control over ranges, bet sizes, board and solving parameters through a guided setup wizard.
- **Quick Mode**: pick a spot from a poker table, auto-filled ranges by position (6-max and full ring), get a direct recommendation without configuring a tree by hand.
- **"What did your opponent do?"**: jump straight from a solved spot to the strategy for a specific villain action (check / small bet / big bet).
- **Practice Mode**: quiz yourself against the solver's own output.
- Multi-language UI: English, Spanish, Portuguese, Chinese.
- 5 visual themes (Dark, Light, Poker Room, Violet, Fintech).

## Building

Requires Qt 6 (Widgets, Network) and a MinGW or MSVC toolchain on Windows (MSVC on macOS/Linux is untested; the original engine this was built on top of also supported those platforms).

```
qmake TexasSolverGui.pro
mingw32-make -j4 release
```

## Project layout

- `src/{solver,nodes,ranges,runtime,tools,trainable,compairer,console,pybind,experimental}` + matching `include/` folders: the CFR solving engine.
- `src/gui` + `include/gui`: the desktop application layer (windows, dialogs, custom widgets).
- `src/data` + `include/data`: licensing, account, and quick-mode range data.
- `src/ui` + `include/ui`: Qt models/delegates used by both the engine's own views and the app layer.
- `resources/`: themes (`.qss`) and fonts.
- `installer.iss`: Inno Setup installer script.

## License

[GNU AGPL v3](https://www.gnu.org/licenses/agpl-3.0.en.html)
