#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <wchar.h>
#include <stdio.h>

// Button identifiers for the on-screen keypad.
#define IDC_BTN_0       2000
#define IDC_BTN_1       2001
#define IDC_BTN_2       2002
#define IDC_BTN_3       2003
#define IDC_BTN_4       2004
#define IDC_BTN_5       2005
#define IDC_BTN_6       2006
#define IDC_BTN_7       2007
#define IDC_BTN_8       2008
#define IDC_BTN_9       2009
#define IDC_BTN_DECIMAL 2010
#define IDC_BTN_CLEAR   2011
#define IDC_BTN_EQUAL   2012
#define IDC_BTN_ADD     2013
#define IDC_BTN_SUBTRACT 2014
#define IDC_BTN_MULTIPLY 2015
#define IDC_BTN_DIVIDE   2016

// Calculator state that persists while the window lives.
typedef struct CalculatorState {
    HWND  display;        // Handle to the read-only display control.
    double accumulator;   // Stored value waiting for the next operand.
    double lastOperand;   // Operand used during the most recent equals.
    int    pendingOp;     // Which operator should run next.
    BOOL   hasPending;    // TRUE if accumulator holds a value awaiting another operand.
    BOOL   enteringNew;   // TRUE when the next digit should start a brand-new entry.
    BOOL   lastWasEquals; // Tracks if '=' was the last button pressed.
} CalculatorState;

static CalculatorState g_calc = { 0 };

static void SetDisplayText(const wchar_t *text)
{
    if (g_calc.display) {
        SetWindowTextW(g_calc.display, text);
    }
}

static void SetDisplayFromDouble(double value)
{
    wchar_t buffer[64];
    swprintf(buffer, ARRAYSIZE(buffer), L"%.15g", value);
    SetDisplayText(buffer);
}

static double GetDisplayValue(void)
{
    wchar_t buffer[64];
    GetWindowTextW(g_calc.display, buffer, ARRAYSIZE(buffer));
    return wcstod(buffer, NULL);
}

static void ResetCalculator(HWND hwnd)
{
    (void)hwnd;
    g_calc.accumulator = 0.0;
    g_calc.lastOperand = 0.0;
    g_calc.pendingOp = 0;
    g_calc.hasPending = FALSE;
    g_calc.enteringNew = TRUE;
    g_calc.lastWasEquals = FALSE;
    SetDisplayText(L"0");
}

static void BeginFreshEntryIfNeeded(void)
{
    if (g_calc.enteringNew && g_calc.lastWasEquals && !g_calc.hasPending) {
        g_calc.pendingOp = 0;
        g_calc.accumulator = 0.0;
        g_calc.lastOperand = 0.0;
        g_calc.lastWasEquals = FALSE;
    }
}

static void AppendDigit(int digit)
{
    BeginFreshEntryIfNeeded();
    wchar_t buffer[64];
    GetWindowTextW(g_calc.display, buffer, ARRAYSIZE(buffer));

    if (g_calc.enteringNew) {
        swprintf(buffer, ARRAYSIZE(buffer), L"%d", digit);
        g_calc.enteringNew = FALSE;
    } else {
        size_t length = wcslen(buffer);
        if (length == 1 && buffer[0] == L'0' && wcschr(buffer, L'.') == NULL) {
            buffer[0] = (wchar_t)(L'0' + digit);
            buffer[1] = L'\0';
        } else if (length < ARRAYSIZE(buffer) - 1) {
            buffer[length] = (wchar_t)(L'0' + digit);
            buffer[length + 1] = L'\0';
        }
    }

    SetDisplayText(buffer);
}

static void AppendDecimalPoint(void)
{
    BeginFreshEntryIfNeeded();
    wchar_t buffer[64];
    GetWindowTextW(g_calc.display, buffer, ARRAYSIZE(buffer));

    if (g_calc.enteringNew) {
        wcscpy_s(buffer, ARRAYSIZE(buffer), L"0.");
        g_calc.enteringNew = FALSE;
    } else if (wcschr(buffer, L'.') == NULL) {
        size_t length = wcslen(buffer);
        if (length < ARRAYSIZE(buffer) - 1) {
            buffer[length] = L'.';
            buffer[length + 1] = L'\0';
        }
    }

    SetDisplayText(buffer);
}

static BOOL ApplyOperation(HWND hwndDlg, int op, double left, double right, double *result)
{
    switch (op) {
        case IDC_BTN_ADD:
            *result = left + right;
            return TRUE;
        case IDC_BTN_SUBTRACT:
            *result = left - right;
            return TRUE;
        case IDC_BTN_MULTIPLY:
            *result = left * right;
            return TRUE;
        case IDC_BTN_DIVIDE:
            if (right == 0.0) {
                MessageBoxW(hwndDlg, L"Cannot divide by zero.", L"Input error", MB_OK | MB_ICONERROR);
                return FALSE;
            }
            *result = left / right;
            return TRUE;
        default:
            return FALSE;
    }
}

