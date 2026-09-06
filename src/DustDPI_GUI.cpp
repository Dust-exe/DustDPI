#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_SHOW    2001
#define ID_TRAY_START   2002
#define ID_TRAY_STOP    2003
#define ID_TRAY_FIX     2004
#define ID_TRAY_FILTER  2005
#define ID_TRAY_EXIT    2006

#define IDC_BTN_START      101
#define IDC_BTN_STOP       102
#define IDC_BTN_FILTER     103
#define IDC_BTN_DIAG       104
#define IDC_BTN_TOGGLEMODE 105
#define IDC_BTN_INSTALL    106

// Filter Dialog Control IDs
#define IDC_FILTER_LIST    301
#define IDC_FILTER_EDIT    302
#define IDC_FILTER_ADD     303
#define IDC_FILTER_REMOVE  304
#define IDC_FILTER_NOTEPAD 305
#define IDC_FILTER_SAVE    306
#define IDC_FILTER_CLOSE   307

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
static HWND g_hFilterWnd = NULL;
static HBRUSH g_hBgBrush = NULL;
static HBRUSH g_hCardBrush = NULL;
static HBRUSH g_hInputBrush = NULL;
static HFONT g_hFontTitle = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontStatus = NULL;
static HFONT g_hFontSub = NULL;
static HFONT g_hFontBtn = NULL;
static HFONT g_hFontBadge = NULL;
static HICON g_hAppIcon = NULL;

static ServiceState g_curState = STATE_UNKNOWN;
static wchar_t g_appDir[MAX_PATH] = {0};
static int g_hoverBtnId = 0;
static bool g_fullMode = false;
static int g_activeDomainCount = 0;

std::wstring GetAppDirectoryW() {
    if (g_appDir[0] == L'\0') {
        GetModuleFileNameW(NULL, g_appDir, MAX_PATH);
        wchar_t* lastSlash = wcsrchr(g_appDir, L'\\');
        if (lastSlash) *lastSlash = L'\0';
    }
    return std::wstring(g_appDir);
}

int CountBlacklistDomains() {
    std::wstring blPath = GetAppDirectoryW() + L"\\blacklist.txt";
    std::ifstream file(std::string(blPath.begin(), blPath.end()));
    if (!file.is_open()) return 0;
    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        // trim whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        if (line[first] == '#') continue;
        count++;
    }
    return count;
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

void ActionFixNetwork() {
    std::wstring dir = GetAppDirectoryW();
    std::wstring cmd = dir + L"\\network_diagnostics_and_repair.cmd";
    // Check if network_diagnostics_and_repair.cmd exists, else fallback to discord_fix_and_start.cmd
    DWORD attr = GetFileAttributesW(cmd.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        cmd = dir + L"\\discord_fix_and_start.cmd";
    }
    ShellExecuteW(NULL, L"runas", cmd.c_str(), NULL, dir.c_str(), SW_SHOWNORMAL);
}

void ActionInstallService(bool fullMode = false) {
    std::wstring dir = GetAppDirectoryW();
    std::wstring exe64 = dir + L"\\x86_64\\dust_engine.exe";
    std::wstring bl = dir + L"\\blacklist.txt";
    std::wstring cmd = L"sc.exe stop \"DustDPI\" & sc.exe delete \"DustDPI\" & ";
    if (fullMode) {
        cmd += L"sc.exe create \"DustDPI\" binPath= \"\\\"" + exe64 + L"\\\" -5 --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253\" start= auto DisplayName= \"DustDPI Service\" & ";
    } else {
        cmd += L"sc.exe create \"DustDPI\" binPath= \"\\\"" + exe64 + L"\\\" -5 --set-ttl 5 --dns-addr 77.88.8.8 --dns-port 1253 --dnsv6-addr 2a02:6b8::feed:0ff --dnsv6-port 1253 --allow-no-sni --blacklist \\\"" + bl + L"\\\"\" start= auto DisplayName= \"DustDPI Service\" & ";
    }
    cmd += L"sc.exe description \"DustDPI\" \"Dust Studio High-Performance Internet Freedom & Selective Traffic Optimization Service\" & ";
    cmd += L"sc.exe start \"DustDPI\"";
    RunCmdAsyncW(cmd, true);
}

