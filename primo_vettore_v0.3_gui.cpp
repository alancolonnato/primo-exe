#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

const int ID_EDIT_INPUT = 1001;
const int ID_BTN_CALC = 1002;
const int ID_BTN_QUIT = 1003;
const int ID_STATIC_LABEL = 1004;
const int ID_STATIC_OUT = 1005;

struct Nyan {
    double fx;
    double fy;
    int size;
    int speed;
    int phase;
};

static std::vector<Nyan> cats;
static POINT stars[80];
static int tickCount = 0;
static HBRUSH g_panelBrush = nullptr;
static HWND hInput = nullptr;
static HWND hOut = nullptr;
static HWND hLabel = nullptr;
static HWND hCalc = nullptr;
static HWND hQuit = nullptr;
static int g_skyTop = 0;

static void FillRectColor(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {
    RECT r = { x1, y1, x2, y2 };
    HBRUSH b = CreateSolidBrush(c);
    FillRect(hdc, &r, b);
    DeleteObject(b);
}

static void DrawNyan(HDC hdc, int w, int h, const Nyan& n) {
    int s = n.size;
    int x = (int)(n.fx * (w + 4 * s)) - 2 * s;
    int sky = std::max(1, h - g_skyTop);
    int y = g_skyTop + (int)(n.fy * std::max(1, sky - s));
    y = std::max(g_skyTop, std::min(y, h - s - 4));

    int stripeH = std::max(2, s / 9);
    int trail = s * 3;
    COLORREF rainbow[6] = {
        RGB(255, 70, 70), RGB(255, 170, 60), RGB(255, 240, 80),
        RGB(80, 220, 120), RGB(80, 160, 255), RGB(190, 100, 255)
    };
    for (int i = 0; i < 6; i++) {
        int yy = y + i * stripeH;
        if (yy >= y + s) break;
        int wobble = (tickCount / 4 + n.phase) % stripeH;
        FillRectColor(hdc, x - trail + wobble, yy, x, std::min(yy + stripeH, y + s), rainbow[i]);
    }

    int leg = ((tickCount / 6 + n.phase) % 2) ? (s / 6) : 0;
    FillRectColor(hdc, x + s / 5, y + s * 3 / 4, x + s * 2 / 5, y + s + leg, RGB(200, 200, 212));
    FillRectColor(hdc, x + s * 3 / 5, y + s * 3 / 4, x + s * 4 / 5,
                  y + s + (leg ? 0 : s / 6), RGB(200, 200, 212));
    FillRectColor(hdc, x - s / 4, y + s / 2, x, y + s * 3 / 4, RGB(225, 225, 235));
    FillRectColor(hdc, x, y + s / 4, x + s, y + s * 3 / 4, RGB(225, 225, 235));
    FillRectColor(hdc, x + s / 2, y, x + s, y + s * 3 / 5, RGB(245, 245, 250));
    FillRectColor(hdc, x + s * 3 / 5, y - s / 6, x + s * 3 / 4, y + s / 8, RGB(245, 245, 250));
    FillRectColor(hdc, x + s * 4 / 5, y - s / 6, x + s, y + s / 8, RGB(245, 245, 250));

    int eye = std::max(2, s / 12);
    FillRectColor(hdc, x + s * 3 / 5, y + s / 6, x + s * 3 / 5 + eye, y + s / 6 + eye, RGB(0, 0, 0));
    FillRectColor(hdc, x + s * 4 / 5, y + s / 6, x + s * 4 / 5 + eye, y + s / 6 + eye, RGB(0, 0, 0));
    FillRectColor(hdc, x + s * 7 / 10, y + s * 2 / 5, x + s * 4 / 5, y + s / 2, RGB(255, 120, 160));
}

static void Layout(HWND hwnd, int w, int h) {
    const int m = 12;
    int fieldW = std::max(120, w - 2 * m);
    int by = m + 26 + 64 + 8;
    int oy = by + 36;
    int outH = std::min(200, std::max(70, (h - oy - m) / 2));
    g_skyTop = oy + outH;

    MoveWindow(hLabel, m, m, fieldW, 20, TRUE);
    MoveWindow(hInput, m, m + 26, fieldW, 64, TRUE);
    MoveWindow(hCalc, m, by, 100, 28, TRUE);
    MoveWindow(hQuit, m + 108, by, 80, 28, TRUE);
    MoveWindow(hOut, m, oy, fieldW, outH, TRUE);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HINSTANCE inst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        g_panelBrush = CreateSolidBrush(RGB(10, 12, 35));

        hLabel = CreateWindowEx(0, "STATIC", "Inserisci i valori separati da spazio:",
            WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, hwnd, (HMENU)(INT_PTR)ID_STATIC_LABEL, inst, nullptr);
        hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            0, 0, 1, 1, hwnd, (HMENU)(INT_PTR)ID_EDIT_INPUT, inst, nullptr);
        hCalc = CreateWindowEx(0, "BUTTON", "Calcola",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 1, 1, hwnd,
            (HMENU)(INT_PTR)ID_BTN_CALC, inst, nullptr);
        hQuit = CreateWindowEx(0, "BUTTON", "Esci",
            WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, hwnd, (HMENU)(INT_PTR)ID_BTN_QUIT, inst, nullptr);
        hOut = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "Risultati qui.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 1, 1, hwnd,
            (HMENU)(INT_PTR)ID_STATIC_OUT, inst, nullptr);

        srand(1337);
        for (int i = 0; i < 80; i++) {
            stars[i].x = rand() % 1000;
            stars[i].y = rand() % 700;
        }
        cats.push_back({ -0.10, 0.10, 56, 3, 0 });
        cats.push_back({ 0.45, 0.45, 42, 2, 7 });
        cats.push_back({ 0.75, 0.78, 34, 4, 13 });

        RECT rc; GetClientRect(hwnd, &rc);
        Layout(hwnd, rc.right, rc.bottom);
        SetTimer(hwnd, 1, 40, nullptr);
        return 0;
    }

    case WM_SIZE:
        Layout(hwnd, LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_GETMINMAXINFO: {
        MINMAXINFO* m = (MINMAXINFO*)lParam;
        m->ptMinTrackSize.x = 340;
        m->ptMinTrackSize.y = 260;
        return 0;
    }

    case WM_DPICHANGED: {
        RECT* r = (RECT*)lParam;
        MoveWindow(hwnd, r->left, r->top, r->right - r->left, r->bottom - r->top, TRUE);
        return 0;
    }

    case WM_TIMER: {
        RECT rc; GetClientRect(hwnd, &rc);
        int w = (int)std::max<LONG>(1, rc.right);
        tickCount++;
        for (Nyan& n : cats) {
            n.fx += n.speed / (double)(w + 4 * n.size);
            if (n.fx > 1.05) n.fx = -0.05;
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_ERASEBKGND:
        return TRUE;

    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wParam;
        SetTextColor(dc, RGB(235, 240, 255));
        SetBkColor(dc, RGB(10, 12, 35));
        return (LRESULT)g_panelBrush;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int w = (int)std::max<LONG>(1, rc.right);
        int h = (int)std::max<LONG>(1, rc.bottom);

        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, w, h);
        HGDIOBJ old = SelectObject(mem, bmp);

        FillRectColor(mem, 0, 0, w, h, RGB(8, 10, 32));
        for (int i = 0; i < 80; i++) {
            int sx = (int)((long long)stars[i].x * w / 1000);
            int sy = (int)((long long)stars[i].y * h / 700);
            int sz = ((tickCount / 8 + i) % 10 < 6) ? 2 : 1;
            FillRectColor(mem, sx, sy, sx + sz, sy + sz,
                          (i % 5 == 0) ? RGB(255, 255, 180) : RGB(200, 210, 255));
        }
        for (Nyan& n : cats) DrawNyan(mem, w, h, n);

        SetTextColor(mem, RGB(120, 140, 220));
        SetBkMode(mem, TRANSPARENT);
        TextOut(mem, 12, h - 24, "NYAN v0.3", 9);

        BitBlt(hdc, 0, 0, w, h, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_QUIT) {
            PostQuitMessage(0);
            return 0;
        }
        if (LOWORD(wParam) == ID_BTN_CALC) {
            char buffer[8192];
            GetWindowText(hInput, buffer, 8192);
            std::istringstream iss(buffer);
            std::vector<int> v;
            int x = 0;
            while (iss >> x) v.push_back(x);

            std::ostringstream out;
            if (v.empty()) {
                out << "Nessun valore valido.";
            } else {
                long long somma = 0;
                int massimo = v[0];
                int minimo = v[0];
                for (int y : v) {
                    somma += y;
                    if (y > massimo) massimo = y;
                    if (y < minimo) minimo = y;
                }
                double media = static_cast<double>(somma) / v.size();
                out << "Media: " << media << "\nSomma: " << somma
                    << "\nMassimo: " << massimo << "\nMinimo: " << minimo
                    << "\nValori: " << v.size();
            }
            SetWindowText(hOut, out.str().c_str());
            return 0;
        }
        break;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        if (g_panelBrush) DeleteObject(g_panelBrush);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void EnableDpiAwareness() {
    HMODULE user32 = LoadLibraryA("user32.dll");
    if (!user32) return;
    typedef BOOL(WINAPI* PFN_CTX)(void*);
    typedef BOOL(WINAPI* PFN_OLD)(void);
    PFN_CTX setCtx = (PFN_CTX)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
    if (setCtx) {
        setCtx((void*)-4);
    } else {
        PFN_OLD setOld = (PFN_OLD)GetProcAddress(user32, "SetProcessDPIAware");
        if (setOld) setOld();
    }
    FreeLibrary(user32);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    EnableDpiAwareness();

    const char* className = "PrimoVettoreNyan";
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    if (!RegisterClassEx(&wc)) return 1;

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int ww = 560, wh = 420;
    HWND hwnd = CreateWindowEx(0, className, "Primo vettore v0.3 - Nyan",
        WS_OVERLAPPEDWINDOW, (sw - ww) / 2, (sh - wh) / 2, ww, wh,
        nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 1;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
