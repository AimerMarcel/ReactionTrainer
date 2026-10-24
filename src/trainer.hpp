// Reaction Trainer - platform independent core (game logic, data, layout).
// The Win32 front end lives in main_win32.cpp; this file has no OS dependencies.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace rt {

struct Color { int r = 0, g = 0, b = 0; };
inline Color mix(Color a, Color b, double t) {
    return {(int)std::lround(a.r * (1 - t) + b.r * t), (int)std::lround(a.g * (1 - t) + b.g * t),
            (int)std::lround(a.b * (1 - t) + b.b * t)};
}
struct Pt { double x, y; };
enum Anchor { NW, W, C, NE, SW };
enum { DIM = 1, INV = 2 };

// Drawing surface implemented by the front end (GDI) and by the tests.
struct Canvas {
    virtual ~Canvas() {}
    virtual void clear(Color bg) = 0;
    virtual void text(double x, double y, const std::string& s, int pt, Anchor a, Color c, double angleDeg, int wrap) = 0;
    virtual void rect(double x1, double y1, double x2, double y2, Color outline, const Color* fill, int w) = 0;
    virtual void line(double x1, double y1, double x2, double y2, Color c, int w) = 0;
    virtual void poly(const std::vector<Pt>& p, Color outline, const Color* fill, int w) = 0;
    virtual void oval(double cx, double cy, double r, Color outline, const Color* fill, int w) = 0;
};

inline std::string F(const char* f, ...) {
    char buf[600];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return buf;
}

struct Theme { const char* name; Color ink, bg; };
inline const Theme THEMES[3] = {{"GREEN", {0x3c, 0xff, 0x6e}, {0x03, 0x12, 0x08}},
                                {"AMBER", {0xff, 0xb0, 0x2e}, {0x14, 0x0b, 0x00}},
                                {"RED", {0xff, 0x3a, 0x30}, {0x10, 0x00, 0x00}}};

enum GameId { VRT, ART, SCH, TSW, STR };
struct GameInfo { const char* code; const char* name; const char* desc; };
inline const GameInfo GAMES[5] = {
    {"VRT", "VISUAL REACTION", "Click the instant the screen flashes."},
    {"ART", "AUDIO REACTION", "Click the instant you hear the beep."},
    {"SCH", "SCHULTE TABLE", "Click the numbers in order, 1 up to the last, as fast as you can."},
    {"TSW", "TARGET SWITCH", "Click the lit dot. A new one lights up after each hit. A miss costs 500 ms."},
    {"STR", "COLOR TRIAL",
     "A color word is printed in a different color. Press the button named after the color it is PRINTED in. "
     "Example: GREEN printed in red means press Red. A miss costs 500 ms."}};
inline const char* DIFFS[3] = {"EASY", "MEDIUM", "HARD"};
inline const int ROUNDS[3] = {5, 10, 20};
inline const char* DETAIL[5][3] = {
    {"Wait 1.5-3 s", "Wait 1.5-5 s", "Wait 0.8-6.5 s"},
    {"Wait 1.5-3 s", "Wait 1.5-5 s", "Wait 0.8-6.5 s"},
    {"3x3 table (1-9)", "4x4 table (1-16)", "5x5 table (1-25)"},
    {"2x3 grid (6 dots)", "3x3 grid (9 dots)", "4x4 grid (16 dots)"},
    {"Label colors stay fixed", "Label colors change every word", "Colors change + buttons shuffle"}};
struct ColorDef { const char* name; Color c; };
inline const ColorDef COLORS[9] = {{"PINK", {0xff, 0x7a, 0xd9}},   {"RED", {0xff, 0x1f, 0x1f}},
                                   {"PURPLE", {0x9b, 0x30, 0xff}}, {"YELLOW", {0xff, 0xe1, 0x3a}},
                                   {"WHITE", {0xff, 0xff, 0xff}},  {"BLACK", {0x5f, 0x5f, 0x5f}},
                                   {"GREEN", {0x22, 0xe0, 0x4a}},  {"BLUE", {0x2a, 0x4d, 0xff}},
                                   {"ORANGE", {0xff, 0x9a, 0x1a}}};

inline int gameIndex(const std::string& code) {
    for (int i = 0; i < 5; i++) if (code == GAMES[i].code) return i;
    return 0;
}
inline std::string fmtv(int game, double ms) {
    if (ms < 0) return "--";
    return game == SCH ? F("%.2fs", ms / 1000) : F("%.1fms", ms);
}
inline const char* rating(int game, int di, double avg) {
    static const char* names[5] = {"EXCEPTIONAL", "EXCELLENT", "GOOD", "FAIR", "KEEP PRACTICING"};
    double t[4];
    if (game == VRT || game == ART) { double v[4] = {200, 250, 300, 400}; std::copy(v, v + 4, t); }
    else if (game == SCH) { int n = 3 + di; avg /= n * n; double v[4] = {450, 650, 900, 1300}; std::copy(v, v + 4, t); }
    else if (game == STR) { double v[4] = {700, 900, 1150, 1500}; std::copy(v, v + 4, t); }
    else { double v[4] = {300, 400, 550, 700}; std::copy(v, v + 4, t); }
    for (int i = 0; i < 4; i++) if (avg < t[i]) return names[i];
    return names[4];
}