void ActionToggleMode() {
    g_fullMode = !g_fullMode;
    ActionInstallService(g_fullMode);
    HWND hBtn = GetDlgItem(g_hMainWnd, IDC_BTN_TOGGLEMODE);
    if (hBtn) {
        SetWindowTextW(hBtn, g_fullMode ? L"Mode: Full Optimization" : L"Mode: Selective (Filter)");
        InvalidateRect(hBtn, NULL, FALSE);
    }
    InvalidateRect(g_hMainWnd, NULL, FALSE);
}

void ActionOpenNotepad() {
    std::wstring bl = GetAppDirectoryW() + L"\\blacklist.txt";
    ShellExecuteW(NULL, L"open", L"notepad.exe", bl.c_str(), NULL, SW_SHOW);
}

void SetupTrayW(HWND hWnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = g_hAppIcon ? g_hAppIcon : LoadIconW(NULL, IDI_APPLICATION);
    wcscpy(g_nid.szTip, L"DustDPI - Internet Freedom & Traffic Engine");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

void ShowTrayMenuW(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_SHOW, L"Open DustDPI Dashboard");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_START, L"Start Service");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_STOP, L"Stop Service");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_FIX, L"Run Network Diagnostic & Fix");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_FILTER, L"Manage Target Filter...");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

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

// -------------------------------------------------------------
// Target Filter Manager Window Procedure
// -------------------------------------------------------------
void PopulateFilterList(HWND hList) {
    SendMessageW(hList, LB_RESETCONTENT, 0, 0);
    std::wstring blPath = GetAppDirectoryW() + L"\\blacklist.txt";
    std::ifstream file(std::string(blPath.begin(), blPath.end()));
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        std::string clean = line.substr(first, (last - first + 1));
        if (clean.empty() || clean[0] == '#') continue;

        std::wstring wClean(clean.begin(), clean.end());
        SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)wClean.c_str());
    }
}

void SaveFilterListToFile(HWND hList) {
    int count = (int)SendMessageW(hList, LB_GETCOUNT, 0, 0);
    std::vector<std::string> domains;

    for (int i = 0; i < count; i++) {
        wchar_t buf[256] = {0};
        SendMessageW(hList, LB_GETTEXT, i, (LPARAM)buf);
        std::wstring ws(buf);
        std::string s(ws.begin(), ws.end());
        if (!s.empty()) {
            domains.push_back(s);
        }
    }

    // Sort and remove duplicates
    std::sort(domains.begin(), domains.end());
    domains.erase(std::unique(domains.begin(), domains.end()), domains.end());

    std::wstring blPath = GetAppDirectoryW() + L"\\blacklist.txt";
    std::ofstream file(std::string(blPath.begin(), blPath.end()), std::ios::trunc);
    if (file.is_open()) {
        for (const auto& d : domains) {
            file << d << "\n";
        }
        file.close();
    }

    g_activeDomainCount = (int)domains.size();
    if (g_hMainWnd) {
        InvalidateRect(g_hMainWnd, NULL, FALSE);
    }

    // If service is running, restart it to immediately apply changes
    if (QueryDpiService() == STATE_RUNNING) {
        RunCmdAsyncW(L"sc.exe stop \"DustDPI\" & sc.exe start \"DustDPI\"", true);
    }
}

std::wstring SanitizeDomainInput(const std::wstring& in) {
    std::wstring s = in;
    // Trim spaces
    size_t first = s.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos) return L"";
    size_t last = s.find_last_not_of(L" \t\r\n");
    s = s.substr(first, (last - first + 1));

    // Strip http:// or https://
    const std::wstring httpPrefix = L"http://";
    const std::wstring httpsPrefix = L"https://";
    if (s.rfind(httpPrefix, 0) == 0) s = s.substr(httpPrefix.length());
    else if (s.rfind(httpsPrefix, 0) == 0) s = s.substr(httpsPrefix.length());

    // Strip trailing slashes or paths
    size_t slash = s.find(L'/');
    if (slash != std::wstring::npos) s = s.substr(0, slash);

    // Convert to lowercase
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    return s;
}

