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

inline Color hsv(double h, double s, double v) {
    h = std::fmod(h, 360.0);
    if (h < 0) h += 360.0;
    double c = v * s, x = c * (1 - std::fabs(std::fmod(h / 60.0, 2.0) - 1)), m = v - c, r = 0, g = 0, b = 0;
    switch ((int)(h / 60.0) % 6) {
        case 0: r = c; g = x; break;
        case 1: r = x; g = c; break;
        case 2: g = c; b = x; break;
        case 3: g = x; b = c; break;
        case 4: r = x; b = c; break;
        default: r = c; b = x;
    }
    return {(int)std::lround((r + m) * 255), (int)std::lround((g + m) * 255), (int)std::lround((b + m) * 255)};
}

struct Theme { const char* name; Color ink, bg; };
// index 4 (CUSTOM) is filled in from the player's color wheel settings
inline const Theme THEMES[5] = {{"GREEN", {0x3c, 0xff, 0x6e}, {0x03, 0x12, 0x08}},
                                {"AMBER", {0xff, 0xb0, 0x2e}, {0x14, 0x0b, 0x00}},
                                {"RED", {0xff, 0x3a, 0x30}, {0x10, 0x00, 0x00}},
                                {"B&W", {0xf2, 0xf2, 0xf2}, {0x00, 0x00, 0x00}},
                                {"CUSTOM", {0xff, 0xff, 0xff}, {0x00, 0x00, 0x00}}};

enum GameId { VRT, ART, SCH, TSW, STR };
struct GameInfo { const char* code; const char* name; const char* desc; };
inline const GameInfo GAMES[5] = {
    {"VRT", "VISUAL REACTION", "Click the instant the screen flashes."},
    {"ART", "AUDIO REACTION", "Click the instant you hear the beep."},
    {"SCH", "SCHULTE TABLE", "Click the numbers in order, 1 up to the last, as fast as you can."},
    {"TSW", "TARGET SWITCH", "Click the lit dot. A new one lights up after each hit. A miss costs 500 ms."},
    {"STR", "COLOR TRIAL",
     "A color word is printed in a different color. Press the button named after the color it is PRINTED in. "
     "Example: GREEN printed in red means press Red. A draining timer runs on every word. A miss costs 500 ms."}};
inline const char* DIFFS[3] = {"EASY", "MEDIUM", "HARD"};
inline const int ROUNDS[3] = {5, 10, 20};
inline const char* DETAIL[5][3] = {
    {"Wait 1.5-3 s", "Wait 1.5-5 s", "Wait 0.8-6.5 s"},
    {"Wait 1.5-3 s", "Wait 1.5-5 s", "Wait 0.8-6.5 s"},
    {"Table spins at 20 deg/s", "Table spins at 32 deg/s", "Table spins at 48 deg/s"},
    {"2x3 grid (6 dots)", "3x3 grid (9 dots)", "4x4 grid (16 dots)"},
    {"5 s per word, labels fixed", "3.5 s per word, label colors change", "2.5 s per word, colors change + buttons shuffle"}};
struct ColorDef { const char* name; Color c; };
inline const ColorDef COLORS[9] = {{"PINK", {0xff, 0x7a, 0xd9}},   {"RED", {0xff, 0x1f, 0x1f}},
                                   {"PURPLE", {0x9b, 0x30, 0xff}}, {"YELLOW", {0xff, 0xe1, 0x3a}},
                                   {"WHITE", {0xff, 0xff, 0xff}},  {"BLACK", {0x5f, 0x5f, 0x5f}},
                                   {"GREEN", {0x22, 0xe0, 0x4a}},  {"BLUE", {0x2a, 0x4d, 0xff}},
                                   {"ORANGE", {0xff, 0x9a, 0x1a}}};

// ---------------------------------------------------------------- ranks
inline const char* RANK_NAMES[12] = {"IRON", "BRONZE", "SILVER", "GOLD", "PLATINUM", "DIAMOND",
                                     "JADE", "MASTER", "GRANDMASTER", "NOVA", "ASTRA", "CELESTIAL"};
inline const Color RANK_COL[12] = {{0x9a, 0x9a, 0x9a}, {0xcd, 0x7f, 0x32}, {0xc8, 0xd0, 0xd8}, {0xff, 0xd7, 0x00},
                                   {0x5f, 0xe3, 0xd0}, {0x6e, 0xc6, 0xff}, {0x2e, 0xcc, 0x71}, {0xb8, 0x6b, 0xff},
                                   {0xff, 0x5a, 0x5a}, {0xff, 0x9f, 0x43}, {0xff, 0x6b, 0xd6}, {0x7d, 0xf9, 0xff}};