// ---------------------------------------------------------------- data
struct Run {
    std::string t, game, diff, mods;
    int rounds = 0, done = 0, miss = 0;
    bool complete = true;
    double avg = 0, best = 0, total = 0;
    std::vector<double> vals;
};
struct Settings { int theme = 0, game = 0, rounds = 10, diff = 1; bool spinN = false, spinT = false; };

inline std::vector<std::string> split(const std::string& s, char d) {
    std::vector<std::string> o;
    std::string cur;
    for (char c : s) { if (c == d) { o.push_back(cur); cur.clear(); } else cur += c; }
    o.push_back(cur);
    return o;
}

struct Store {
    Settings st;
    std::vector<Run> runs;
    static std::string dir() {
        const char* o = std::getenv("RT_DATA_DIR");
        if (o && *o) return o;
        const char* h = std::getenv("USERPROFILE");
        if (!h) h = std::getenv("HOME");
        return std::string(h ? h : ".") + "/ReactionTrainer";
    }
    static std::string path() { return dir() + "/data.txt"; }
    void load() {
        std::ifstream f(path());
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto p = split(line, '\t');
            if (p[0] == "S" && p.size() >= 7) {
                st.theme = std::clamp(std::atoi(p[1].c_str()), 0, 2);
                st.game = std::clamp(std::atoi(p[2].c_str()), 0, 4);
                int r = std::atoi(p[3].c_str());
                st.rounds = (r == 5 || r == 10 || r == 20) ? r : 10;
                st.diff = std::clamp(std::atoi(p[4].c_str()), 0, 2);
                st.spinN = p[5] == "1";
                st.spinT = p[6] == "1";
            } else if (p[0] == "R" && p.size() >= 13) {
                Run r;
                r.t = p[1]; r.game = p[2]; r.diff = p[3];
                r.rounds = std::atoi(p[4].c_str()); r.done = std::atoi(p[5].c_str());
                r.complete = p[6] == "1";
                r.avg = std::atof(p[7].c_str()); r.best = std::atof(p[8].c_str()); r.total = std::atof(p[9].c_str());
                r.miss = std::atoi(p[10].c_str()); r.mods = p[11];
                for (auto& v : split(p[12], ',')) if (!v.empty()) r.vals.push_back(std::atof(v.c_str()));
                runs.push_back(r);
            }
        }
    }
    void save() const {
        std::error_code ec;
        std::filesystem::create_directories(dir(), ec);
        std::ofstream f(path(), std::ios::trunc);
        if (!f) return;
        f << "S\t" << st.theme << '\t' << st.game << '\t' << st.rounds << '\t' << st.diff << '\t'
          << (st.spinN ? 1 : 0) << '\t' << (st.spinT ? 1 : 0) << '\n';
        for (auto& r : runs) {
            f << "R\t" << r.t << '\t' << r.game << '\t' << r.diff << '\t' << r.rounds << '\t' << r.done << '\t'
              << (r.complete ? 1 : 0) << '\t' << F("%.1f", r.avg) << '\t' << F("%.1f", r.best) << '\t'
              << F("%.1f", r.total) << '\t' << r.miss << '\t' << r.mods << '\t';
            for (size_t i = 0; i < r.vals.size(); i++) f << (i ? "," : "") << F("%.1f", r.vals[i]);
            f << '\n';
        }
    }
};

// ---------------------------------------------------------------- app
using Fn = std::function<void()>;

struct Trainer {
    enum Screen { MENU, DATA, PLAY, RESULT };
    enum Phase { READY, WAIT, GO, SHOW, RUN, DONE };
    struct Region { double x1, y1, x2, y2; Fn cb; };
    struct PolyRegion { std::vector<Pt> pts; Fn cb; };
    struct G {
        int game = 0, di = 0, rounds = 0, idx = 0, miss = 0;
        std::vector<double> vals;
        Phase phase = READY;
        double t0 = 0, spin0 = 0, pen = 0;
        int n = 0, cur = 1, lit = -1, r = 0, c = 0, word = 0, ink = 0;
        std::vector<int> nums, ang, order, lab;
        bool spinN = false, spinT = false, newbest = false, hasRec = false;
        Run rec;
    };

    // platform hooks
    std::function<double()> clock;
    Fn onBeep, onQuit, onToggleFs;

    Store store;
    int theme = 0, sel = 0, rounds = 10, diff = 1;
    bool spinN = false, spinT = false;
    Screen screen = MENU;
    int tab = 0;
    bool flash = false, confirm = false, dirty = true, awaitPaint = false;
    int bad = -1;
    std::string msg;
    G g;
    std::vector<Region> regions;
    std::vector<PolyRegion> polys;
    std::mt19937 rng;

    struct Job { double due = 0; Fn fn; bool active = false; } job;
    double badDue = 0, lastSpin = 0;

