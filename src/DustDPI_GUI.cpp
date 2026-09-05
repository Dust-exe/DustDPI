#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <string>
#include <sstream>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_SHOW    2001
#define ID_TRAY_START   2002
#define ID_TRAY_STOP    2003
#define ID_TRAY_EDIT    2004
#define ID_TRAY_EXIT    2005

#define IDC_BTN_START     101
#define IDC_BTN_STOP      102
#define IDC_BTN_RESTART   103
#define IDC_BTN_INSTALL   104
#define IDC_BTN_BLACKLIST 105
#define IDC_BTN_CONSOLE   106

enum ServiceState {
    STATE_UNKNOWN,
    STATE_NOT_INSTALLED,
    STATE_STOPPED,
    STATE_START_PENDING,
    STATE_STOP_PENDING,
    STATE_RUNNING
};

static NOTIFYICONDATA g_nid = {0};
static HWND g_hMainWnd = NULL;
static HBRUSH g_hBgBrush = NULL;
static HBRUSH g_hCardBrush = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontStatus = NULL;
static HFONT g_hFontSub = NULL;
static ServiceState g_curState = STATE_UNKNOWN;
static char g_appDir[MAX_PATH] = {0};

std::string GetAppDirectory() {
    if (g_appDir[0] == '\0') {
        GetModuleFileNameA(NULL, g_appDir, MAX_PATH);
        char* lastSlash = strrchr(g_appDir, '\\');
        if (lastSlash) *lastSlash = '\0';
    }
    return std::string(g_appDir);
}

