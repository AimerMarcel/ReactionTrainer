// Headless logic test for the platform-independent core. Runs on Linux/macOS/Windows:
//   g++ -std=c++17 -O1 -Wall -Wextra tests/test_core.cpp -o core_tests && ./core_tests
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "../src/trainer.hpp"

using namespace rt;

struct Null : Canvas {
    int items = 0;
    void clear(Color) override {}
    void text(double, double, const std::string&, int, Anchor, Color, double, int) override { items++; }
    void rect(double, double, double, double, Color, const Color*, int) override { items++; }
    void line(double, double, double, double, Color, int) override { items++; }
    void poly(const std::vector<Pt>&, Color, const Color*, int) override { items++; }
    void oval(double, double, double, Color, const Color*, int) override { items++; }
};

#define CHECK(c) do { if (!(c)) { std::printf("FAIL line %d: %s\n", __LINE__, #c); std::exit(1); } } while (0)

int main() {
#ifdef _WIN32
    _putenv_s("RT_DATA_DIR", "rt_test_data");
#else
    setenv("RT_DATA_DIR", "/tmp/rt_test_data", 1);
#endif
    std::remove((Store::dir() + "/data.txt").c_str());

    double T = 0;
    Trainer app([&] { return T; });
    int beeps = 0;
    std::vector<std::string> spoken;
    app.onBeep = [&] { beeps++; };
    app.onSpeak = [&](const std::string& s) { spoken.push_back(s); };
    Null cv;
    auto frame = [&] { T += 25; app.update(); app.draw(cv, 1100, 720); app.presented(); };
    auto center = [&](const Trainer::Region& r) { return std::pair<int, int>((int)((r.x1 + r.x2) / 2), (int)((r.y1 + r.y2) / 2)); };

    frame();
    CHECK(cv.items > 20);
    CHECK(app.screen == Trainer::MENU);

    // play the current selection to the result screen, with one deliberate wrong click in some games
    auto session = [&](int game) {
        app.begin();
        for (int step = 0; step < 8000 && app.screen == Trainer::PLAY; step++) {
            frame();
            auto& g = app.g;
            T += 5;
            if (game <= ART) {
                if (g.phase == Trainer::GO) { T += 230; app.key(0x0D); }
            } else if (game == SCH && g.phase == Trainer::RUN) {
                int k = (int)(std::find(g.nums.begin(), g.nums.end(), g.cur) - g.nums.begin());
                int pi = 0;
                for (int j = 0; j < k; j++) if (g.nums[j] >= g.cur) pi++;
                CHECK(pi < (int)app.polys.size());
                double sx = 0, sy = 0;
                for (auto& p : app.polys[pi].pts) { sx += p.x; sy += p.y; }
                T += 300; app.click((int)(sx / 4), (int)(sy / 4));
            } else if (game == TSW && g.phase == Trainer::RUN) {
                int base = (int)app.regions.size() - g.r * g.c;
                if (g.idx == 1) { auto c = center(app.regions[base + (g.lit + 1) % (g.r * g.c)]); app.click(c.first, c.second); frame(); }
                auto c = center(app.regions[base + g.lit]);
                T += 280; app.click(c.first, c.second);
            } else if (game == STR && g.phase == Trainer::RUN) {
                CHECK(g.word != g.ink);
                int base = (int)app.regions.size() - 9, slot = 0;
                for (int k = 0; k < 9; k++) if (g.order[k] == g.ink) slot = k;
                if (g.idx == 1) { auto c = center(app.regions[base + (slot + 1) % 9]); app.click(c.first, c.second); frame(); }
                auto c = center(app.regions[base + slot]);
                T += 400; app.click(c.first, c.second);
            }
        }
        CHECK(app.screen == Trainer::RESULT);
        CHECK(app.g.rec.done == 5 && (int)app.g.rec.vals.size() == 5);
        frame();
    };

    int sessions = 0;
    for (int game = 0; game < 5; game++)
        for (int di = 0; di < 3; di++) {
            app.sel = game; app.diff = di; app.rounds = 5; app.schSize = 3 + di;
            app.spinN = app.spinT = (game == SCH && di >= 1);
            session(game);
            if (game == TSW || game == STR) CHECK(app.g.rec.miss >= 1);
            if (game == ART) CHECK(beeps > 0);
            sessions++;
        }
    // Schulte 6x6 (with and without spin)
    for (int spin = 0; spin < 2; spin++) {
        app.sel = SCH; app.diff = 1; app.rounds = 5; app.schSize = 6; app.spinN = app.spinT = spin;
        session(SCH);
        CHECK(app.g.n == 6 && (int)app.g.nums.size() == 36);
        CHECK(app.g.rec.mods == (spin ? "6NT" : "6"));
        sessions++;
    }
    CHECK(sessions == 17);
    CHECK(app.store.runs.size() == 17);
    CHECK(!spoken.empty());                       // color trial spoke the written word
    CHECK(spoken.size() >= 3 * 5);

    // color trial: the spoken word is the WRITTEN word, and the draining timer ends a word
    spoken.clear();
    app.sel = STR; app.diff = 2; app.rounds = 5; app.begin();
    for (int i = 0; i < 100 && app.g.phase != Trainer::RUN; i++) frame();
    CHECK(spoken.size() == 1 && spoken[0] == Trainer::titleCase(COLORS[app.g.word].name));
    int idx0 = app.g.idx, miss0 = app.g.miss;
    T += 2600; frame();                            // hard limit is 2500 ms
    CHECK(app.g.idx == idx0 + 1 && app.g.miss == miss0 + 1 && app.g.vals.back() >= 2500);
    app.strAudio = false; spoken.clear();
    for (int i = 0; i < 40; i++) { T += 2600; frame(); }
    CHECK(app.screen == Trainer::RESULT && spoken.empty());
    app.strAudio = true;

    // Space restarts the run and does not save it
    size_t saved = app.store.runs.size();
    app.sel = VRT; app.diff = 0; app.begin();
    for (int i = 0; i < 200 && app.g.phase != Trainer::GO; i++) frame();
    T += 200; app.key(0x0D);
    CHECK(app.g.idx == 1);
    app.key(0x20);                                  // restart
    CHECK(app.screen == Trainer::PLAY && app.g.idx == 0 && app.g.vals.empty());
    CHECK(app.store.runs.size() == saved);
    app.back();                                     // no values -> straight back to the menu
    CHECK(app.screen == Trainer::MENU);

    // ranks
    CHECK(RANK_NAMES[rankFromScore(VRT, 160)] == std::string("CELESTIAL"));
    CHECK(RANK_NAMES[rankFromScore(VRT, 199)] == std::string("NOVA"));
    CHECK(RANK_NAMES[rankFromScore(VRT, 500)] == std::string("IRON"));
    int oi = -1; double of = 0;
    CHECK(app.overall(oi, of) && oi >= 0 && oi < 12);
    for (int i = 0; i < 5; i++) CHECK(app.gameRank(i) >= 0);
    app.setTab(2); frame();
    CHECK(app.screen == Trainer::RANK);

    // themes: every preset renders, color wheel + brightness bar edit the custom theme
    for (int i = 0; i < 6; i++) { app.cycleTheme(); frame(); }
    app.setTab(4); frame();
    CHECK(app.screen == Trainer::THEME);
    const Trainer::Region* wheel = nullptr; const Trainer::Region* bar = nullptr;
    for (auto& r : app.regions) {
        double w = r.x2 - r.x1, h = r.y2 - r.y1;
        if (std::fabs(w - h) < 0.5 && w > 60) wheel = &r;
    }
    CHECK(wheel != nullptr);
    for (auto& r : app.regions)
        if (&r != wheel && std::fabs((r.x2 - r.x1) - (wheel->x2 - wheel->x1)) < 0.5 && r.y1 > wheel->y2 - 1) bar = &r;
    CHECK(bar != nullptr);
    double R0 = (wheel->x2 - wheel->x1) / 2, wx = wheel->x1 + R0, wy = wheel->y1 + R0;
    app.click((int)(wx + R0 * 0.5), (int)wy);       // right of centre = hue 0 (red), half saturation
    CHECK(app.theme == 4 && std::fabs(app.ih) < 2 && std::fabs(app.is - 0.5) < 0.05);
    app.click((int)(bar->x1 + (bar->x2 - bar->x1) * 0.25), (int)((bar->y1 + bar->y2) / 2));
    CHECK(std::fabs(app.iv - 0.25) < 0.03);
    frame();
    app.editBg = true; frame();
    app.click((int)(wx - R0 * 0.5), (int)wy);       // left of centre = hue 180
    CHECK(std::fabs(app.bh - 180) < 2);
    app.resetCustom();

    // keybinds: rebinding, reserved keys, swapping
    app.setTab(3); frame();
    CHECK(app.screen == Trainer::KEYS);
    app.rebind = 0; app.key('X'); CHECK(app.kRestart == 'X' && app.rebind == -1);
    app.rebind = 1; app.key('R'); CHECK(app.kTab == 0x09 && app.rebind == 1);   // reserved, still waiting
    app.key(0x1B); CHECK(app.rebind == -1);
    app.rebind = 2; app.key('X'); CHECK(app.kTheme == 'X' && app.kRestart == 0x71);  // swapped
    app.rebind = 2; app.key(0x71); CHECK(app.kTheme == 0x71 && app.kRestart == 'X');
    app.rebind = 0; app.key(0x20); CHECK(app.kRestart == 0x20);
    frame();

    // tabs via keyboard
    app.setTab(0);
    for (int i = 0; i < 5; i++) { app.key(app.kTab); frame(); }
    CHECK(app.tab == 0);
    app.setTab(1); frame();

    // persistence round trip of everything new
    app.schSize = 6; app.strAudio = false; app.ih = 200; app.persist();
    Store s2; s2.load();
    CHECK(s2.runs.size() == app.store.runs.size());
    CHECK(s2.st.schSize == 6 && !s2.st.strAudio && std::fabs(s2.st.ih - 200) < 0.1);
    CHECK(s2.st.kRestart == 0x20 && s2.st.kTab == 0x09 && s2.st.theme == app.theme);
    CHECK(s2.runs.back().vals.size() == 5);

    // abort path keeps finished rounds, clear works
    app.sel = VRT; app.begin();
    for (int i = 0; i < 400 && app.g.phase != Trainer::GO; i++) frame();
    T += 200; app.key(0x0D);
    app.back();
    CHECK(app.screen == Trainer::MENU && app.store.runs.back().complete == false);
    app.setTab(1); app.clearData(); app.clearData();
    CHECK(app.store.runs.empty());
    std::printf("core tests passed (%d full sessions)\n", sessions);
    return 0;
}
