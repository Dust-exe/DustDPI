#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <string>
#include <vector>

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
#define IDC_BTN_BLACKLIST 104
#define IDC_BTN_CONSOLE   105
#define IDC_BTN_INSTALL   106

enum ServiceState {
    STATE_UNKNOWN,
    STATE_NOT_INSTALLED,
    STATE_STOPPED,
    STATE_START_PENDING,
    STATE_STOP_PENDING,
    STATE_RUNNING
};

static NOTIFYICONDATAW g_nid = {0};
static HWND g_hMainWnd = NULL;
static HBRUSH g_hBgBrush = NULL;
static HBRUSH g_hCardBrush = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontStatus = NULL;
static HFONT g_hFontSub = NULL;
static HFONT g_hFontBtn = NULL;
static ServiceState g_curState = STATE_UNKNOWN;
static wchar_t g_appDir[MAX_PATH] = {0};
static int g_hoverBtnId = 0;

std::wstring GetAppDirectoryW() {
    if (g_appDir[0] == L'\0') {
        GetModuleFileNameW(NULL, g_appDir, MAX_PATH);
        wchar_t* lastSlash = wcsrchr(g_appDir, L'\\');
        if (lastSlash) *lastSlash = L'\0';
    }
    return std::wstring(g_appDir);
}

ServiceState QueryDpiService() {
    SC_HANDLE scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return STATE_UNKNOWN;

    SC_HANDLE svc = OpenServiceW(scm, L"DustDPI", SERVICE_QUERY_STATUS);
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

void RunCmdAsyncW(const std::wstring& cmd, bool elevated = false) {
    ShellExecuteW(NULL, elevated ? L"runas" : L"open", L"cmd.exe", (L"/c " + cmd).c_str(), GetAppDirectoryW().c_str(), SW_HIDE);
}

void ActionStartService() {
    RunCmdAsyncW(L"sc.exe start \"DustDPI\"", true);
}

void ActionStopService() {
    RunCmdAsyncW(L"sc.exe stop \"DustDPI\"", true);
}

void ActionRestartService() {
    RunCmdAsyncW(L"sc.exe stop \"DustDPI\" & timeout /t 1 & sc.exe start \"DustDPI\"", true);
}

void ActionInstallService() {
    std::wstring dir = GetAppDirectoryW();
    std::wstring exe64 = dir + L"\\x86_64\\dust_engine.exe";
    std::wstring bl = dir + L"\\blacklist.txt";
    std::wstring cmd = L"sc.exe stop \"DustDPI\" & sc.exe delete \"DustDPI\" & ";
    cmd += L"sc.exe create \"DustDPI\" binPath= \"\\\"" + exe64 + L"\\\" -9 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \\\"" + bl + L"\\\"\" start= auto DisplayName= \"DustDPI Service\" & ";
    cmd += L"sc.exe description \"DustDPI\" \"Dust Studio Selective Network Optimization Service\" & ";
    cmd += L"sc.exe start \"DustDPI\"";
    RunCmdAsyncW(cmd, true);
}

void ActionEditBlacklist() {
    std::wstring bl = GetAppDirectoryW() + L"\\blacklist.txt";
    ShellExecuteW(NULL, L"open", L"notepad.exe", bl.c_str(), NULL, SW_SHOW);
}

void ActionRunConsole() {
    std::wstring dir = GetAppDirectoryW();
    std::wstring exe64 = dir + L"\\x86_64\\dust_engine.exe";
    std::wstring bl = dir + L"\\blacklist.txt";
    std::wstring args = L"/k title DustDPI Console Test & \"" + exe64 + L"\" -9 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \"" + bl + L"\"";
    ShellExecuteW(NULL, L"runas", L"cmd.exe", args.c_str(), dir.c_str(), SW_SHOW);
}

void SetupTrayW(HWND hWnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICON);
    if (!g_nid.hIcon) {
        g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    }
    wcscpy(g_nid.szTip, L"Dust Studio - DustDPI Service Manager");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

void ShowTrayMenuW(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW, L"DustDPI Kontrol Panelini Ac");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_START, L"Servisi Baslat");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_STOP, L"Servisi Durdur");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EDIT, L"Hedef Listesini Duzenle (blacklist.txt)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Cikis");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