// A score below RANKT[game][i] earns rank 11-i (CELESTIAL first). Anything slower is IRON.
// Units: ms. Schulte is ms per number. The visual reaction ladder (row 0) is the reference scale.
inline const double RANKT[5][11] = {{170, 185, 200, 215, 230, 245, 260, 280, 305, 340, 400},
                                    {140, 150, 160, 172, 185, 198, 212, 230, 255, 290, 350},
                                    {300, 340, 380, 430, 490, 560, 640, 740, 860, 1050, 1400},
                                    {260, 290, 320, 355, 395, 440, 490, 550, 620, 720, 900},
                                    {600, 680, 760, 850, 950, 1070, 1200, 1370, 1600, 1950, 2600}};
inline const double DIFF_FACTOR[3] = {1.0, 0.93, 0.86};  // harder difficulty counts a little better
inline int rankFromScore(int game, double v) {
    for (int i = 0; i < 11; i++) if (v < RANKT[game][i]) return 11 - i;
    return 0;
}
inline std::string vkName(int k) {
    if ((k >= '0' && k <= '9') || (k >= 'A' && k <= 'Z')) return std::string(1, (char)k);
    if (k >= 0x70 && k <= 0x7B) return F("F%d", k - 0x6F);
    switch (k) {
        case 0x20: return "SPACE"; case 0x09: return "TAB"; case 0x0D: return "ENTER"; case 0x1B: return "ESC";
        case 0x08: return "BACKSPACE"; case 0x2E: return "DELETE"; case 0x2D: return "INSERT";
        case 0x24: return "HOME"; case 0x23: return "END"; case 0x21: return "PAGE UP"; case 0x22: return "PAGE DOWN";
    }
    return F("KEY %d", k);
}

inline int gameIndex(const std::string& code) {
    for (int i = 0; i < 5; i++) if (code == GAMES[i].code) return i;
    return 0;
}
inline std::string fmtv(int game, double ms) {
    if (ms < 0) return "--";
    return game == SCH ? F("%.2fs", ms / 1000) : F("%.1fms", ms);
}
inline const char* rating(int game, double v) {
    static const char* names[5] = {"EXCEPTIONAL", "EXCELLENT", "GOOD", "FAIR", "KEEP PRACTICING"};
    static const double t[5][4] = {{200, 250, 300, 400}, {160, 200, 250, 350}, {450, 650, 900, 1300},
                                   {300, 400, 550, 700}, {700, 900, 1150, 1500}};
    for (int i = 0; i < 4; i++) if (v < t[game][i]) return names[i];
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
struct Settings {
    int theme = 0, game = 0, rounds = 10, diff = 1, schSize = 5;
    bool spinN = false, spinT = false, strAudio = true;
    double ih = 130, is = 0.8, iv = 1.0, bh = 130, bs = 0.9, bv = 0.07;  // custom theme (HSV)
    int kRestart = 0x20, kTab = 0x09, kTheme = 0x71, kFull = 0x79;       // rebindable keys
};
inline int diffIndex(const std::string& d) {
    for (int i = 0; i < 3; i++) if (d == DIFFS[i]) return i;
    return 1;
}

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
                st.theme = std::clamp(std::atoi(p[1].c_str()), 0, 4);
                st.game = std::clamp(std::atoi(p[2].c_str()), 0, 4);
                int r = std::atoi(p[3].c_str());
                st.rounds = (r == 5 || r == 10 || r == 20) ? r : 10;
                st.diff = std::clamp(std::atoi(p[4].c_str()), 0, 2);
                st.spinN = p[5] == "1";
                st.spinT = p[6] == "1";
                if (p.size() >= 19) {
                    st.schSize = std::clamp(std::atoi(p[7].c_str()), 3, 6);
                    st.strAudio = p[8] == "1";
                    st.ih = std::atof(p[9].c_str()); st.is = std::atof(p[10].c_str()); st.iv = std::atof(p[11].c_str());
                    st.bh = std::atof(p[12].c_str()); st.bs = std::atof(p[13].c_str()); st.bv = std::atof(p[14].c_str());
                    st.kRestart = std::clamp(std::atoi(p[15].c_str()), 1, 255);
                    st.kTab = std::clamp(std::atoi(p[16].c_str()), 1, 255);
                    st.kTheme = std::clamp(std::atoi(p[17].c_str()), 1, 255);
                    st.kFull = std::clamp(std::atoi(p[18].c_str()), 1, 255);
                }
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
          << (st.spinN ? 1 : 0) << '\t' << (st.spinT ? 1 : 0) << '\t' << st.schSize << '\t' << (st.strAudio ? 1 : 0)
          << '\t' << F("%.1f", st.ih) << '\t' << F("%.3f", st.is) << '\t' << F("%.3f", st.iv) << '\t' << F("%.1f", st.bh)
          << '\t' << F("%.3f", st.bs) << '\t' << F("%.3f", st.bv) << '\t' << st.kRestart << '\t' << st.kTab << '\t'
          << st.kTheme << '\t' << st.kFull << '\n';
        for (auto& r : runs) {
            f << "R\t" << r.t << '\t' << r.game << '\t' << r.diff << '\t' << r.rounds << '\t' << r.done << '\t'
              << (r.complete ? 1 : 0) << '\t' << F("%.1f", r.avg) << '\t' << F("%.1f", r.best) << '\t'
              << F("%.1f", r.total) << '\t' << r.miss << '\t' << r.mods << '\t';
            for (size_t i = 0; i < r.vals.size(); i++) f << (i ? "," : "") << F("%.1f", r.vals[i]);
            f << '\n';
        }
    }
};