ServiceState QueryDpiService() {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return STATE_UNKNOWN;

    SC_HANDLE svc = OpenServiceA(scm, "GoodbyeDPI", SERVICE_QUERY_STATUS);
    if (!svc) {
        CloseServiceHandle(scm);
        return STATE_NOT_INSTALLED;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    ServiceState st = STATE_UNKNOWN;
    if (QueryServiceStatusEx(svc, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
        switch (ssp.dwCurrentState) {
            case SERVICE_RUNNING: st = STATE_RUNNING; break;
            case SERVICE_STOPPED: st = STATE_STOPPED; break;
            case SERVICE_START_PENDING: st = STATE_START_PENDING; break;
            case SERVICE_STOP_PENDING: st = STATE_STOP_PENDING; break;
            default: st = STATE_UNKNOWN; break;
        }
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return st;
}

void RunCmdAsync(const std::string& cmd, bool elevated = false) {
    ShellExecuteA(NULL, elevated ? "runas" : "open", "cmd.exe", ("/c " + cmd).c_str(), GetAppDirectory().c_str(), SW_HIDE);
}

void ActionStartService() {
    RunCmdAsync("sc.exe start \"GoodbyeDPI\"", true);
}

void ActionStopService() {
    RunCmdAsync("sc.exe stop \"GoodbyeDPI\"", true);
}

void ActionRestartService() {
    RunCmdAsync("sc.exe stop \"GoodbyeDPI\" & timeout /t 1 & sc.exe start \"GoodbyeDPI\"", true);
}

void ActionInstallService() {
    std::string dir = GetAppDirectory();
    std::string exe64 = dir + "\\x86_64\\goodbyedpi.exe";
    std::string bl = dir + "\\blacklist.txt";
    std::string cmd = "sc.exe stop \"GoodbyeDPI\" & sc.exe delete \"GoodbyeDPI\" & ";
    cmd += "sc.exe create \"GoodbyeDPI\" binPath= \"\\\"" + exe64 + "\\\" -5 --set-ttl 5 --blacklist \\\"" + bl + "\\\"\" start= auto DisplayName= \"DustDPI - Selective Service\" & ";
    cmd += "sc.exe description \"GoodbyeDPI\" \"Dust Studio Selective DPI Circumvention Service\" & ";
    cmd += "sc.exe start \"GoodbyeDPI\"";
    RunCmdAsync(cmd, true);
}

void ActionEditBlacklist() {
    std::string bl = GetAppDirectory() + "\\blacklist.txt";
    ShellExecuteA(NULL, "open", "notepad.exe", bl.c_str(), NULL, SW_SHOW);
}

void ActionRunConsole() {
    std::string dir = GetAppDirectory();
    std::string exe64 = dir + "\\x86_64\\goodbyedpi.exe";
    std::string bl = dir + "\\blacklist.txt";
    std::string args = "/k title DustDPI Console Test & \"" + exe64 + "\" -5 --set-ttl 5 --blacklist \"" + bl + "\"";
    ShellExecuteA(NULL, "runas", "cmd.exe", args.c_str(), dir.c_str(), SW_SHOW);
}

void SetupTray(HWND hWnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
    if (!g_nid.hIcon) {
        g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    strcpy(g_nid.szTip, "Dust Studio — DustDPI Control Matrix");
    Shell_NotifyIconA(NIM_ADD, &g_nid);
}

void ShowTrayMenu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_SHOW, "DustDPI Penceresini Ac");
    AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_START, "Servisi Baslat");
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_STOP, "Servisi Durdur");
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_EDIT, "Hedef Listesini Duzenle (blacklist.txt)");
    AppendMenuA(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "Cikis");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hWnd;
            
            // Enable Windows 10/11 Dark Titlebar
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

            // Fonts
            g_hFontTitle = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            g_hFontSub = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            g_hFontStatus = CreateFontA(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
            g_hFontNormal = CreateFontA(15, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");

            // Brushes (Obsidian dark palette #0D0B14 and card #171424)
            g_hBgBrush = CreateSolidBrush(RGB(13, 11, 20));
            g_hCardBrush = CreateSolidBrush(RGB(23, 20, 36));

            // Buttons
            CreateWindowA("BUTTON", "▶  Servisi Baslat", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          35, 175, 200, 38, hWnd, (HMENU)IDC_BTN_START, NULL, NULL);

            CreateWindowA("BUTTON", "⏹  Servisi Durdur", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          255, 175, 200, 38, hWnd, (HMENU)IDC_BTN_STOP, NULL, NULL);

            CreateWindowA("BUTTON", "🔄  Yeniden Baslat", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          35, 225, 200, 38, hWnd, (HMENU)IDC_BTN_RESTART, NULL, NULL);

            CreateWindowA("BUTTON", "📝  Hedef Listesi (Blacklist)", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          255, 225, 200, 38, hWnd, (HMENU)IDC_BTN_BLACKLIST, NULL, NULL);

            CreateWindowA("BUTTON", "💻  Konsol Test Modu", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          35, 275, 200, 38, hWnd, (HMENU)IDC_BTN_CONSOLE, NULL, NULL);

            CreateWindowA("BUTTON", "⚙  Servisi Yeniden Kur", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_FLAT,
                          255, 275, 200, 38, hWnd, (HMENU)IDC_BTN_INSTALL, NULL, NULL);

            // Set buttons font
            for (int id = IDC_BTN_START; id <= IDC_BTN_CONSOLE; id++) {
                HWND hBtn = GetDlgItem(hWnd, id);
                if (hBtn) SendMessage(hBtn, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
            }

            SetupTray(hWnd);
            SetTimer(hWnd, 1, 1000, NULL);
            g_curState = QueryDpiService();
            break;
        }

        case WM_TIMER: {
            ServiceState newState = QueryDpiService();
            if (newState != g_curState) {
                g_curState = newState;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDC_BTN_START: ActionStartService(); break;
                case IDC_BTN_STOP: ActionStopService(); break;
                case IDC_BTN_RESTART: ActionRestartService(); break;
                case IDC_BTN_INSTALL: ActionInstallService(); break;
                case IDC_BTN_BLACKLIST: ActionEditBlacklist(); break;
                case IDC_BTN_CONSOLE: ActionRunConsole(); break;
                case ID_TRAY_SHOW:
                    ShowWindow(hWnd, SW_RESTORE);
                    SetForegroundWindow(hWnd);
                    break;
                case ID_TRAY_START: ActionStartService(); break;
                case ID_TRAY_STOP: ActionStopService(); break;
                case ID_TRAY_EDIT: ActionEditBlacklist(); break;
                case ID_TRAY_EXIT:
                    DestroyWindow(hWnd);
                    break;
            }
            // Trigger status refresh
            SetTimer(hWnd, 2, 400, NULL);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);

            // Fill background
            FillRect(hdc, &clientRect, g_hBgBrush);

            SetBkMode(hdc, TRANSPARENT);

            // Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, RGB(245, 243, 255)); // Near white
            TextOutA(hdc, 35, 22, "DustDPI", 7);

            // Subtitle
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(167, 139, 250)); // Purple-400
            TextOutA(hdc, 130, 29, "•  Secici DPI Bypass & Guvenlik Kalkanı", 41);

            // Description info
            SetTextColor(hdc, RGB(161, 161, 170)); // Zinc-400
            TextOutA(hdc, 35, 52, "Discord, Roblox ve Turkiye engelli hedefleri filtreler; diger uygulamalara dokunmaz.", 85);

            // Status Card Box
            RECT cardRect = { 35, 85, 455, 145 };
            FillRect(hdc, &cardRect, g_hCardBrush);

            // Draw card border
            HBRUSH borderBrush = CreateSolidBrush(RGB(76, 29, 149)); // Purple border
            FrameRect(hdc, &cardRect, borderBrush);
            DeleteObject(borderBrush);

            // Draw status indicator
            SelectObject(hdc, g_hFontStatus);
            if (g_curState == STATE_RUNNING) {
                SetTextColor(hdc, RGB(52, 211, 153)); // Emerald green
                TextOutA(hdc, 55, 103, "●  DURUM: AKTIF (Servis Calisiyor)", 34);
            } else if (g_curState == STATE_STOPPED) {
                SetTextColor(hdc, RGB(248, 113, 113)); // Rose red
                TextOutA(hdc, 55, 103, "●  DURUM: DURDURULDU (Servis Kapali)", 37);
            } else if (g_curState == STATE_NOT_INSTALLED) {
                SetTextColor(hdc, RGB(251, 191, 36)); // Amber
                TextOutA(hdc, 55, 103, "●  DURUM: KURULU DEGIL ('Yeniden Kur'a basin)", 45);
            } else {
                SetTextColor(hdc, RGB(192, 132, 252)); // Purple
                TextOutA(hdc, 55, 103, "●  DURUM: ISLEM YAPILIYOR...", 27);
            }

            // Footer info
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(113, 113, 122)); // Zinc-500
            TextOutA(hdc, 35, 332, "Dust Studio  •  dust-studio.com  •  Antigravity IDE & Oyun Uyumlu", 66);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenu(hWnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
            }
            break;
        }

        case WM_SYSCOMMAND: {
            if ((wParam & 0xFFF0) == SC_MINIMIZE) {
                ShowWindow(hWnd, SW_HIDE);
                return 0;
            }
            return DefWindowProcA(hWnd, msg, wParam, lParam);
        }

        case WM_DESTROY: {
            KillTimer(hWnd, 1);
            Shell_NotifyIconA(NIM_DELETE, &g_nid);
            if (g_hBgBrush) DeleteObject(g_hBgBrush);
            if (g_hCardBrush) DeleteObject(g_hCardBrush);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontSub) DeleteObject(g_hFontSub);
            if (g_hFontStatus) DeleteObject(g_hFontStatus);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "DustDPI_Class";
    wc.hIcon = LoadIconA(hInstance, "APP_ICON");
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExA(&wc);

    int w = 505;
    int h = 395;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND hWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "DustDPI_Class",
        "Dust Studio — DustDPI Control Matrix",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, w, h,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