static void HandleOperator(HWND hwndDlg, int operatorId)
{
    double current = GetDisplayValue();

    if (!g_calc.hasPending) {
        g_calc.accumulator = current;
        g_calc.hasPending = TRUE;
    } else {
        double result = 0.0;
        if (!ApplyOperation(hwndDlg, g_calc.pendingOp, g_calc.accumulator, current, &result)) {
            ResetCalculator(hwndDlg);
            return;
        }
        g_calc.accumulator = result;
        SetDisplayFromDouble(result);
    }

    g_calc.pendingOp = operatorId;
    g_calc.enteringNew = TRUE;
    g_calc.lastWasEquals = FALSE;
}

static void HandleEquals(HWND hwndDlg)
{
    if (g_calc.pendingOp == 0 && !g_calc.lastWasEquals) {
        return; // Nothing to evaluate yet.
    }

    double current = 0.0;
    if (g_calc.lastWasEquals) {
        current = g_calc.lastOperand;
    } else {
        current = GetDisplayValue();
        g_calc.lastOperand = current;
    }

    double result = 0.0;
    if (!ApplyOperation(hwndDlg, g_calc.pendingOp, g_calc.accumulator, current, &result)) {
        ResetCalculator(hwndDlg);
        return;
    }

    g_calc.accumulator = result;
    SetDisplayFromDouble(result);
    g_calc.enteringNew = TRUE;
    g_calc.lastWasEquals = TRUE;
    g_calc.hasPending = FALSE;
}

static void HandleClear(HWND hwndDlg)
{
    ResetCalculator(hwndDlg);
}

// Creates the calculator UI controls when the window first opens.
static void InitializeControls(HWND hwnd)
{
    const int marginX = 20;
    const int marginY = 20;
    const int btnWidth = 50;
    const int btnHeight = 40;
    const int spacing = 10;
    const int displayWidth = btnWidth * 4 + spacing * 3;

    g_calc.display = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0",
                                     WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY,
                                     marginX, marginY, displayWidth, 40,
                                     hwnd, NULL, NULL, NULL);

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(g_calc.display, WM_SETFONT, (WPARAM)font, TRUE);

    struct ButtonDef {
        int id;
        const wchar_t *text;
        int row;
        int col;
        int colSpan;
    };

    static const struct ButtonDef buttons[] = {
        { IDC_BTN_CLEAR,    L"C", 0, 0, 2 },
        { IDC_BTN_DIVIDE,   L"/", 0, 3, 1 },
        { IDC_BTN_7,        L"7", 1, 0, 1 },
        { IDC_BTN_8,        L"8", 1, 1, 1 },
        { IDC_BTN_9,        L"9", 1, 2, 1 },
        { IDC_BTN_MULTIPLY, L"x", 1, 3, 1 },
        { IDC_BTN_4,        L"4", 2, 0, 1 },
        { IDC_BTN_5,        L"5", 2, 1, 1 },
        { IDC_BTN_6,        L"6", 2, 2, 1 },
        { IDC_BTN_SUBTRACT, L"-", 2, 3, 1 },
        { IDC_BTN_1,        L"1", 3, 0, 1 },
        { IDC_BTN_2,        L"2", 3, 1, 1 },
        { IDC_BTN_3,        L"3", 3, 2, 1 },
        { IDC_BTN_ADD,      L"+", 3, 3, 1 },
        { IDC_BTN_0,        L"0", 4, 0, 2 },
        { IDC_BTN_DECIMAL,  L".", 4, 2, 1 },
        { IDC_BTN_EQUAL,    L"=", 4, 3, 1 }
    };

    const int startY = marginY + 50;

    for (size_t i = 0; i < ARRAYSIZE(buttons); ++i) {
        int x = marginX + buttons[i].col * (btnWidth + spacing);
        int y = startY + buttons[i].row * (btnHeight + spacing);
        int width = btnWidth * buttons[i].colSpan + spacing * (buttons[i].colSpan - 1);

        HWND hBtn = CreateWindowExW(0, L"BUTTON", buttons[i].text,
                                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                    x, y, width, btnHeight,
                                    hwnd, (HMENU)(INT_PTR)buttons[i].id, NULL, NULL);
        SendMessageW(hBtn, WM_SETFONT, (WPARAM)font, TRUE);
    }

    ResetCalculator(hwnd);
}

// The window procedure receives every message sent to our window.
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_CREATE:
            InitializeControls(hwnd);
            return 0;
        case WM_COMMAND: {
            int controlId = LOWORD(wParam);

            if (controlId >= IDC_BTN_0 && controlId <= IDC_BTN_9) {
                AppendDigit(controlId - IDC_BTN_0);
                return 0;
            }

            switch (controlId) {
                case IDC_BTN_DECIMAL:
                    AppendDecimalPoint();
                    return 0;
                case IDC_BTN_ADD:
                case IDC_BTN_SUBTRACT:
                case IDC_BTN_MULTIPLY:
                case IDC_BTN_DIVIDE:
                    HandleOperator(hwnd, controlId);
                    return 0;
                case IDC_BTN_EQUAL:
                    HandleEquals(hwnd);
                    return 0;
                case IDC_BTN_CLEAR:
                    HandleClear(hwnd);
                    return 0;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    const wchar_t CLASS_NAME[] = L"FourFunctionCalc";

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Failed to register window class.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Win32 Calculator",
                                WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 300, 360,
                                NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxW(NULL, L"Failed to create window.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}
