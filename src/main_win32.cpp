// Reaction Trainer - Win32 / GDI front end. No third-party dependencies.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cmath>
#include <map>
#include <string>

#include "trainer.hpp"

static const double PI = 3.14159265358979;
static HWND gWnd = nullptr;
static rt::Trainer* gApp = nullptr;
static HDC gMem = nullptr;
static HBITMAP gBmp = nullptr, gOldBmp = nullptr;
static int gBufW = 0, gBufH = 0;

static double nowMs() {
    static LARGE_INTEGER freq = {};
    if (!freq.QuadPart) QueryPerformanceFrequency(&freq);
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)freq.QuadPart;
}

static COLORREF cr(rt::Color c) { return RGB(c.r, c.g, c.b); }

struct Gdi : rt::Canvas {
    HDC dc = nullptr;
    int W = 0, H = 0;
    std::map<long long, HFONT> fonts;

    HFONT font(int pt, int angleTenths) {
        long long key = ((long long)pt << 16) | angleTenths;
        auto it = fonts.find(key);
        if (it != fonts.end()) return it->second;
        HFONT f = CreateFontA(-(int)std::lround(pt * 96.0 / 72.0), 0, angleTenths, angleTenths, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                              FIXED_PITCH | FF_MODERN, "Consolas");
        fonts[key] = f;
        return f;
    }
    void clear(rt::Color c) override {
        HBRUSH b = CreateSolidBrush(cr(c));
        RECT r = {0, 0, W, H};
        FillRect(dc, &r, b);
        DeleteObject(b);
    }
    void text(double x, double y, const std::string& s, int pt, rt::Anchor a, rt::Color c, double ang, int wrap) override {
        int at = (int)std::lround(ang * 10.0) % 3600;
        if (at < 0) at += 3600;
        HFONT old = (HFONT)SelectObject(dc, font(pt, wrap > 0 ? 0 : at));
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, cr(c));
        if (wrap > 0) {
            RECT r = {(LONG)x, (LONG)y, (LONG)x + wrap, (LONG)y + 4000};
            DrawTextA(dc, s.c_str(), -1, &r, DT_WORDBREAK | DT_NOPREFIX | DT_LEFT | DT_TOP);
            SelectObject(dc, old);
            return;
        }
        SIZE sz = {0, 0};
        GetTextExtentPoint32A(dc, s.c_str(), (int)s.size(), &sz);
        double w = sz.cx, h = sz.cy, ox = 0, oy = 0;
        switch (a) {
            case rt::NW: break;
            case rt::W: oy = -h / 2; break;
            case rt::C: ox = -w / 2; oy = -h / 2; break;
            case rt::NE: ox = -w; break;
            case rt::SW: oy = -h; break;
        }
        double r = ang * PI / 180.0, ca = std::cos(r), sa = std::sin(r);
        double px = x + ox * ca + oy * sa, py = y - ox * sa + oy * ca;
        SetTextAlign(dc, TA_LEFT | TA_TOP | TA_NOUPDATECP);
        TextOutA(dc, (int)std::lround(px), (int)std::lround(py), s.c_str(), (int)s.size());
        SelectObject(dc, old);
    }
    void rect(double x1, double y1, double x2, double y2, rt::Color o, const rt::Color* fill, int w) override {
        HPEN p = CreatePen(PS_INSIDEFRAME, w, cr(o));
        HBRUSH b = fill ? CreateSolidBrush(cr(*fill)) : (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ op = SelectObject(dc, p), ob = SelectObject(dc, b);
        Rectangle(dc, (int)std::lround(x1), (int)std::lround(y1), (int)std::lround(x2), (int)std::lround(y2));
        SelectObject(dc, op);
        SelectObject(dc, ob);
        DeleteObject(p);
        if (fill) DeleteObject(b);
    }
    void line(double x1, double y1, double x2, double y2, rt::Color c, int w) override {
        HPEN p = CreatePen(PS_SOLID, w, cr(c));
        HGDIOBJ op = SelectObject(dc, p);
        MoveToEx(dc, (int)std::lround(x1), (int)std::lround(y1), nullptr);
        LineTo(dc, (int)std::lround(x2), (int)std::lround(y2));
        SelectObject(dc, op);
        DeleteObject(p);
    }
    void poly(const std::vector<rt::Pt>& pts, rt::Color o, const rt::Color* fill, int w) override {
        std::vector<POINT> v;
        for (auto& p : pts) { POINT q = {(LONG)std::lround(p.x), (LONG)std::lround(p.y)}; v.push_back(q); }
        HPEN p = CreatePen(PS_SOLID, w, cr(o));
        HBRUSH b = fill ? CreateSolidBrush(cr(*fill)) : (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ op = SelectObject(dc, p), ob = SelectObject(dc, b);
        Polygon(dc, v.data(), (int)v.size());
        SelectObject(dc, op);
        SelectObject(dc, ob);
        DeleteObject(p);
        if (fill) DeleteObject(b);
    }
    void oval(double cx, double cy, double r, rt::Color o, const rt::Color* fill, int w) override {
        HPEN p = CreatePen(PS_SOLID, w, cr(o));
        HBRUSH b = fill ? CreateSolidBrush(cr(*fill)) : (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ op = SelectObject(dc, p), ob = SelectObject(dc, b);
        Ellipse(dc, (int)std::lround(cx - r), (int)std::lround(cy - r), (int)std::lround(cx + r), (int)std::lround(cy + r));
        SelectObject(dc, op);
        SelectObject(dc, ob);
        DeleteObject(p);
        if (fill) DeleteObject(b);
    }
};
static Gdi gGdi;

static void ensureBuffer(HDC hdc, int w, int h) {
    if (gMem && w == gBufW && h == gBufH) return;
    if (gMem) {
        SelectObject(gMem, gOldBmp);
        DeleteObject(gBmp);
        DeleteDC(gMem);
    }
    gMem = CreateCompatibleDC(hdc);
    gBmp = CreateCompatibleBitmap(hdc, w, h);
    gOldBmp = (HBITMAP)SelectObject(gMem, gBmp);
    gBufW = w;
    gBufH = h;
}

static void refresh(HWND h) {
    if (gApp->dirty) {
        InvalidateRect(h, nullptr, FALSE);
        UpdateWindow(h);  // paint right now so the flash time is measured accurately
    }
}

static void toggleFullscreen() {
    static WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
    static bool full = false;
    LONG style = GetWindowLongA(gWnd, GWL_STYLE);
    if (!full) {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        if (GetWindowPlacement(gWnd, &wp) && GetMonitorInfoA(MonitorFromWindow(gWnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongA(gWnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(gWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            full = true;
        }
    } else {
        SetWindowLongA(gWnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(gWnd, &wp);
        SetWindowPos(gWnd, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        full = false;
    }
}

static DWORD WINAPI beepThread(LPVOID) {
    Beep(1000, 150);
    return 0;
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_CREATE:
            SetTimer(h, 1, 10, nullptr);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mm = (MINMAXINFO*)l;
            mm->ptMinTrackSize.x = 760;
            mm->ptMinTrackSize.y = 520;
            return 0;
        }
        case WM_SIZE:
            InvalidateRect(h, nullptr, FALSE);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(h, &ps);
            RECT rc;
            GetClientRect(h, &rc);
            int W = rc.right - rc.left, H = rc.bottom - rc.top;
            if (W > 0 && H > 0 && gApp) {
                ensureBuffer(hdc, W, H);
                gGdi.dc = gMem;
                gGdi.W = W;
                gGdi.H = H;
                gApp->draw(gGdi, W, H);
                BitBlt(hdc, 0, 0, W, H, gMem, 0, 0, SRCCOPY);
                GdiFlush();
                gApp->presented();
            }
            EndPaint(h, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN:
            gApp->click((short)LOWORD(l), (short)HIWORD(l));
            refresh(h);
            return 0;
        case WM_KEYDOWN: {
            if (l & (1 << 30)) return 0;  // ignore auto-repeat
            int k = (int)w;
            if (k >= VK_NUMPAD0 && k <= VK_NUMPAD9) k = '0' + (k - VK_NUMPAD0);
            gApp->key(k);
            refresh(h);
            return 0;
        }
        case WM_SYSKEYDOWN:
            if (w == VK_F10) {
                if (!(l & (1 << 30))) { gApp->key(0x79); refresh(h); }
                return 0;
            }
            break;
        case WM_TIMER:
            if (gApp->update()) refresh(h);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    SetProcessDPIAware();
    HDC sdc = GetDC(nullptr);
    double scale = GetDeviceCaps(sdc, LOGPIXELSX) / 96.0;
    ReleaseDC(nullptr, sdc);

    WNDCLASSA wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.lpszClassName = "ReactionTrainerWnd";
    RegisterClassA(&wc);

    rt::Trainer app(nowMs);
    gApp = &app;
    app.onBeep = [] { HANDLE t = CreateThread(nullptr, 0, beepThread, nullptr, 0, nullptr); if (t) CloseHandle(t); };
    app.onQuit = [] { DestroyWindow(gWnd); };
    app.onToggleFs = toggleFullscreen;

    gWnd = CreateWindowExA(0, wc.lpszClassName, "Reaction Trainer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                           (int)(1100 * scale), (int)(720 * scale), nullptr, nullptr, inst, nullptr);
    ShowWindow(gWnd, show);
    UpdateWindow(gWnd);

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