inline double normAvg(const Run& r) {
    if (gameIndex(r.game) == SCH) {
        int n = (!r.mods.empty() && std::isdigit((unsigned char)r.mods[0])) ? r.mods[0] - '0' : 5;
        return r.avg / (n * n);
    }
    return r.avg;
}
inline double runScore(const Run& r) { return normAvg(r) * DIFF_FACTOR[diffIndex(r.diff)]; }

// ---------------------------------------------------------------- app
using Fn = std::function<void()>;

struct Trainer {
    enum Screen { MENU, DATA, RANK, KEYS, THEME, PLAY, RESULT };  // first five are the nav-bar tabs
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
    std::function<void(const std::string&)> onSpeak;

    Store store;
    int theme = 0, sel = 0, rounds = 10, diff = 1, schSize = 5;
    bool spinN = false, spinT = false, strAudio = true;
    double ih = 130, is = 0.8, iv = 1.0, bh = 130, bs = 0.9, bv = 0.07;
    int kRestart = 0x20, kTab = 0x09, kTheme = 0x71, kFull = 0x79;
    int rebind = -1, mx = 0, my = 0;
    bool editBg = false;
    std::string keyMsg;
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
        const Settings& z = store.st;
        theme = z.theme; sel = z.game; rounds = z.rounds; diff = z.diff; schSize = z.schSize;
        spinN = z.spinN; spinT = z.spinT; strAudio = z.strAudio;
        ih = z.ih; is = z.is; iv = z.iv; bh = z.bh; bs = z.bs; bv = z.bv;
        kRestart = z.kRestart; kTab = z.kTab; kTheme = z.kTheme; kFull = z.kFull;
        rng.seed((unsigned)std::time(nullptr) ^ (unsigned)(clock() * 1000.0));
    }

    // ---- helpers
    double rnd(double lo, double hi) { return std::uniform_real_distribution<double>(lo, hi)(rng); }
    int rint(int n) { return std::uniform_int_distribution<int>(0, n - 1)(rng); }
    Color themeInk(int t) const { return t == 4 ? hsv(ih, is, iv) : THEMES[t].ink; }
    Color themeBg(int t) const { return t == 4 ? hsv(bh, bs, bv) : THEMES[t].bg; }
    const char* themeName() const { return THEMES[theme].name; }
    Color ink() const { return flash ? themeBg(theme) : themeInk(theme); }
    Color bg() const { return flash ? themeInk(theme) : themeBg(theme); }
    void redraw() { dirty = true; }
    bool isTab() const { return screen <= THEME; }
    bool animating() const {
        return screen == PLAY && g.phase == RUN && ((g.game == SCH && g.spinT) || g.game == STR);
    }
    std::string mods() const {
        return sel == SCH ? F("%d", schSize) + (spinN ? "N" : "") + (spinT ? "T" : "") : std::string();
    }
    double strLimit() const { static const double lim[3] = {5000, 3500, 2500}; return lim[g.di]; }

    // ---- ranks
    double bestScore(int game) const {
        double b = -1;
        for (auto& r : store.runs) {
            if (gameIndex(r.game) != game || !r.complete) continue;
            double v = runScore(r);
            if (b < 0 || v < b) b = v;
        }
        return b;
    }
    int gameRank(int game) const { double b = bestScore(game); return b < 0 ? -1 : rankFromScore(game, b); }
    bool overall(int& idx, double& frac) const {
        double sum = 0; int c = 0;
        for (int i = 0; i < 5; i++) { int r = gameRank(i); if (r >= 0) { sum += r; c++; } }
        if (!c) return false;
        double m = sum / c;
        idx = (int)std::floor(m + 1e-9); frac = m - idx;
        return true;
    }
    double bestNorm(int game, const std::string& dn) const {
        double b = -1;
        for (auto& r : store.runs)
            if (gameIndex(r.game) == game && r.diff == dn && r.complete && (b < 0 || normAvg(r) < b)) b = normAvg(r);
        return b;
    }

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
        if (screen == PLAY && g.game == STR && g.phase == RUN && !awaitPaint && n - g.t0 >= strLimit()) strTimeout();
        if (animating() && n - lastSpin >= 16) { lastSpin = n; dirty = true; }
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
            case RANK: drawRank(); break;
            case KEYS: drawKeys(); break;
            case THEME: drawTheme(); break;
            case PLAY: play(); break;
            case RESULT: result(); break;
        }
        dirty = false;
    }

    std::string hint() const {
        std::string tk = vkName(kTab), rk = vkName(kRestart);
        switch (screen) {
            case MENU: return "1-5 GAME  R ROUNDS  D DIFF  N/S SPIN  A AUDIO  ENTER START  " + tk + " NEXT TAB  ESC QUIT";
            case DATA: case RANK: return tk + " NEXT TAB   F2 COLOR   ESC BACK";
            case KEYS: return "CLICK A KEY ROW, THEN PRESS THE NEW KEY   ESC CANCEL / BACK";
            case THEME: return "CLICK THE WHEEL AND THE BAR TO MAKE YOUR OWN COLORS   ESC BACK";
            case RESULT: return "ENTER / " + rk + " AGAIN   ESC MENU";
            default: break;
        }
        return "ESC ABORT   " + rk + " RESTART" + ((g.game == VRT || g.game == ART) ? "   CLICK / ENTER = REACT" : "");
    }

    void frame() {
        double x1 = 48, x2 = W - 48, y1 = 30, y2 = H - 28;
        cx1 = x1; cx2 = x2;
        int rowh = (int)(fs * 2.3);
        if (isTab()) {
            double x = x1;
            static const char* names[5] = {"TEST", "DATA", "RANK", "KEYS", "THEME"};
            for (int i = 0; i < 5; i++) {
                double w = std::strlen(names[i]) * fs * 0.76 + 30;
                if (i == tab) R(x, y1, x + w, y1 + rowh);
                T(x + w / 2, y1 + rowh / 2, names[i], -1, Anchor::C, i != tab ? DIM : 0);
                regions.push_back({x, y1, x + w, y1 + rowh, [this, i] { setTab(i); }});
                x += w + 8;
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
        double bw = std::strlen(themeName()) * fs * 0.76 + 24;
        B(x2 - bw, y1 + 2, x2, y1 + rowh - 2, themeName(), isTab() ? Fn([this] { setTab(4); }) : Fn(), false, false, fs - 2);
        double ly = y1 + rowh + 8;
        Ln(x1, ly, x2, ly);
        top = ly + 16;

        int bh_ = (int)(fs * 2.3);
        double sy = y2 - bh_;
        bot = sy - (int)(fs * 2.3);
        T(x1, sy - 8, hint(), std::max(9, fs - 3), SW, DIM);
        std::string left, center, right;
        double ratio;
        int runs = (int)store.runs.size();
        if (isTab()) {
            left = F("RUNS %d", runs); center = F("LEVEL %d", 1 + runs / 5); ratio = (runs % 5) / 5.0;
            right = themeName();
        } else {
            left = screen == RESULT ? "DONE" : F("ROUND %d/%d", std::min(g.idx + 1, g.rounds), g.rounds);
            center = DIFFS[g.di]; ratio = (double)g.idx / g.rounds; right = F("MISS %d", g.miss);
        }
        double lw = (x2 - x1) * 0.25;
        R(x1, sy, x1 + lw, y2);
        T(x1 + lw / 2, sy + bh_ / 2.0, left, fs - 1, Anchor::C);
        R(x2 - lw, sy, x2, y2);
        T(x2 - lw / 2, sy + bh_ / 2.0, right, fs - 1, Anchor::C);
        double mx1 = x1 + lw + 10, mx2 = x2 - lw - 10;
        R(mx1, sy, mx2, y2);
        T(mx1 + 12, sy + bh_ / 2.0, center, fs - 1, Anchor::W);
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
        double yy = y + fs * 2.3 + lines * fs * 1.5 + fs * 0.8;
        double rh2 = (int)(fs * 2.1), labw = fs * 0.76 * 11;
        auto row = [&](const char* label, int n, const std::function<std::string(int)>& nm,
                       const std::function<bool(int)>& on, const std::function<void(int)>& act) {
            T(px, yy + rh2 / 2, label, -1, Anchor::W, DIM);
            double bx = px + labw, bw = (x2 - bx - (n - 1) * 8) / n;
            for (int i = 0; i < n; i++)
                B(bx + i * (bw + 8), yy, bx + i * (bw + 8) + bw, yy + rh2, nm(i), [act, i] { act(i); }, on(i), false, fs - 1);
            yy += rh2 + 8;
        };
        row("ROUNDS", 3, [](int i) { return F("%d", ROUNDS[i]); }, [&](int i) { return ROUNDS[i] == rounds; },
            [this](int i) { setRounds(ROUNDS[i]); });
        row("DIFFICULTY", 3, [](int i) { return std::string(DIFFS[i]); }, [&](int i) { return i == diff; },
            [this](int i) { setDiff(i); });
        auto toggles = [&](const std::string& a, bool aOn, Fn aCb, const std::string& b, bool bOn, Fn bCb) {
            double hw = (pw - 10) / 2;
            B(px, yy, px + hw, yy + rh2, a, aCb, aOn, false, fs - 2);
            if (bCb) B(px + hw + 10, yy, x2, yy + rh2, b, bCb, bOn, false, fs - 2);
            yy += rh2 + 8;
        };
        if (sel == SCH) {
            row("SIZE", 4, [](int i) { return F("%dx%d", i + 3, i + 3); }, [&](int i) { return i + 3 == schSize; },
                [this](int i) { setSize(i + 3); });
            toggles(std::string("SPIN NUMBERS: ") + (spinN ? "ON" : "OFF"), spinN, [this] { toggleN(); },
                    std::string("SPIN TABLE: ") + (spinT ? "ON" : "OFF"), spinT, [this] { toggleT(); });
        } else if (sel == STR) {
            toggles(std::string("WORD AUDIO: ") + (strAudio ? "ON" : "OFF"), strAudio, [this] { toggleAudio(); }, "", false, Fn());
        }
        T(px, yy, DETAIL[sel][diff], -1, NW, DIM);
        yy += fs * 1.7;
        T(px, yy, "BEST AVG: " + fmtv(sel, bestAvg(GAMES[sel].code, DIFFS[diff], mods())));
        double ys = std::max(yy + fs * 1.9, bot - rh * 1.15);
        B(px, ys, x2, ys + rh * 1.15, "[ START ]", [this] { begin(); }, false, false, fs + 3);
    }

    void drawData() {
        double x = cx1, y = top;
        int lh = (int)(fs * 1.65);
        T(x, y, "PERSONAL BEST (AVG PER ROUND, SCHULTE PER NUMBER)   E=EASY M=MEDIUM H=HARD", -1, NW, DIM);
        y += lh;
        for (int i = 0; i < 5; i++) {
            std::string parts;
            for (int d = 0; d < 3; d++) parts += std::string(d ? "   " : "") + F("%c ", DIFFS[d][0]) + fmtv(i, bestNorm(i, DIFFS[d]));
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
        double bh0 = fs * 1.3;
        double frac = awaitPaint ? 1.0 : std::max(0.0, 1.0 - (clock() - g.t0) / strLimit());
        Color ik = ink(), warn = {0xff, 0x3c, 0x30};
        cv->rect(cx1, top, cx2, top + bh0, ik, nullptr, 2);
        if (frac > 0) {
            const Color& fc = frac < 0.25 ? warn : ik;
            cv->rect(cx1, top, cx1 + (cx2 - cx1) * frac, top + bh0, fc, &fc, 1);
        }
        T((cx1 + cx2) / 2, top + bh0 / 2, F("%d%%", (int)std::lround(frac * 100)), fs - 2, Anchor::C, frac > 0.5 ? INV : 0);
        double hy = top + bh0 + 10;
        T(cx1, hy, F("WORD %d/%d", g.idx + 1, g.rounds), fs + 4);
        if (!g.vals.empty()) {
            double s = 0;
            for (double d : g.vals) s += d;
            T(cx2, hy + 4, F("AVG %.0f ms", s / g.vals.size()), -1, NE);
        }
        double ty = hy + fs * 3, avail = bot - ty, wh = avail * 0.30, gap = 8;
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

    void drawRank() {
        double x1 = cx1, x2 = cx2, y = top;
        int lh = (int)(fs * 1.6);
        double lw = (x2 - x1) * 0.56, cw = fs * 0.76;
        int oi = 0;
        double of = 0;
        bool has = overall(oi, of);
        T(x1, y, "CURRENT RANK", -1, NW, DIM);
        y += lh;
        int bigSz = std::max(20, (int)(big * 0.8));
        Tx(x1, y, has ? RANK_NAMES[oi] : "UNRANKED", bigSz, NW, has ? RANK_COL[oi] : mix(ink(), bg(), 0.5));
        y += (int)(bigSz * 1.7);
        if (has && oi < 11) {
            R(x1, y, x1 + lw * 0.8, y + fs * 1.2, false, 2);
            if (of > 0.01) R(x1, y, x1 + lw * 0.8 * of, y + fs * 1.2, true, 1);
            T(x1 + lw * 0.8 + 12, y + fs * 0.6, std::string("NEXT: ") + RANK_NAMES[oi + 1], fs - 1, Anchor::W, DIM);
        } else if (has) T(x1, y, "MAX RANK REACHED", -1, NW, DIM);
        else T(x1, y, "FINISH A RUN IN ANY TEST TO GET A RANK", -1, NW, DIM);
        y += lh * 1.6;
        T(x1, y, "BY TEST (BEST SCORE)", -1, NW, DIM);
        y += lh;
        for (int i = 0; i < 5; i++) {
            int r = gameRank(i);
            T(x1, y, GAMES[i].name);
            if (r >= 0) {
                Tx(x1 + 17 * cw, y, RANK_NAMES[r], fs, NW, RANK_COL[r]);
                T(x1 + 30 * cw, y, F("%.0f ms%s", bestScore(i), i == SCH ? "/no" : ""));
            } else T(x1 + 17 * cw, y, "UNRANKED", -1, NW, DIM);
            y += lh;
        }
        T(x1, y + 4, "SCORES: EASY x1.00  MEDIUM x0.93  HARD x0.86", fs - 3, NW, DIM);

        double px = x1 + lw + 24, ly = top;
        T(px, ly, "LADDER (VISUAL REACTION AVG)", fs - 1, NW, DIM);
        ly += lh;
        double rowh = std::min((double)lh, (bot - ly) / 12.0);
        for (int i = 11; i >= 0; i--) {
            bool me = has && i == oi;
            if (me) R(px - 6, ly - 2, x2, ly + rowh - 2, false, 2);
            Tx(px, ly, RANK_NAMES[i], fs - 1, NW, RANK_COL[i]);
            T(x2 - 8, ly, i > 0 ? F("< %.0f ms", RANKT[VRT][11 - i]) : F("> %.0f ms", RANKT[VRT][10]), fs - 1, NE, DIM);
            ly += rowh;
        }
    }

    void drawKeys() {
        double x1 = cx1, x2 = cx2, y = top;
        int rh = (int)(fs * 2.0);
        double cw = fs * 0.76, colw = (x2 - x1) * 0.5, kw = cw * 14;
        T(x1, y, "REBINDABLE (CLICK, THEN PRESS A KEY)", -1, NW, DIM);
        T(x1 + colw, y, "FIXED", -1, NW, DIM);
        y += fs * 1.9;
        static const char* names[4] = {"RESTART RUN", "NEXT TAB", "COLOR THEME", "FULLSCREEN"};
        int* slots[4] = {&kRestart, &kTab, &kTheme, &kFull};
        for (int i = 0; i < 4; i++) {
            double yy = y + i * (rh + 8);
            T(x1, yy + rh / 2.0, names[i], -1, Anchor::W);
            B(x1 + cw * 15, yy, x1 + cw * 15 + kw, yy + rh, rebind == i ? "PRESS A KEY" : vkName(*slots[i]),
              [this, i] { rebind = i; keyMsg = "PRESS THE NEW KEY (ESC CANCELS)"; redraw(); }, rebind == i, false, fs - 1);
        }
        T(x1, y + 4 * (rh + 8) + 4, keyMsg, fs - 1, NW, DIM);
        static const char* fl[9][2] = {{"START / REACT", "ENTER"}, {"BACK / ABORT / QUIT", "ESC"}, {"PICK TEST", "1-5"},
                                       {"ROUNDS", "R"}, {"DIFFICULTY", "D"}, {"SPIN NUMBERS", "N"},
                                       {"SPIN TABLE", "S"}, {"WORD AUDIO", "A"}, {"CLICK TARGETS", "MOUSE"}};
        double yy = y;
        double step = std::min((double)(rh + 8), (bot - y) / 9.0);
        for (auto& f : fl) {
            T(x1 + colw, yy, f[0], fs - 1, NW, DIM);
            T(x2, yy, f[1], fs - 1, NE);
            yy += step;
        }
    }

    void drawTheme() {
        int rh = (int)(fs * 2.1);
        double x1 = cx1, x2 = cx2, y = top, lw = (x2 - x1) * 0.24;
        T(x1, y, "PRESETS", -1, NW, DIM);
        y += fs * 1.9;
        for (int i = 0; i < 5; i++) {
            double t = y + i * (rh + 8);
            B(x1, t, x1 + lw, t + rh, THEMES[i].name, [this, i] { setTheme(i); }, theme == i);
        }
        double px = x1 + lw + 30, pw = x2 - px, ty = top;
        T(px, ty, "MAKE YOUR OWN THEME", -1, NW, DIM);
        ty += fs * 1.9;
        double hw = (pw - 10) / 2;
        B(px, ty, px + hw, ty + rh, "EDIT INK COLOR", [this] { editBg = false; redraw(); }, !editBg, false, fs - 1);
        B(px + hw + 10, ty, x2, ty + rh, "EDIT BACKGROUND", [this] { editBg = true; redraw(); }, editBg, false, fs - 1);
        ty += rh + 12;
        double barH = fs * 1.8, D = std::max(70.0, std::min(pw * 0.52, bot - ty - barH - 14));
        double R0 = D / 2, wx = px + R0, wy = ty + R0;
        double h = editBg ? bh : ih, s = editBg ? bs : is, v = editBg ? bv : iv;
        const double PI = 3.14159265358979;
        for (int ring = 0; ring < 6; ring++)
            for (int a = 0; a < 60; a++) {
                double a0 = a * 6 * PI / 180, a1 = (a + 1) * 6 * PI / 180, r0 = R0 * ring / 6, r1 = R0 * (ring + 1) / 6;
                std::vector<Pt> q = {{wx + r0 * std::cos(a0), wy - r0 * std::sin(a0)}, {wx + r1 * std::cos(a0), wy - r1 * std::sin(a0)},
                                     {wx + r1 * std::cos(a1), wy - r1 * std::sin(a1)}, {wx + r0 * std::cos(a1), wy - r0 * std::sin(a1)}};
                Color c = hsv(a * 6 + 3, (ring + 0.5) / 6, 1.0);
                cv->poly(q, c, &c, 1);
            }
        Color white = {255, 255, 255};
        cv->oval(wx + s * R0 * std::cos(h * PI / 180), wy - s * R0 * std::sin(h * PI / 180), 7, white, nullptr, 2);
        regions.push_back({px, ty, px + D, ty + D, [this, wx, wy, R0] { wheelClick(wx, wy, R0); }});
        double by = ty + D + 12;
        for (int i = 0; i < 24; i++) {
            Color c = hsv(h, s, (i + 0.5) / 24);
            cv->rect(px + D * i / 24, by, px + D * (i + 1) / 24 + 1, by + barH, c, &c, 1);
        }
        cv->line(px + v * D, by - 3, px + v * D, by + barH + 3, white, 3);
        regions.push_back({px, by, px + D, by + barH, [this, px, D] { barClick(px, D); }});
        T(px + D + 12, by + barH / 2, "BRIGHTNESS", fs - 2, Anchor::W, DIM);

        double vx = px + D + 24;
        if (x2 - vx > 120) {
            Color ci = hsv(ih, is, iv), cb = hsv(bh, bs, bv);
            double ph = D * 0.5;
            cv->rect(vx, ty, x2, ty + ph, ci, &cb, 2);
            Tx((vx + x2) / 2, ty + ph * 0.38, "PREVIEW", fs + 4, Anchor::C, ci);
            Tx((vx + x2) / 2, ty + ph * 0.72, "245.3 ms", fs, Anchor::C, ci);
            T(vx, ty + ph + 10, F("INK  #%02X%02X%02X", ci.r, ci.g, ci.b), fs - 1);
            T(vx, ty + ph + 10 + fs * 1.6, F("BACK #%02X%02X%02X", cb.r, cb.g, cb.b), fs - 1);
            B(vx, ty + ph + 10 + fs * 3.4, x2, ty + ph + 10 + fs * 3.4 + rh, "RESET CUSTOM", [this] { resetCustom(); }, false, false, fs - 1);
        }
    }

    void wheelClick(double wx, double wy, double R0) {
        double dx = mx - wx, dy = wy - my, r = std::sqrt(dx * dx + dy * dy);
        if (r > R0) return;
        double h = std::atan2(dy, dx) * 180.0 / 3.14159265358979;
        if (h < 0) h += 360;
        if (editBg) { bh = h; bs = r / R0; } else { ih = h; is = r / R0; }
        theme = 4; persist(); redraw();
    }
    void barClick(double bx, double D) {
        double v = std::clamp((mx - bx) / D, 0.0, 1.0);
        if (editBg) bv = v; else iv = v;
        theme = 4; persist(); redraw();
    }
    void resetCustom() { ih = 130; is = 0.8; iv = 1.0; bh = 130; bs = 0.9; bv = 0.07; theme = 4; persist(); redraw(); }
    void setTheme(int i) { theme = i; persist(); redraw(); }

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
        T(px, yy, rating(gi, normAvg(r)), fs + 6);
        yy += fs * 2.6;
        T(px, yy, "SAVED TO DATA", -1, NW, DIM);
        int rh = (int)(fs * 2.5);
        double by = bot - rh + 6, bw = (x2 - px - 10) / 2;
        B(px, by, px + bw, by + rh, "[ AGAIN ]", [this] { begin(); });
        B(px + bw + 10, by, x2, by + rh, "[ MENU ]", [this] { back(); });
    }

    // ---- input
    void click(int x, int y) {
        mx = x; my = y;
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
    static bool reserved(int k) {
        return k == 0x0D || k == 0x1B || (k >= '1' && k <= '5') || k == 'R' || k == 'D' || k == 'N' || k == 'S' || k == 'A';
    }
    // k: Windows virtual-key code (letters/digits are their ASCII codes)
    void key(int k) {
        if (rebind >= 0) {
            int* slots[4] = {&kRestart, &kTab, &kTheme, &kFull};
            if (k == 0x1B) { rebind = -1; keyMsg = "CANCELLED"; }
            else if (reserved(k)) keyMsg = vkName(k) + " IS RESERVED, TRY ANOTHER KEY";
            else {
                for (int i = 0; i < 4; i++) if (i != rebind && *slots[i] == k) *slots[i] = *slots[rebind];  // swap
                *slots[rebind] = k;
                keyMsg = "SAVED";
                rebind = -1;
                persist();
            }
            redraw();
            return;
        }
        if (k == 0x1B) back();
        else if (k == kFull) { if (onToggleFs) onToggleFs(); }
        else if (k == kTheme) cycleTheme();
        else if (k == kTab) { if (isTab()) setTab((tab + 1) % 5); }
        else if (k == kRestart) { if (screen == PLAY || screen == RESULT || screen == MENU) begin(); }
        else if (screen == MENU) {
            if (k >= '1' && k <= '5') select(k - '1');
            else if (k == 'R') setRounds(ROUNDS[(std::find(ROUNDS, ROUNDS + 3, rounds) - ROUNDS + 1) % 3]);
            else if (k == 'D') setDiff((diff + 1) % 3);
            else if (k == 'N') toggleN();
            else if (k == 'S') toggleT();
            else if (k == 'A') toggleAudio();
            else if (k == 0x0D) begin();
        } else if (screen == PLAY && k == 0x0D) react();
        else if (screen == RESULT && k == 0x0D) begin();
    }
    void back() {
        if (screen == MENU) { if (onQuit) onQuit(); }
        else if (screen == PLAY) {
            cancel();
            if (!g.vals.empty()) finish(false);
            else { screen = MENU; flash = false; bad = -1; redraw(); }
        } else { screen = MENU; tab = 0; confirm = false; rebind = -1; redraw(); }
    }
    void persist() {
        Settings z;
        z.theme = theme; z.game = sel; z.rounds = rounds; z.diff = diff; z.schSize = schSize;
        z.spinN = spinN; z.spinT = spinT; z.strAudio = strAudio;
        z.ih = ih; z.is = is; z.iv = iv; z.bh = bh; z.bs = bs; z.bv = bv;
        z.kRestart = kRestart; z.kTab = kTab; z.kTheme = kTheme; z.kFull = kFull;
        store.st = z;
        store.save();
    }
    void select(int i) { sel = i; persist(); redraw(); }
    void setRounds(int r) { rounds = r; persist(); redraw(); }
    void setDiff(int i) { diff = i; persist(); redraw(); }
    void setSize(int n) { schSize = n; persist(); redraw(); }
    void toggleN() { spinN = !spinN; persist(); redraw(); }
    void toggleT() { spinT = !spinT; persist(); redraw(); }
    void toggleAudio() { strAudio = !strAudio; persist(); redraw(); }
    void setTab(int i) {
        static const Screen TABS[5] = {MENU, DATA, RANK, KEYS, THEME};
        tab = i; confirm = false; rebind = -1; screen = TABS[i]; redraw();
    }
    void cycleTheme() { theme = (theme + 1) % 5; persist(); redraw(); }
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
        if (sel == SCH) { g.spinN = spinN; g.spinT = spinT; g.n = schSize; }
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
            if (g.game == SCH) m = F("%d", g.n);
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
        int n = g.n;
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
        if (strAudio && onSpeak) onSpeak(titleCase(COLORS[w].name));  // says the WRITTEN word, not the color
    }
    void strTimeout() {
        g.miss++;
        g.vals.push_back(strLimit() + g.pen);
        g.idx++;
        strNext();
    }
    void strClick(int ci) {
        if (!live()) return;
        if (ci == g.ink) { g.vals.push_back(clock() - g.t0 + g.pen); g.idx++; strNext(); }
        else { g.pen += 500; missFlash(ci); }
    }
};

}  // namespace rt