    Canvas* cv = nullptr;
    int W = 1100, H = 720, fs = 14, big = 30;
    double cx1 = 0, cx2 = 0, top = 0, bot = 0;

    explicit Trainer(std::function<double()> clk) : clock(std::move(clk)) {
        store.load();
        theme = store.st.theme; sel = store.st.game; rounds = store.st.rounds; diff = store.st.diff;
        spinN = store.st.spinN; spinT = store.st.spinT;
        rng.seed((unsigned)std::time(nullptr) ^ (unsigned)(clock() * 1000.0));
    }

    // ---- helpers
    double rnd(double lo, double hi) { return std::uniform_real_distribution<double>(lo, hi)(rng); }
    int rint(int n) { return std::uniform_int_distribution<int>(0, n - 1)(rng); }
    Color ink() const { return flash ? THEMES[theme].bg : THEMES[theme].ink; }
    Color bg() const { return flash ? THEMES[theme].ink : THEMES[theme].bg; }
    void redraw() { dirty = true; }
    bool spinning() const { return screen == PLAY && g.game == SCH && g.spinT && g.phase == RUN; }
    std::string mods() const { return sel == SCH ? std::string(spinN ? "N" : "") + (spinT ? "T" : "") : ""; }

    double bestAvg(const std::string& code, const std::string& dn, const std::string& m) const {
        double b = -1;
        for (auto& r : store.runs)
            if (r.game == code && r.diff == dn && r.complete && r.mods == m && (b < 0 || r.avg < b)) b = r.avg;
        return b;
    }

    // ---- timers (polled from update())
    void after(double ms, Fn fn) { cancel(); job.due = clock() + ms; job.fn = std::move(fn); job.active = true; }
    void cancel() { job.active = false; }
    bool update() {
        double n = clock();
        if (job.active && n >= job.due) { Fn f = job.fn; job.active = false; f(); }
        if (badDue > 0 && n >= badDue) { badDue = 0; bad = -1; if (screen == PLAY) redraw(); }
        if (spinning() && n - lastSpin >= 16) { lastSpin = n; dirty = true; }
        return dirty;
    }
    void armT0() { awaitPaint = true; }
    void presented() { if (awaitPaint) { g.t0 = clock(); awaitPaint = false; } }

    // ---- drawing primitives
    void T(double x, double y, const std::string& s, int size = -1, Anchor a = NW, int fl = 0) {
        Color c = (fl & INV) ? bg() : ((fl & DIM) ? mix(ink(), bg(), 0.5) : ink());
        cv->text(x, y, s, size < 0 ? fs : size, a, c, 0, 0);
    }
    void Tx(double x, double y, const std::string& s, int size, Anchor a, Color c, double ang = 0) {
        cv->text(x, y, s, size, a, c, ang, 0);
    }
    void Tw(double x, double y, const std::string& s, int wrap, int fl) {
        cv->text(x, y, s, fs, NW, (fl & DIM) ? mix(ink(), bg(), 0.5) : ink(), 0, wrap);
    }
    void R(double x1, double y1, double x2, double y2, bool fill = false, int w = 2) {
        Color k = ink();
        cv->rect(x1, y1, x2, y2, k, fill ? &k : nullptr, w);
    }
    void Ln(double x1, double y1, double x2, double y2) { cv->line(x1, y1, x2, y2, ink(), 2); }
    void B(double x1, double y1, double x2, double y2, const std::string& label, Fn cb, bool on = false,
           bool dim = false, int size = -1, bool left = false) {
        R(x1, y1, x2, y2, on);
        int fl = (on ? INV : 0) | ((dim && !on) ? DIM : 0);
        if (left) T(x1 + 16, (y1 + y2) / 2, label, size, Anchor::W, fl);
        else T((x1 + x2) / 2, (y1 + y2) / 2, label, size, Anchor::C, fl);
        if (cb) regions.push_back({x1, y1, x2, y2, std::move(cb)});
    }
    void centerMsg() { T((cx1 + cx2) / 2, (top + bot) / 2, msg, big, Anchor::C); }

    // ---- main draw
    void draw(Canvas& canvas, int w, int h) {
        cv = &canvas; W = w; H = h;
        fs = std::max(11, (int)(H / 36.0));
        big = std::max(24, (int)(H / 10.0));
        regions.clear(); polys.clear();
        cv->clear(bg());
        frame();
        switch (screen) {
            case MENU: menu(); break;
            case DATA: drawData(); break;
            case PLAY: play(); break;
            case RESULT: result(); break;
        }
        dirty = false;
    }

    std::string hint() const {
        if (screen == MENU) return "1-5 GAME  R ROUNDS  D DIFF  N/S SPIN  ENTER START  TAB DATA  F2 COLOR  F10 FULL  ESC QUIT";
        if (screen == DATA) return "TAB BACK   F2 COLOR   ESC BACK";
        if (screen == RESULT) return "ENTER AGAIN   ESC MENU";
        return std::string("ESC ABORT") + ((g.game == VRT || g.game == ART) ? "   CLICK / SPACE / ENTER = REACT" : "");
    }

