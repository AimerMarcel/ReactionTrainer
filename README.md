# Reaction Trainer

A small Windows reflex-training app with a retro terminal look. Five tests, three difficulties, and every run is saved so you can watch yourself improve. One portable `.exe`, no installer, no dependencies.

## Tests

| Test | What you do |
|------|-------------|
| **Visual Reaction** | Click the instant the screen flashes. Clicking too early counts as a miss. |
| **Audio Reaction** | Click the instant you hear the beep. |
| **Schulte Table** | Click 1 to N in order. Optional **Spin Numbers** (tilted digits) and **Spin Table** (rotating grid). |
| **Target Switch** | Click the lit dot; a new one lights up after every hit. A miss adds 500 ms. |
| **Color Trial** | A color word is printed in another color. Press the button named after the color it is *printed* in (GREEN printed in red means press **Red**). A miss adds 500 ms. |

Rounds: **5 / 10 / 20**. Difficulty: **Easy / Medium / Hard** (wait times, table size, dot count, or label tricks depending on the test).

## Controls

| Key | Action |
|-----|--------|
| `1`-`5` | Pick a test |
| `R` / `D` | Cycle rounds / difficulty |
| `N` / `S` | Toggle Spin Numbers / Spin Table (Schulte) |
| `Enter` | Start / react / play again |
| `Space`, click | React (reaction tests) |
| `Tab` | Switch between TEST and DATA tabs |
| `F2` | Color theme (green, amber, red) |
| `F10` | Fullscreen |
| `Esc` | Back / abort run / quit |

Everything is also clickable with the mouse.

## Your data

Results are saved after every run (and for the finished rounds if you abort) to:

```
%USERPROFILE%\ReactionTrainer\data.txt
```

It is a plain tab-separated text file. The DATA tab shows personal bests per test and difficulty plus recent runs, and can clear everything. Each person who runs the app gets their own file.

## Download

Grab `ReactionTrainer.exe` from the [Releases](../../releases) page and run it.

> Windows SmartScreen may warn about an unsigned app. Click **More info** then **Run anyway**.

## Build from source

Requirements: Windows and either MinGW-w64 (g++) or Visual Studio Build Tools. No other libraries.

```bat
build.bat
```

or with CMake:

```bat
cmake -S . -B build -A x64
cmake --build build --config Release
```

The exe is statically linked, so it runs on any Windows PC.

## Tests

The game logic is OS independent and has a headless test that plays every game on every difficulty:

```sh
g++ -std=c++17 -O1 -Wall -Wextra tests/test_core.cpp -o core_tests && ./core_tests
```

## Publishing a release (GitHub)

```sh
git init
git add .
git commit -m "Initial release"
git branch -M main
git remote add origin https://github.com/<your-username>/ReactionTrainer.git
git push -u origin main

git tag v1.0.0
git push origin v1.0.0
```

Pushing a `v*` tag makes GitHub Actions run the tests, build the exe on Windows, and attach `ReactionTrainer.exe` and `ReactionTrainer-windows-x64.zip` to a new Release automatically. You can also run the workflow by hand from the Actions tab.

## Project layout

```
src/trainer.hpp     game logic, data storage and screen layout (no OS code)
src/main_win32.cpp  Win32 window, GDI drawing, input, timing
tests/test_core.cpp headless logic tests
.github/workflows/  CI: test, build, release
```

## Credits

Created by `AimerMarcel`. Built with the help of Claude (Anthropic).

## License

MIT, see [LICENSE](LICENSE).
