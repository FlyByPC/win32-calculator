#include "ui.h"

#include "calculator.h"
#include "resource.h"
#include <stddef.h>
#include <wchar.h>
typedef struct CalculatorUI {
    HWND display;
    Calculator calculator;
    HFONT font;
} CalculatorUI;

static CalculatorUI g_ui;

static void Ui_UpdateDisplay(void)
{
    SetWindowTextW(g_ui.display, Calculator_GetDisplay(&g_ui.calculator));
}

static CalculatorOperator Ui_OperatorFromId(int controlId)
{
    switch (controlId) {
        case IDC_BTN_ADD:
            return CALC_OPERATOR_ADD;
        case IDC_BTN_SUBTRACT:
            return CALC_OPERATOR_SUBTRACT;
        case IDC_BTN_MULTIPLY:
            return CALC_OPERATOR_MULTIPLY;
        case IDC_BTN_DIVIDE:
            return CALC_OPERATOR_DIVIDE;
        default:
            return CALC_OPERATOR_NONE;
    }
}

static HWND Ui_CreateButton(HWND parent, int id, const wchar_t *text,
                            int x, int y, int width, int height)
{
    HWND hBtn = CreateWindowExW(0, L"BUTTON", text,
                                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                x, y, width, height,
                                parent, (HMENU)(INT_PTR)id, NULL, NULL);
    SendMessageW(hBtn, WM_SETFONT, (WPARAM)g_ui.font, TRUE);
    return hBtn;
}

BOOL Ui_OnCreate(HWND hwnd)
{
    g_ui.font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    Calculator_Init(&g_ui.calculator);

    const int marginX = 20;
    const int marginY = 20;
    const int btnWidth = 50;
    const int btnHeight = 40;
    const int spacing = 10;
    const int displayWidth = btnWidth * 4 + spacing * 3;

    g_ui.display = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0",
                                   WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY,
                                   marginX, marginY, displayWidth, 40,
                                   hwnd, NULL, NULL, NULL);
    if (!g_ui.display) {
        return FALSE;
    }

    SendMessageW(g_ui.display, WM_SETFONT, (WPARAM)g_ui.font, TRUE);

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

        Ui_CreateButton(hwnd, buttons[i].id, buttons[i].text, x, y, width, btnHeight);
    }

    Ui_UpdateDisplay();
    return TRUE;
}

static void Ui_HandleError(HWND hwnd, CalculatorResult result)
{
    if (result == CALC_ERROR_DIVIDE_BY_ZERO) {
        MessageBoxW(hwnd, L"Cannot divide by zero.", L"Input error", MB_OK | MB_ICONERROR);
        Calculator_Clear(&g_ui.calculator);
    }
}

BOOL Ui_OnCommand(HWND hwnd, int controlId)
{
    if (controlId >= IDC_BTN_0 && controlId <= IDC_BTN_9) {
        Calculator_InputDigit(&g_ui.calculator, controlId - IDC_BTN_0);
        Ui_UpdateDisplay();
        return TRUE;
    }

    CalculatorResult result = CALC_OK;

    switch (controlId) {
        case IDC_BTN_DECIMAL:
            Calculator_InputDecimal(&g_ui.calculator);
            break;
        case IDC_BTN_CLEAR:
            Calculator_Clear(&g_ui.calculator);
            break;
        case IDC_BTN_ADD:
        case IDC_BTN_SUBTRACT:
        case IDC_BTN_MULTIPLY:
        case IDC_BTN_DIVIDE:
            result = Calculator_ApplyOperator(&g_ui.calculator, Ui_OperatorFromId(controlId));
            break;
        case IDC_BTN_EQUAL:
            result = Calculator_ApplyEquals(&g_ui.calculator);
            break;
        default:
            return FALSE;
    }

    Ui_HandleError(hwnd, result);
    Ui_UpdateDisplay();
    return TRUE;
}