    void frame() {
        double x1 = 48, x2 = W - 48, y1 = 30, y2 = H - 28;
        cx1 = x1; cx2 = x2;
        int rowh = (int)(fs * 2.3);
        if (screen == MENU || screen == DATA) {
            double x = x1;
            const char* names[2] = {"TEST", "DATA"};
            for (int i = 0; i < 2; i++) {
                double w = std::strlen(names[i]) * fs * 0.76 + 36;
                if (i == tab) R(x, y1, x + w, y1 + rowh);
                T(x + w / 2, y1 + rowh / 2, names[i], -1, Anchor::C, i != tab ? DIM : 0);
                regions.push_back({x, y1, x + w, y1 + rowh, [this, i] { setTab(i); }});
                x += w + 10;
            }
        } else {
            std::string sp;
            if (g.spinN) sp += "NUMBERS";
            if (g.spinT) sp += std::string(sp.empty() ? "" : "+") + "TABLE";
            T(x1, y1 + rowh / 2,
              F("TEST // %s // %s // %d ROUNDS", GAMES[g.game].name, DIFFS[g.di], g.rounds) +
                  (sp.empty() ? "" : " // SPIN " + sp),
              -1, Anchor::W);
        }
        double bw = std::strlen(THEMES[theme].name) * fs * 0.76 + 24;
        B(x2 - bw, y1 + 2, x2, y1 + rowh - 2, THEMES[theme].name, [this] { cycleTheme(); }, false, false, fs - 2);
        double ly = y1 + rowh + 8;
        Ln(x1, ly, x2, ly);
        top = ly + 16;

        int bh = (int)(fs * 2.3);
        double sy = y2 - bh;
        bot = sy - (int)(fs * 2.3);
        T(x1, sy - 8, hint(), std::max(9, fs - 3), SW, DIM);
        std::string left, center, right;
        double ratio;
        int runs = (int)store.runs.size();
        if (screen == MENU || screen == DATA) {
            left = F("RUNS %d", runs); center = F("LEVEL %d", 1 + runs / 5); ratio = (runs % 5) / 5.0;
            right = THEMES[theme].name;
        } else {
            left = screen == RESULT ? "DONE" : F("ROUND %d/%d", std::min(g.idx + 1, g.rounds), g.rounds);
            center = DIFFS[g.di]; ratio = (double)g.idx / g.rounds; right = F("MISS %d", g.miss);
        }
        double lw = (x2 - x1) * 0.25;
        R(x1, sy, x1 + lw, y2);
        T(x1 + lw / 2, sy + bh / 2.0, left, fs - 1, Anchor::C);
        R(x2 - lw, sy, x2, y2);
        T(x2 - lw / 2, sy + bh / 2.0, right, fs - 1, Anchor::C);
        double mx1 = x1 + lw + 10, mx2 = x2 - lw - 10;
        R(mx1, sy, mx2, y2);
        T(mx1 + 12, sy + bh / 2.0, center, fs - 1, Anchor::W);
        double bx1 = mx1 + 12 + center.size() * (fs - 1) * 0.76 + 14;
        if (bx1 < mx2 - 24) {
            R(bx1, sy + 8, mx2 - 10, y2 - 8, false, 1);
            if (ratio > 0) R(bx1, sy + 8, bx1 + (mx2 - 10 - bx1) * ratio, y2 - 8, true, 1);
        }
    }

    void menu() {
        int rh = (int)(fs * 2.5);
        double x1 = cx1, x2 = cx2, y = top, lw = (x2 - x1) * 0.44;
        for (int i = 0; i < 5; i++) {
            double t = y + i * (rh + 8);
            B(x1, t, x1 + lw, t + rh, F("%d  %s", i + 1, GAMES[i].name), [this, i] { select(i); }, i == sel, false, -1, true);
        }
        double px = x1 + lw + 26, pw = x2 - px;
        const GameInfo& gi = GAMES[sel];
        T(px, y, gi.name, fs + 4);
        int lines = (int)std::ceil(std::strlen(gi.desc) * fs * 0.76 / std::max(1.0, pw));
        Tw(px, y + fs * 2.3, gi.desc, (int)pw, DIM);
        double yy = y + fs * 2.3 + lines * fs * 1.5 + fs, bw = (pw - 16) / 3;
        T(px, yy, "ROUNDS");
        yy += fs * 1.9;
        for (int i = 0; i < 3; i++) {
            int r = ROUNDS[i];
            B(px + i * (bw + 8), yy, px + i * (bw + 8) + bw, yy + rh, F("%d", r), [this, r] { setRounds(r); }, r == rounds);
        }
        yy += rh + fs * 1.2;
        T(px, yy, "DIFFICULTY");
        yy += fs * 1.9;
        for (int i = 0; i < 3; i++)
            B(px + i * (bw + 8), yy, px + i * (bw + 8) + bw, yy + rh, DIFFS[i], [this, i] { setDiff(i); }, i == diff, false, fs - 1);
        yy += rh + fs * 0.9;
        T(px, yy, DETAIL[sel][diff], -1, NW, DIM);
        yy += fs * 1.8;
        if (sel == SCH) {
            double hw = (pw - 10) / 2;
            B(px, yy, px + hw, yy + rh, std::string("SPIN NUMBERS: ") + (spinN ? "ON" : "OFF"), [this] { toggleN(); }, spinN, false, fs - 2);
            B(px + hw + 10, yy, x2, yy + rh, std::string("SPIN TABLE: ") + (spinT ? "ON" : "OFF"), [this] { toggleT(); }, spinT, false, fs - 2);
            yy += rh + fs * 0.9;
        }
        T(px, yy, "BEST AVG: " + fmtv(sel, bestAvg(GAMES[sel].code, DIFFS[diff], mods())));
        double ys = std::max(yy + fs * 1.9, bot - rh * 1.15);
        B(px, ys, x2, ys + rh * 1.15, "[ START ]", [this] { begin(); }, false, false, fs + 3);
    }