LRESULT CALLBACK FilterWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

            // ListBox
            HWND hList = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | LBS_NOTIFY | LBS_HASSTRINGS,
                30, 75, 455, 230,
                hWnd, (HMENU)IDC_FILTER_LIST, GetModuleHandleW(NULL), NULL
            );
            SendMessageW(hList, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Edit box
            HWND hEdit = CreateWindowExW(
                WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                30, 335, 320, 34,
                hWnd, (HMENU)IDC_FILTER_EDIT, GetModuleHandleW(NULL), NULL
            );
            SendMessageW(hEdit, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);

            // Add button
            CreateWindowExW(
                0, L"BUTTON", L"+ Add Domain",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                365, 335, 120, 34,
                hWnd, (HMENU)IDC_FILTER_ADD, GetModuleHandleW(NULL), NULL
            );

            // Remove button
            CreateWindowExW(
                0, L"BUTTON", L"- Remove Selected",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                30, 385, 155, 36,
                hWnd, (HMENU)IDC_FILTER_REMOVE, GetModuleHandleW(NULL), NULL
            );

            // Notepad button
            CreateWindowExW(
                0, L"BUTTON", L"Open in Notepad",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                200, 385, 145, 36,
                hWnd, (HMENU)IDC_FILTER_NOTEPAD, GetModuleHandleW(NULL), NULL
            );

            // Close button
            CreateWindowExW(
                0, L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                360, 385, 125, 36,
                hWnd, (HMENU)IDC_FILTER_CLOSE, GetModuleHandleW(NULL), NULL
            );

            // Save & Apply Button
            CreateWindowExW(
                0, L"BUTTON", L"Save & Apply Filter Changes",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                30, 435, 455, 42,
                hWnd, (HMENU)IDC_FILTER_SAVE, GetModuleHandleW(NULL), NULL
            );

            PopulateFilterList(hList);
            break;
        }

        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(19, 27, 46));
            SetTextColor(hdc, RGB(241, 245, 249));
            return (LRESULT)g_hCardBrush;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(19, 27, 46));
            SetTextColor(hdc, RGB(248, 250, 252));
            return (LRESULT)g_hCardBrush;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
            if (pDIS->CtlType == ODT_BUTTON) {
                bool isPressed = (pDIS->itemState & ODS_SELECTED);
                COLORREF bgColor = RGB(30, 41, 59);
                COLORREF borderColor = RGB(51, 65, 85);
                COLORREF textColor = RGB(241, 245, 249);

                if (pDIS->CtlID == IDC_FILTER_SAVE) {
                    // Electric Cyan Primary
                    bgColor = isPressed ? RGB(2, 132, 199) : RGB(3, 105, 161);
                    borderColor = RGB(56, 189, 248);
                    textColor = RGB(255, 255, 255);
                } else if (pDIS->CtlID == IDC_FILTER_ADD) {
                    bgColor = isPressed ? RGB(16, 75, 50) : RGB(20, 83, 45);
                    borderColor = RGB(34, 197, 94);
                    textColor = RGB(240, 253, 244);
                } else if (pDIS->CtlID == IDC_FILTER_REMOVE) {
                    bgColor = isPressed ? RGB(75, 25, 25) : RGB(69, 26, 26);
                    borderColor = RGB(220, 38, 38);
                    textColor = RGB(254, 242, 242);
                }

                HBRUSH b = CreateSolidBrush(bgColor);
                HPEN p = CreatePen(PS_SOLID, 1, borderColor);
                HGDIOBJ oldB = SelectObject(pDIS->hDC, b);
                HGDIOBJ oldP = SelectObject(pDIS->hDC, p);

                RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right, pDIS->rcItem.bottom, 6, 6);

                SelectObject(pDIS->hDC, oldB);
                SelectObject(pDIS->hDC, oldP);
                DeleteObject(b);
                DeleteObject(p);

                wchar_t text[64];
                GetWindowTextW(pDIS->hwndItem, text, 64);
                SetBkMode(pDIS->hDC, TRANSPARENT);
                SetTextColor(pDIS->hDC, textColor);
                SelectObject(pDIS->hDC, g_hFontBtn);
                DrawTextW(pDIS->hDC, text, -1, &pDIS->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            HWND hList = GetDlgItem(hWnd, IDC_FILTER_LIST);
            HWND hEdit = GetDlgItem(hWnd, IDC_FILTER_EDIT);

            switch (wmId) {
                case IDC_FILTER_ADD: {
                    wchar_t raw[256] = {0};
                    GetWindowTextW(hEdit, raw, 256);
                    std::wstring domain = SanitizeDomainInput(raw);
                    if (domain.empty()) {
                        MessageBoxW(hWnd, L"Please enter a valid domain name (e.g. reddit.com)", L"Input Error", MB_ICONWARNING);
                        break;
                    }
                    if (domain.find(L'.') == std::wstring::npos) {
                        MessageBoxW(hWnd, L"Domain must include an extension (e.g. .com, .net, .org)", L"Input Error", MB_ICONWARNING);
                        break;
                    }

                    // Check if already in list
                    LRESULT findIdx = SendMessageW(hList, LB_FINDSTRINGEXACT, -1, (LPARAM)domain.c_str());
                    if (findIdx != LB_ERR) {
                        MessageBoxW(hWnd, L"This domain is already in the filter list.", L"Duplicate Notice", MB_ICONINFORMATION);
                        SendMessageW(hList, LB_SETCURSEL, findIdx, 0);
                        break;
                    }

                    int newIdx = (int)SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)domain.c_str());
                    SendMessageW(hList, LB_SETCURSEL, newIdx, 0);
                    SetWindowTextW(hEdit, L"");
                    InvalidateRect(hWnd, NULL, FALSE);
                    break;
                }

                case IDC_FILTER_REMOVE: {
                    int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
                    if (sel == LB_ERR) {
                        MessageBoxW(hWnd, L"Please select a domain from the list to remove.", L"Notice", MB_ICONINFORMATION);
                        break;
                    }
                    SendMessageW(hList, LB_DELETESTRING, sel, 0);
                    InvalidateRect(hWnd, NULL, FALSE);
                    break;
                }

                case IDC_FILTER_NOTEPAD: {
                    ActionOpenNotepad();
                    break;
                }

                case IDC_FILTER_SAVE: {
                    SaveFilterListToFile(hList);
                    MessageBoxW(hWnd, L"Filter list saved and applied successfully!\nDustDPI service has refreshed routing targets.", L"Filter Saved", MB_ICONINFORMATION);
                    DestroyWindow(hWnd);
                    break;
                }

                case IDC_FILTER_CLOSE: {
                    DestroyWindow(hWnd);
                    break;
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, g_hBgBrush);
            SetBkMode(hdc, TRANSPARENT);

            // Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, RGB(248, 250, 252));
            TextOutW(hdc, 30, 16, L"Target Domain Filter", 20);

            // Subtitle
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutW(hdc, 30, 48, L"Add or remove domains and apps for selective packet optimization.", 66);

            // Input label
            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutW(hdc, 30, 314, L"Add New Domain / Hostname:", 26);

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_DESTROY: {
            g_hFilterWnd = NULL;
            break;
        }

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void OpenFilterManagerWindow(HWND hParent) {
    if (g_hFilterWnd) {
        SetForegroundWindow(g_hFilterWnd);
        return;
    }

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = FilterWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"DustDPI_FilterClass";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon = g_hAppIcon;
    RegisterClassExW(&wc);

    int w = 530;
    int h = 535;
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - w) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - h) / 2;

    g_hFilterWnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"DustDPI_FilterClass",
        L"Target Filter Manager — DustDPI",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, w, h,
        hParent, NULL, GetModuleHandleW(NULL), NULL
    );
}

