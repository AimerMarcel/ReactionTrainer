# Reaction Trainer

A small Windows reflex-training app with a retro terminal look. Five tests, three difficulties, ranks, rebindable keys, your own color themes, and every run is saved so you can watch yourself improve. One portable `.exe`, no installer, no dependencies.

## Tests

| Test | What you do |
|------|-------------|
| **Visual Reaction** | Click the instant the screen flashes. Clicking too early counts as a miss. |
| **Audio Reaction** | Click the instant you hear the beep. |
| **Schulte Table** | Click 1 to N in order. Table sizes **3x3 to 6x6**. Optional **Spin Numbers** (tilted digits) and **Spin Table** (rotating grid, faster on higher difficulty). |
| **Target Switch** | Click the lit dot; a new one lights up after every hit. A miss adds 500 ms. |
| **Color Trial** | A color word is printed in another color. Press the button named after the color it is *printed* in (GREEN printed in red means press **Red**). A draining timer bar runs on every word (5 / 3.5 / 2.5 s), and the voice reads out the **written** word to throw you off (toggle with `A`). A miss adds 500 ms. |

Rounds: **5 / 10 / 20**. Difficulty: **Easy / Medium / Hard**.

## Tabs

| Tab | What it does |
|-----|--------------|
| **TEST** | Pick a test and its options, then start. |
| **DATA** | Personal bests per test and difficulty, recent runs, clear data. |
| **RANK** | Your overall rank, a rank per test, and the full ladder. |
| **KEYS** | See every key and rebind the main ones. |
| **THEME** | Presets (green, amber, red, black and white) or make your own with a color wheel and brightness bar. |

## Ranks

Iron, Bronze, Silver, Gold, Platinum, Diamond, Jade, Master, Grandmaster, Nova, Astra, Celestial.

Each test has its own ladder, with **Visual Reaction as the reference** (average of a run, in ms):

| Rank | Visual Reaction | Rank | Visual Reaction |
|------|-----------------|------|-----------------|
| Celestial | under 170 | Gold | under 305 |
| Astra | under 185 | Silver | under 340 |
| Nova | under 200 | Bronze | under 400 |
| Grandmaster | under 215 | Iron | 400 and slower |
| Master | under 230 | | |
| Jade | under 245 | | |
| Diamond | under 260 | | |
| Platinum | under 280 | | |

Your rank in a test comes from your best finished run. Harder difficulties count a little better (Medium x0.93, Hard x0.86), and Schulte is scored per number so different table sizes compare fairly. Your overall rank is the average across the tests you have played. The other ladders are in `RANKT` in `src/trainer.hpp`.

## Controls

| Key | Action |
|-----|--------|
| `1`-`5` | Pick a test |
| `R` / `D` | Cycle rounds / difficulty |
| `N` / `S` | Toggle Spin Numbers / Spin Table (Schulte) |
| `A` | Toggle word audio (Color Trial) |
| `Enter` | Start / react / play again |
| click | React (reaction tests) and click targets |
| `Space` | **Restart the current run** (rebindable) |
| `Tab` | Next tab (rebindable) |
| `F2` | Next color theme (rebindable) |
| `F10` | Fullscreen (rebindable) |
| `Esc` | Back / abort run (saves finished rounds) / quit |

## Your data

Results and settings are saved after every run (and for the finished rounds if you abort) to:

```
%USERPROFILE%\ReactionTrainer\data.txt
```

It is a plain tab-separated text file. Each person who runs the app gets their own file.

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

The game logic is OS independent and has a headless test that plays every game, size and difficulty and checks ranks, themes, keybinds and saving:

```sh
g++ -std=c++17 -O1 -Wall -Wextra tests/test_core.cpp -o core_tests && ./core_tests
```

## Publishing a release (GitHub)

Easiest: double-click `publish.bat`, type a version like `v1.1.0` and a short description. It commits, pushes and tags for you.

By hand:

```sh
git add .
git commit -m "Describe your change"
git push
git tag v1.1.0
git push origin v1.1.0
```

Pushing a `v*` tag makes GitHub Actions run the tests, build the exe on Windows, and attach `ReactionTrainer.exe` and `ReactionTrainer-windows-x64.zip` to a new Release automatically. Always use a new version number for each release.

## Project layout

```
src/trainer.hpp     game logic, ranks, themes, data storage and screen layout (no OS code)
src/main_win32.cpp  Win32 window, GDI drawing, input, timing, text-to-speech
tests/test_core.cpp headless logic tests
.github/workflows/  CI: test, build, release
```

## Credits

Created by AimerMarcel.

## License

MIT, see [LICENSE](LICENSE).
