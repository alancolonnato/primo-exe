#include <windows.h>
#include <sstream>
#include <string>
#include <vector>

const int ID_EDIT_INPUT = 1001;
const int ID_BTN_CALC = 1002;
const int ID_BTN_QUIT = 1003;
const int ID_STATIC_LABEL = 1004;
const int ID_STATIC_OUT = 1005;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hInput = nullptr;
    static HWND hOut = nullptr;

    switch (msg) {
    case WM_CREATE: {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        CreateWindowEx(0, "STATIC", "Inserisci i valori separati da spazio:",
            WS_CHILD | WS_VISIBLE, 20, 20, 330, 20, hwnd,
            (HMENU)(INT_PTR)ID_STATIC_LABEL, hInst, nullptr);

        hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            20, 45, 330, 70, hwnd, (HMENU)(INT_PTR)ID_EDIT_INPUT, hInst, nullptr);

        CreateWindowEx(0, "BUTTON", "Calcola",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 20, 130, 100, 28, hwnd,
            (HMENU)(INT_PTR)ID_BTN_CALC, hInst, nullptr);

        CreateWindowEx(0, "BUTTON", "Esci",
            WS_CHILD | WS_VISIBLE, 135, 130, 80, 28, hwnd,
            (HMENU)(INT_PTR)ID_BTN_QUIT, hInst, nullptr);

        hOut = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "Risultati qui.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 175, 330, 90, hwnd,
            (HMENU)(INT_PTR)ID_STATIC_OUT, hInst, nullptr);
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
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    const char* className = "PrimoVettoreGUI";
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    if (!RegisterClassEx(&wc)) return 1;

    HWND hwnd = CreateWindowEx(0, className, "Primo vettore v0.2",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 320,
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