    void drawData() {
        double x = cx1, y = top;
        int lh = (int)(fs * 1.65);
        T(x, y, "PERSONAL BEST (AVG PER ROUND)   E=EASY M=MEDIUM H=HARD", -1, NW, DIM);
        y += lh;
        for (int i = 0; i < 5; i++) {
            std::string parts;
            for (int d = 0; d < 3; d++) parts += std::string(d ? "   " : "") + F("%c ", DIFFS[d][0]) + fmtv(i, bestAvg(GAMES[i].code, DIFFS[d], ""));
            T(x, y, F("%-16s", GAMES[i].name) + parts);
            y += lh;
        }
        y += lh * 0.5;
        T(x, y, "RECENT RUNS  (* = aborted)", -1, NW, DIM);
        y += lh;
        int rows = std::max(0, (int)((bot - y - lh * 1.2) / lh));
        int n = (int)store.runs.size();
        if (n == 0) T(x, y, "NO DATA YET - PLAY A TEST");
        for (int i = 0; i < std::min(rows, n); i++) {
            const Run& r = store.runs[n - 1 - i];
            std::string md = r.mods.empty() ? "" : "  +" + r.mods;
            T(x, y, F("%s  %s %-3s %d/%d%s  avg %s  miss %d%s", r.t.substr(5).c_str(), r.game.c_str(), r.diff.substr(0, 3).c_str(),
                      r.done, r.rounds, r.complete ? "" : "*", fmtv(gameIndex(r.game), r.avg).c_str(), r.miss, md.c_str()));
            y += lh;
        }
        int rh = (int)(fs * 2.3);
        double w = fs * 0.76 * 18 + 20;
        B(cx2 - w, bot - rh + 6, cx2, bot + 6, confirm ? "CONFIRM CLEAR?" : "CLEAR DATA", [this] { clearData(); }, confirm, false, fs - 1);
    }

    void play() {
        if (g.game == VRT || g.game == ART) pVrt();
        else if (g.phase != RUN) centerMsg();
        else if (g.game == SCH) pSch();
        else if (g.game == STR) pStr();
        else pTsw();
    }

    void pVrt() {
        double x = cx1, y = top;
        auto& v = g.vals;
        if (!v.empty()) {
            double s = 0;
            for (double d : v) s += d;
            T(x, y, F("Avg: %.1f ms  Best: %.1f ms", s / v.size(), *std::min_element(v.begin(), v.end())));
        } else T(x, y, "Avg: -  Best: -");
        int lh = (int)(fs * 1.7), cnt = (int)v.size();
        int shown = std::min(std::max(1, (int)((bot - y - lh) / lh)), cnt), start = cnt - shown;
        for (int i = 0; i < shown; i++) T(x + 28, y + lh * (i + 1) + 4, F("%d. %.1f ms", start + i + 1, v[start + i]));
        centerMsg();
    }

    void pSch() {
        int n = g.n;
        T(cx1, top, F("FIND %d", g.cur), fs + 10);
        T(cx2, top + 4, F("TABLE %d/%d", g.idx + 1, g.rounds), -1, NE);
        double gy = top + fs * 3.4, avail = std::min(cx2 - cx1, bot - gy);
        double side = g.spinT ? avail / 1.42 : avail, cell = side / n;
        double mx = (cx1 + cx2) / 2, my = gy + (bot - gy) / 2;
        static const double speed[3] = {20, 32, 48};
        double thDeg = g.spinT ? std::fmod((clock() - g.spin0) / 1000.0 * speed[g.di], 360.0) : 0.0;
        double th = thDeg * 3.14159265358979 / 180.0, co = std::cos(th), si = std::sin(th);
        int size = std::max(12, (int)(cell * 0.30));
        for (int k = 0; k < n * n; k++) {
            int r = k / n, c = k % n, v = g.nums[k];
            double ox = (c + 0.5) * cell - side / 2, oy = (r + 0.5) * cell - side / 2;
            double cx = mx + ox * co - oy * si, cy = my + ox * si + oy * co, ang = g.ang[k] - thDeg;
            if (v < g.cur) { Tx(cx, cy, F("%d", v), size, Anchor::C, mix(ink(), bg(), 0.5), ang); continue; }
            double h = cell / 2 - 3;
            std::vector<Pt> pts;
            const double d[4][2] = {{-h, -h}, {h, -h}, {h, h}, {-h, h}};
            for (auto& q : d) pts.push_back({cx + q[0] * co - q[1] * si, cy + q[0] * si + q[1] * co});
            bool isBad = bad == k;
            Color k1 = ink();
            cv->poly(pts, k1, isBad ? &k1 : nullptr, 2);
            Tx(cx, cy, F("%d", v), size, Anchor::C, isBad ? bg() : ink(), ang);
            polys.push_back({pts, [this, k] { schClick(k); }});
        }
    }

