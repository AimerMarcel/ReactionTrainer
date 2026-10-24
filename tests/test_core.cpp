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
    app.onBeep = [&] { beeps++; };
    Null cv;
    auto frame = [&] { T += 25; app.update(); app.draw(cv, 1100, 720); app.presented(); };
    auto center = [&](const Trainer::Region& r) { return std::pair<int, int>((int)((r.x1 + r.x2) / 2), (int)((r.y1 + r.y2) / 2)); };

    frame();
    CHECK(cv.items > 20);

    int completed = 0;
    for (int game = 0; game < 5; game++)
        for (int di = 0; di < 3; di++) {
            app.sel = game; app.diff = di; app.rounds = 5;
            app.spinN = app.spinT = (game == SCH && di >= 1);
            app.begin();
            for (int step = 0; step < 6000 && app.screen == Trainer::PLAY; step++) {
                frame();
                auto& g = app.g;
                T += 5;
                if (game <= ART) {
                    if (g.phase == Trainer::GO) { T += 230; app.key(0x20); }
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
            if (game == TSW || game == STR) CHECK(app.g.rec.miss >= 1);
            if (game == ART) CHECK(beeps > 0);
            frame();  // draw result screen
            completed++;
        }
    CHECK(completed == 15);
    CHECK(app.store.runs.size() == 15);

    // persistence round trip
    Store s2; s2.load();
    CHECK(s2.runs.size() == 15);
    CHECK(s2.runs.back().vals.size() == 5);
    CHECK(s2.st.game == 4);

    // keyboard, tabs, data screen, abort path, clear
    app.back(); CHECK(app.screen == Trainer::MENU);
    app.key('N'); app.key('S'); app.key('D'); app.key('R'); app.key('2'); app.key(0x71);
    app.key(0x09); CHECK(app.screen == Trainer::DATA); frame();
    app.key(0x09); CHECK(app.screen == Trainer::MENU);
    app.sel = VRT; app.begin();
    for (int i = 0; i < 400 && app.g.phase != Trainer::GO; i++) frame();
    T += 200; app.key(0x0D);                       // one reaction, then abort
    app.back();
    CHECK(app.screen == Trainer::MENU && app.store.runs.back().complete == false);
    app.setTab(1); app.clearData(); app.clearData();
    CHECK(app.store.runs.empty());
    std::printf("core tests passed (%d full sessions, %d draw items on last frame)\n", completed, cv.items);
    return 0;
}