// -------------------------------------------------------------
// Main Dashboard Window Procedure
// -------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hMainWnd = hWnd;
            
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

            // Load application icon (user cyan emblem)
            g_hAppIcon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1), IMAGE_ICON, 36, 36, LR_DEFAULTCOLOR);
            if (!g_hAppIcon) {
                g_hAppIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
            }

            g_hFontTitle = CreateFontW(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontBadge = CreateFontW(11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontSub = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontStatus = CreateFontW(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontNormal = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
            g_hFontBtn = CreateFontW(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

            // Cyberpunk Dark Obsidian Palette
            g_hBgBrush = CreateSolidBrush(RGB(11, 15, 25));       // #0b0f19
            g_hCardBrush = CreateSolidBrush(RGB(19, 27, 46));     // #131b2e
            g_hInputBrush = CreateSolidBrush(RGB(15, 23, 42));    // #0f172a

            struct BtnDef {
                int id;
                const wchar_t* text;
                int x, y, w, h;
            };

            BtnDef buttons[] = {
                { IDC_BTN_START,      L"Start Service",               35, 195, 235, 42 },
                { IDC_BTN_STOP,       L"Stop Service",                290, 195, 235, 42 },
                { IDC_BTN_FILTER,     L"Manage Target Filter...",     35, 248, 235, 42 },
                { IDC_BTN_DIAG,       L"Network Diagnostic & Fix",    290, 248, 235, 42 },
                { IDC_BTN_TOGGLEMODE, L"Mode: Selective (Filter)",    35, 301, 235, 42 },
                { IDC_BTN_INSTALL,    L"Reinstall / Repair Service",  290, 301, 235, 42 }
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
            g_activeDomainCount = CountBlacklistDomains();
            break;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
            if (pDIS->CtlType == ODT_BUTTON) {
                bool isHover = (g_hoverBtnId == (int)pDIS->CtlID);
                bool isPressed = (pDIS->itemState & ODS_SELECTED);
                
                COLORREF bgColor;
                COLORREF borderColor;
                COLORREF textColor = RGB(241, 245, 249);

                if (pDIS->CtlID == IDC_BTN_START) {
                    // Electric Cyan / Blue Primary
                    if (isPressed) {
                        bgColor = RGB(2, 132, 199);
                        borderColor = RGB(56, 189, 248);
                    } else if (isHover) {
                        bgColor = RGB(3, 105, 161);
                        borderColor = RGB(56, 189, 248);
                    } else {
                        bgColor = RGB(2, 132, 199);
                        borderColor = RGB(14, 165, 233);
                    }
                    textColor = RGB(255, 255, 255);
                } else if (pDIS->CtlID == IDC_BTN_STOP) {
                    // Subtle Crimson / Rose
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
                    textColor = RGB(254, 202, 202);
                } else if (pDIS->CtlID == IDC_BTN_FILTER) {
                    // Accent Cyber Cyan outline
                    if (isPressed) {
                        bgColor = RGB(15, 23, 42);
                        borderColor = RGB(56, 189, 248);
                    } else if (isHover) {
                        bgColor = RGB(22, 38, 65);
                        borderColor = RGB(0, 229, 255);
                    } else {
                        bgColor = RGB(19, 27, 46);
                        borderColor = RGB(56, 189, 248);
                    }
                    textColor = RGB(56, 189, 248);
                } else {
                    // Neutral Slate
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

                RoundRect(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, pDIS->rcItem.right, pDIS->rcItem.bottom, 8, 8);

                SelectObject(pDIS->hDC, oldBrush);
                SelectObject(pDIS->hDC, oldPen);
                DeleteObject(btnBrush);
                DeleteObject(pen);

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
            int newCount = CountBlacklistDomains();
            if (newState != g_curState || newCount != g_activeDomainCount) {
                g_curState = newState;
                g_activeDomainCount = newCount;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDC_BTN_START: ActionStartService(); break;
                case IDC_BTN_STOP: ActionStopService(); break;
                case IDC_BTN_FILTER: OpenFilterManagerWindow(hWnd); break;
                case IDC_BTN_DIAG: ActionFixNetwork(); break;
                case IDC_BTN_TOGGLEMODE: ActionToggleMode(); break;
                case IDC_BTN_INSTALL: ActionInstallService(g_fullMode); break;
                case ID_TRAY_SHOW:
                    ShowWindow(hWnd, SW_RESTORE);
                    SetForegroundWindow(hWnd);
                    break;
                case ID_TRAY_START: ActionStartService(); break;
                case ID_TRAY_STOP: ActionStopService(); break;
                case ID_TRAY_FIX: ActionFixNetwork(); break;
                case ID_TRAY_FILTER: OpenFilterManagerWindow(hWnd); break;
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

            // Draw Logo Emblem Icon (User Cyan Diamond & Wings)
            if (g_hAppIcon) {
                DrawIconEx(hdc, 35, 20, g_hAppIcon, 38, 38, 0, NULL, DI_NORMAL);
            }

            // Title
            SelectObject(hdc, g_hFontTitle);
            SetTextColor(hdc, RGB(248, 250, 252));
            TextOutW(hdc, 84, 16, L"DustDPI", 7);

            // Subtitle / Tagline
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(56, 189, 248));
            TextOutW(hdc, 195, 26, L"Next-Gen Internet Freedom & Traffic Engine", 43);

            // Description
            SelectObject(hdc, g_hFontNormal);
            SetTextColor(hdc, RGB(148, 163, 184));
            TextOutW(hdc, 35, 68, L"Driver-level network routing for unrestricted, ultra-low latency browsing and connectivity.", 91);

            // Status Card Panel (35, 96, 525, 178)
            RECT cardRect = { 35, 96, 525, 178 };
            HPEN cardPen = CreatePen(PS_SOLID, 1, RGB(30, 41, 59));
            HGDIOBJ oldB = SelectObject(hdc, g_hCardBrush);
            HGDIOBJ oldP = SelectObject(hdc, cardPen);
            RoundRect(hdc, cardRect.left, cardRect.top, cardRect.right, cardRect.bottom, 10, 10);
            SelectObject(hdc, oldB);
            SelectObject(hdc, oldP);
            DeleteObject(cardPen);

            // Status Badge & Info
            SelectObject(hdc, g_hFontStatus);
            wchar_t domainStatusBuf[128];
            wsprintfW(domainStatusBuf, L"Active Filter: %d monitored domains  |  Port 1253 Local DNS Redirection Active", g_activeDomainCount);

            if (g_curState == STATE_RUNNING) {
                SetTextColor(hdc, RGB(56, 189, 248));
                if (g_fullMode) {
                    TextOutW(hdc, 55, 110, L"[ ACTIVE ]  Global Traffic Optimization Enabled", 47);
                } else {
                    TextOutW(hdc, 55, 110, L"[ ACTIVE ]  Selective Traffic Optimization (Filter Active)", 58);
                }

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 140, domainStatusBuf, (int)wcslen(domainStatusBuf));
            } else if (g_curState == STATE_STOPPED) {
                SetTextColor(hdc, RGB(248, 113, 113));
                TextOutW(hdc, 55, 110, L"[ INACTIVE ]  Traffic Optimization Engine Paused", 48);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 140, L"Traffic flows directly without packet optimization. Click 'Start Service' to enable.", 85);
            } else if (g_curState == STATE_NOT_INSTALLED) {
                SetTextColor(hdc, RGB(251, 191, 36));
                TextOutW(hdc, 55, 110, L"[ NOT REGISTERED ]  DustDPI Service Not Found", 45);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 140, L"Click 'Reinstall / Repair Service' to register the driver and configure autostart.", 82);
            } else {
                SetTextColor(hdc, RGB(192, 132, 252));
                TextOutW(hdc, 55, 110, L"[ PROCESSING... ]  Updating Service State", 41);

                SelectObject(hdc, g_hFontSub);
                SetTextColor(hdc, RGB(148, 163, 184));
                TextOutW(hdc, 55, 140, L"Please wait while the Windows service state transitions...", 58);
            }

            // Footer
            SelectObject(hdc, g_hFontSub);
            SetTextColor(hdc, RGB(100, 116, 139));
            TextOutW(hdc, 35, 364, L"Dust Studio  |  Autonomous Traffic Routing Engine  |  Zero Latency & Collateral Freedom", 87);

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
            if (g_hInputBrush) DeleteObject(g_hInputBrush);
            if (g_hFontTitle) DeleteObject(g_hFontTitle);
            if (g_hFontBadge) DeleteObject(g_hFontBadge);
            if (g_hFontSub) DeleteObject(g_hFontSub);
            if (g_hFontStatus) DeleteObject(g_hFontStatus);
            if (g_hFontNormal) DeleteObject(g_hFontNormal);
            if (g_hFontBtn) DeleteObject(g_hFontBtn);
            if (g_hAppIcon) DestroyIcon(g_hAppIcon);
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
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
    wc.hbrBackground = NULL;
    RegisterClassExW(&wc);

    int w = 575;
    int h = 440;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

    HWND hWnd = CreateWindowExW(
        0, L"DustDPIGUIClass",
        L"DustDPI — Internet Freedom & Traffic Optimization Engine",
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