    void pTsw() {
        int r = g.r, c = g.c;
        T(cx1, top, F("TARGET %d/%d", g.idx + 1, g.rounds), fs + 4);
        if (!g.vals.empty()) {
            double s = 0;
            for (double d : g.vals) s += d;
            T(cx2, top + 4, F("AVG %.0f ms", s / g.vals.size()), -1, NE);
        }
        double ty = top + fs * 3, cell = std::min((cx2 - cx1) / c, (bot - ty) / r);
        double gx = (cx1 + cx2) / 2 - cell * c / 2, gy = ty + ((bot - ty) - cell * r) / 2;
        for (int k = 0; k < r * c; k++) {
            int rr = k / c, cc = k % c;
            double cx = gx + cc * cell + cell / 2, cy = gy + rr * cell + cell / 2;
            Color k1 = ink(), dimc = mix(ink(), bg(), 0.5);
            const Color* fill = k == g.lit ? &k1 : (k == bad ? &dimc : nullptr);
            cv->oval(cx, cy, cell * 0.30, k1, fill, 3);
            regions.push_back({cx - cell / 2, cy - cell / 2, cx + cell / 2, cy + cell / 2, [this, k] { tswClick(k); }});
        }
    }

    static std::string titleCase(const char* s) {
        std::string o = s;
        for (size_t i = 1; i < o.size(); i++) o[i] = (char)std::tolower((unsigned char)o[i]);
        return o;
    }

    void pStr() {
        T(cx1, top, F("WORD %d/%d", g.idx + 1, g.rounds), fs + 4);
        if (!g.vals.empty()) {
            double s = 0;
            for (double d : g.vals) s += d;
            T(cx2, top + 4, F("AVG %.0f ms", s / g.vals.size()), -1, NE);
        }
        double ty = top + fs * 3, avail = bot - ty, wh = avail * 0.30, gap = 8;
        double bh = (avail - wh - 14 - 2 * gap) / 3, cx = (cx1 + cx2) / 2;
        double kwid = std::min(cx2 - cx1, 780.0), x0 = cx - kwid / 2;
        cv->rect(x0, ty, x0 + kwid, ty + wh, ink(), nullptr, 2);
        Tx(cx, ty + wh / 2, titleCase(COLORS[g.word].name), std::max(16, std::min((int)(big * 1.2), (int)(wh * 0.55 / 1.33))),
           Anchor::C, COLORS[g.ink].c);
        double bw = (kwid - 2 * gap) / 3, y0 = ty + wh + 14;
        for (int k = 0; k < 9; k++) {
            int r = k / 3, c = k % 3, ci = g.order[k];
            double x = x0 + c * (bw + gap), y = y0 + r * (bh + gap);
            bool isBad = bad == ci;
            Color k1 = ink();
            cv->rect(x, y, x + bw, y + bh, k1, isBad ? &k1 : nullptr, 2);
            Tx(x + bw / 2, y + bh / 2, titleCase(COLORS[ci].name), std::max(11, std::min(fs + 4, (int)(bh * 0.42 / 1.33))),
               Anchor::C, isBad ? bg() : COLORS[g.lab[k]].c);
            regions.push_back({x, y, x + bw, y + bh, [this, ci] { strClick(ci); }});
        }
    }

    void result() {
        const Run& r = g.rec;
        int gi = gameIndex(r.game), lh = (int)(fs * 1.7);
        double x1 = cx1, x2 = cx2, y = top, lw = (x2 - x1) * 0.42;
        T(x1, y, "ROUND VALUES", -1, NW, DIM);
        int rows = std::max(1, (int)((bot - y - lh) / lh)), colw = (int)(fs * 11), maxcols = std::max(1, (int)(lw / colw));
        int lim = std::min((int)r.vals.size(), rows * maxcols);
        for (int i = 0; i < lim; i++) T(x1 + (i / rows) * colw, y + lh * (i % rows + 1), F("%2d. ", i + 1) + fmtv(gi, r.vals[i]));
        double px = x1 + lw + 30;
        T(px, y, "RESULT", fs + 6);
        double yy = y + fs * 2.8;
        if (g.newbest) { T(px, yy, "** NEW PERSONAL BEST **"); yy += lh; }
        std::pair<std::string, std::string> kv[5] = {{"ROUNDS", F("%d/%d", r.done, r.rounds)},
                                                      {gi == SCH ? "AVG / TABLE" : "AVERAGE", fmtv(gi, r.avg)},
                                                      {"BEST", fmtv(gi, r.best)},
                                                      {"TOTAL", F("%.2fs", r.total / 1000)},
                                                      {"MISSES", F("%d", r.miss)}};
        for (auto& p : kv) { T(px, yy, p.first, -1, NW, DIM); T(x2, yy, p.second, -1, NE); yy += lh; }
        yy += lh * 0.4;
        T(px, yy, rating(gi, g.di, r.avg), fs + 6);
        yy += fs * 2.6;
        T(px, yy, "SAVED TO DATA", -1, NW, DIM);
        int rh = (int)(fs * 2.5);
        double by = bot - rh + 6, bw = (x2 - px - 10) / 2;
        B(px, by, px + bw, by + rh, "[ AGAIN ]", [this] { begin(); });
        B(px + bw + 10, by, x2, by + rh, "[ MENU ]", [this] { back(); });
    }