// Custom button window procedure to handle hover states
static WNDPROC g_oldBtnProc = NULL;
static LRESULT CALLBACK CustomButtonProc(HWND hBtn, UINT msg, WPARAM wParam, LPARAM lParam) {
    int id = GetWindowLong(hBtn, GWL_ID);
    switch (msg) {
        case WM_MOUSEMOVE: {
            if (g_hoverBtnId != id) {
                g_hoverBtnId = id;
                InvalidateRect(hBtn, NULL, FALSE);
                TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hBtn, 0 };
                TrackMouseEvent(&tme);
            }
            break;
        }
        case WM_MOUSELEAVE: {
            if (g_hoverBtnId == id) {
                g_hoverBtnId = 0;
                InvalidateRect(hBtn, NULL, FALSE);
            }
            break;
        }
    }
    return CallWindowProcW(g_oldBtnProc, hBtn, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hWnd;
            
            // Enable Windows 10/11 Dark Titlebar
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

            // Fonts: Segoe UI ClearType
            g_hFontTitle = CreateFontW(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontSub = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontStatus = CreateFontW(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontNormal = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontBtn = CreateFontW(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

            // Brushes (Modern dark palette: #0b0f19 and card #161f30)
            g_hBgBrush = CreateSolidBrush(RGB(11, 15, 25));
            g_hCardBrush = CreateSolidBrush(RGB(22, 31, 48));

            // Grid buttons (BS_OWNERDRAW for smooth modern styling)
            struct BtnDef {
                int id;
                const wchar_t* text;
                int x, y, w, h;
            };

            BtnDef buttons[] = {
                { IDC_BTN_START,     L"Servisi Baslat",           35, 185, 215, 42 },
                { IDC_BTN_STOP,      L"Servisi Durdur",          265, 185, 215, 42 },
                { IDC_BTN_RESTART,   L"Yeniden Baslat",           35, 240, 215, 42 },
                { IDC_BTN_BLACKLIST, L"Hedef Listesi (Blacklist)",265, 240, 215, 42 },
                { IDC_BTN_CONSOLE,   L"Canli Test Modu",          35, 295, 215, 42 },
                { IDC_BTN_INSTALL,   L"Servisi Onar / Kur",      265, 295, 215, 42 }
            };

            for (const auto& b : buttons) {
                HWND hBtn = CreateWindowExW(
                    0, L"BUTTON", b.text,
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                    b.x, b.y, b.w, b.h, hWnd, (HMENU)(INT_PTR)b.id, NULL, NULL
                );
                if (!g_oldBtnProc) {
                    g_oldBtnProc = (WNDPROC)GetWindowLongPtrW(hBtn, GWLP_WNDPROC);
                }
                SetWindowLongPtrW(hBtn, GWLP_WNDPROC, (LONG_PTR)CustomButtonProc);
            }

            SetupTrayW(hWnd);
            SetTimer(hWnd, 1, 1000, NULL);
            g_curState = QueryDpiService();
            break;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
            if (pDIS->CtlType == ODT_BUTTON) {
                bool isHover = (g_hoverBtnId == (int)pDIS->CtlID);
                bool isPressed = (pDIS->itemState & ODS_SELECTED);
                
                COLORREF bgColor;
                COLORREF borderColor;
                COLORREF textColor = RGB(241, 245, 249); // White text

                if (pDIS->CtlID == IDC_BTN_START) {
                    // Primary button: Vibrant Blue
                    if (isPressed) {
                        bgColor = RGB(29, 78, 216);
                        borderColor = RGB(37, 99, 235);
                    } else if (isHover) {
                        bgColor = RGB(59, 130, 246);
                        borderColor = RGB(96, 165, 250);
                    } else {
                        bgColor = RGB(37, 99, 235);
                        borderColor = RGB(59, 130, 246);
                    }
                } else if (pDIS->CtlID == IDC_BTN_STOP) {
                    // Secondary Danger: Dark Wine/Red
                    if (isPressed) {
                        bgColor = RGB(69, 26, 26);
                        borderColor = RGB(185, 28, 28);
                    } else if (isHover) {
                        bgColor = RGB(127, 29, 29);
                        borderColor = RGB(239, 68, 68);
                    } else {
                        bgColor = RGB(45, 20, 20);
                        borderColor = RGB(127, 29, 29);
                    }
                } else {
                    // Neutral Dark Slate buttons
                    if (isPressed) {
                        bgColor = RGB(15, 23, 42);
                        borderColor = RGB(71, 85, 105);
                    } else if (isHover) {
                        bgColor = RGB(51, 65, 85);
                        borderColor = RGB(100, 116, 139);
                    } else {
                        bgColor = RGB(30, 41, 59);
                        borderColor = RGB(51, 65, 85);
                    }
                }

                HBRUSH btnBrush = CreateSolidBrush(bgColor);
                HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
                HGDIOBJ oldBrush = SelectObject(pDIS->hDC, btnBrush);
                HGDIOBJ oldPen = SelectObject(pDIS->hDC, pen);

                // Draw rounded rectangle button
                RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right, pDIS->rcItem.bottom, 8, 8);

                SelectObject(pDIS->hDC, oldBrush);
                SelectObject(pDIS->hDC, oldPen);
                DeleteObject(btnBrush);
                DeleteObject(pen);

                // Draw button text
                wchar_t btnText[64];
                GetWindowTextW(pDIS->hwndItem, btnText, 64);
                SetBkMode(pDIS->hDC, TRANSPARENT);
                SetTextColor(pDIS->hDC, textColor);
                SelectObject(pDIS->hDC, g_hFontBtn);
                DrawTextW(pDIS->hDC, btnText, -1, &pDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
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
            SetTimer(hWnd, 2, 400, NULL);
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            FillRect(hdc, &clientRect, g_hBgBrush);
            SetBkMode(hdc, TRANSPARENT);

            // App Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, RGB(248, 250, 252));
            TextOutW(hdc, 35, 20, L"DustDPI", 7);

            // Subtitle
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(96, 165, 250));
            TextOutW(hdc, 140, 27, L"Secici Ag Optimizasyon ve Servis Yoneticisi", 42);

            // Explanation / Scope
            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutW(hdc, 35, 52, L"Discord, Roblox ve hedef istemciler icin baglanti optimizasyonu saglar.", 68);

            // Status Card (Panel: 35, 82, 480, 162)
            RECT cardRect = { 35, 82, 480, 162 };
            HBRUSH cardBrd = CreateSolidBrush(RGB(35, 47, 72));
            HPEN cardPen = CreatePen(PS_SOLID, 1, RGB(35, 47, 72));
            HGDIOBJ oldB = SelectObject(hdc, g_hCardBrush);
            HGDIOBJ oldP = SelectObject(hdc, cardPen);
            RoundRect(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom, 10, 10);
            SelectObject(hdc, oldB);
            SelectObject(hdc, oldP);
            DeleteObject(cardBrd);
            DeleteObject(cardPen);

            // Status Badge / Indicator
            SelectObject(hdc, g_hFontStatus);
            if (g_curState == STATE_RUNNING) {
                SetTextColor(hdc, RGB(52, 211, 153)); // Emerald green
                TextOutW(hdc, 55, 96, L"[ DEVREDE ] - DustDPI Servisi Calisiyor", 38);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 126, L"Secici ag optimizasyonu ve yerel DNS yonlendirmesi devrede.", 58);
            } else if (g_curState == STATE_STOPPED) {
                SetTextColor(hdc, RGB(248, 113, 113)); // Red
                TextOutW(hdc, 55, 96, L"[ DURDURULDU ] - Servis Kapali", 29);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 126, L"Servis su an pasif durumda. 'Servisi Baslat' tusuna basin.", 57);
            } else if (g_curState == STATE_NOT_INSTALLED) {
                SetTextColor(hdc, RGB(251, 191, 36)); // Amber
                TextOutW(hdc, 55, 96, L"[ KURULU DEGIL ] - Servis Kaydi Bulunamadi", 41);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 126, L"Sisteme kaydetmek icin 'Servisi Onar / Kur' tusuna basin.", 56);
            } else {
                SetTextColor(hdc, RGB(192, 132, 252));
                TextOutW(hdc, 55, 96, L"[ ISLEM YAPILIYOR... ]", 22);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 126, L"Servis durumu degistiriliyor, lutfen bekleyin...", 48);
            }

            // Footer
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(100, 116, 139));
            TextOutW(hdc, 35, 360, L"Dust Studio  |  dust-studio.com  |  Sifir Yan Etki ve Gecikmesiz Gecis", 68);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenuW(hWnd);
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
            return DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        case WM_DESTROY: {
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
            KillTimer(hWnd, 1);
            if (g_hBgBrush) DeleteObject(g_hBgBrush);
            if (g_hCardBrush) DeleteObject(g_hCardBrush);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontSub) DeleteObject(g_hFontSub);
            if (g_hFontStatus) DeleteObject(g_hFontStatus);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBtn) DeleteObject(g_hFontBtn);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // Single instance check
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"DustStudio_DustDPI_GUI_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND existing = FindWindowW(L"DustDPIGUIClass", NULL);
        if (existing) {
            ShowWindow(existing, SW_RESTORE);
            SetForegroundWindow(existing);
        }
        return 0;
    }

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DustDPIGUIClass";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(1));
    wc.hbrBackground = NULL; // Handled in WM_PAINT to prevent flicker
    RegisterClassExW(&wc);

    int w = 530;
    int h = 430;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND hWnd = CreateWindowExW(
        0, L"DustDPIGUIClass",
        L"DustDPI - Servis ve Ag Yoneticisi",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, w, h,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (hMutex) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }
    return (int)msg.wParam;
}