    // ---- input
    void click(int x, int y) {
        for (auto it = regions.rbegin(); it != regions.rend(); ++it)
            if (x >= it->x1 && x <= it->x2 && y >= it->y1 && y <= it->y2) { Fn cb = it->cb; cb(); return; }
        for (auto it = polys.rbegin(); it != polys.rend(); ++it)
            if (inside(x, y, it->pts)) { Fn cb = it->cb; cb(); return; }
        react();
    }
    static bool inside(double px, double py, const std::vector<Pt>& p) {
        bool ok = false;
        for (size_t i = 0, j = p.size() - 1; i < p.size(); j = i++)
            if ((p[i].y > py) != (p[j].y > py) && px < (p[j].x - p[i].x) * (py - p[i].y) / (p[j].y - p[i].y) + p[i].x) ok = !ok;
        return ok;
    }
    // k: Windows virtual-key code (letters/digits are their ASCII codes)
    void key(int k) {
        if (k == 0x1B) back();
        else if (k == 0x79) { if (onToggleFs) onToggleFs(); }
        else if (k == 0x71) cycleTheme();
        else if (k == 0x09) { if (screen == MENU || screen == DATA) setTab(1 - tab); }
        else if (screen == MENU) {
            if (k >= '1' && k <= '5') select(k - '1');
            else if (k == 'R') setRounds(ROUNDS[(std::find(ROUNDS, ROUNDS + 3, rounds) - ROUNDS + 1) % 3]);
            else if (k == 'D') setDiff((diff + 1) % 3);
            else if (k == 'N') toggleN();
            else if (k == 'S') toggleT();
            else if (k == 0x0D) begin();
        } else if (screen == PLAY && (k == 0x0D || k == 0x20)) react();
        else if (screen == RESULT && k == 0x0D) begin();
    }
    void back() {
        if (screen == MENU) { if (onQuit) onQuit(); }
        else if (screen == PLAY) {
            cancel();
            if (!g.vals.empty()) finish(false);
            else { screen = MENU; flash = false; bad = -1; redraw(); }
        } else { screen = MENU; tab = 0; confirm = false; redraw(); }
    }
    void persist() {
        store.st = {theme, sel, rounds, diff, spinN, spinT};
        store.save();
    }
    void select(int i) { sel = i; persist(); redraw(); }
    void setRounds(int r) { rounds = r; persist(); redraw(); }
    void setDiff(int i) { diff = i; persist(); redraw(); }
    void toggleN() { spinN = !spinN; persist(); redraw(); }
    void toggleT() { spinT = !spinT; persist(); redraw(); }
    void setTab(int i) { tab = i; confirm = false; screen = i == 0 ? MENU : DATA; redraw(); }
    void cycleTheme() { theme = (theme + 1) % 3; persist(); redraw(); }
    void clearData() {
        if (!confirm) confirm = true;
        else { store.runs.clear(); store.save(); confirm = false; }
        redraw();
    }

    // ---- test engine
    void begin() {
        persist(); cancel();
        g = G();
        g.game = sel; g.di = diff; g.rounds = rounds;
        if (sel == SCH) { g.spinN = spinN; g.spinT = spinT; }
        screen = PLAY; flash = false; bad = -1; awaitPaint = false;
        msg = "GET READY...";
        redraw();
        after(1100, [this] { nextStep(); });
    }
    void nextStep() {
        switch (g.game) {
            case VRT: case ART: vrtNext(); break;
            case SCH: schNew(); break;
            case STR: strNext(); break;
            default: tswNext();
        }
    }
    void finish(bool complete = true) {
        cancel(); flash = false; bad = -1; g.phase = DONE; awaitPaint = false;
        auto& v = g.vals;
        if (!v.empty()) {
            std::string code = GAMES[g.game].code, dn = DIFFS[g.di], m;
            if (g.spinN) m += "N";
            if (g.spinT) m += "T";
            double sum = 0;
            for (double d : v) sum += d;
            double avg = sum / v.size(), prev = bestAvg(code, dn, m);
            g.newbest = complete && (prev < 0 || avg < prev);
            Run r;
            char b[32];
            std::time_t t = std::time(nullptr);
            std::tm tmv{};
#ifdef _WIN32
            localtime_s(&tmv, &t);
#else
            localtime_r(&t, &tmv);
#endif
            std::strftime(b, sizeof b, "%Y-%m-%d %H:%M", &tmv);
            r.t = b; r.game = code; r.diff = dn; r.mods = m;
            r.rounds = g.rounds; r.done = (int)v.size(); r.complete = complete;
            r.avg = std::round(avg * 10) / 10; r.best = std::round(*std::min_element(v.begin(), v.end()) * 10) / 10;
            r.total = std::round(sum * 10) / 10; r.miss = g.miss;
            for (double d : v) r.vals.push_back(std::round(d * 10) / 10);
            store.runs.push_back(r);
            if (store.runs.size() > 500) store.runs.erase(store.runs.begin(), store.runs.end() - 500);
            store.save();
            g.rec = r; g.hasRec = true;
        }
        screen = (complete && !v.empty()) ? RESULT : MENU;
        redraw();
    }
    void react() {
        if (screen == PLAY && (g.game == VRT || g.game == ART)) vrtReact();
    }
    bool live() const { return g.phase == RUN && !awaitPaint; }

    void vrtNext() {
        if (g.idx >= g.rounds) return finish();
        flash = false; g.phase = WAIT; msg = "Wait for it...";
        static const double lo[3] = {1.5, 1.5, 0.8}, hi[3] = {3.0, 5.0, 6.5};
        redraw();
        after(rnd(lo[g.di], hi[g.di]) * 1000, [this] { vrtGo(); });
    }
    void vrtGo() {
        g.phase = GO;
        if (g.game == VRT) { flash = true; msg = "NOW!"; redraw(); armT0(); }
        else { msg = "Listen..."; if (onBeep) onBeep(); g.t0 = clock(); }
    }
    void vrtReact() {
        if (g.phase == GO && !awaitPaint) {
            double ms = clock() - g.t0;
            g.vals.push_back(ms); g.idx++;
            g.phase = SHOW; flash = false; msg = F("%.1f ms", ms);
            redraw();
            after(1000, [this] { vrtNext(); });
        } else if (g.phase == WAIT) {
            g.miss++; g.phase = SHOW; msg = "Too soon!";
            redraw();
            after(1300, [this] { vrtNext(); });
        }
    }
    void missFlash(int k) { g.miss++; bad = k; redraw(); badDue = clock() + 160; }

    // Schulte
    void schNew() {
        if (g.idx >= g.rounds) return finish();
        int n = 3 + g.di;
        g.nums.resize(n * n);
        for (int i = 0; i < n * n; i++) g.nums[i] = i + 1;
        std::shuffle(g.nums.begin(), g.nums.end(), rng);
        g.ang.assign(n * n, 0);
        if (g.spinN) for (auto& a : g.ang) a = rint(360);
        g.n = n; g.cur = 1; g.phase = RUN; g.spin0 = clock();
        redraw(); armT0();
    }
    void schClick(int k) {
        if (!live()) return;
        if (g.nums[k] == g.cur) {
            if (g.cur == g.n * g.n) {
                double ms = clock() - g.t0;
                g.vals.push_back(ms); g.idx++; g.phase = SHOW; msg = F("CLEARED  %.2fs", ms / 1000);
                redraw();
                after(900, [this] { schNew(); });
            } else { g.cur++; redraw(); }
        } else missFlash(k);
    }

    // Target switch
    void tswNext() {
        if (g.idx >= g.rounds) return finish();
        static const int rr[3] = {2, 3, 4}, cc[3] = {3, 3, 4};
        g.r = rr[g.di]; g.c = cc[g.di];
        int k = rint(g.r * g.c);
        while (k == g.lit) k = rint(g.r * g.c);
        g.lit = k; g.pen = 0; g.phase = RUN;
        redraw(); armT0();
    }
    void tswClick(int k) {
        if (!live()) return;
        if (k == g.lit) { g.vals.push_back(clock() - g.t0 + g.pen); g.idx++; tswNext(); }
        else { g.pen += 500; missFlash(k); }
    }

    // Color trial
    void strNext() {
        if (g.idx >= g.rounds) return finish();
        int w = rint(9), ink;
        do ink = rint(9); while (ink == w);
        if (g.order.empty() || g.di == 2) {
            g.order.resize(9);
            for (int i = 0; i < 9; i++) g.order[i] = i;
            if (g.di == 2) std::shuffle(g.order.begin(), g.order.end(), rng);
        }
        if (g.lab.empty() || g.di >= 1) {
            g.lab.assign(9, 0);
            for (int k = 0; k < 9; k++) { int j; do j = rint(9); while (j == g.order[k]); g.lab[k] = j; }
        }
        g.word = w; g.ink = ink; g.pen = 0; g.phase = RUN;
        redraw(); armT0();
    }
    void strClick(int ci) {
        if (!live()) return;
        if (ci == g.ink) { g.vals.push_back(clock() - g.t0 + g.pen); g.idx++; strNext(); }
        else { g.pen += 500; missFlash(ci); }
    }
};

}  // namespace rt
