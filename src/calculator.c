#include "calculator.h"

#include <wchar.h>
#include <stdio.h>

static void Calculator_SetDisplayFromDouble(Calculator *calc, double value)
{
    swprintf(calc->display, ARRAYSIZE(calc->display), L"%.15g", value);
}

static void Calculator_SetDisplay(Calculator *calc, const wchar_t *text)
{
    wcscpy_s(calc->display, ARRAYSIZE(calc->display), text);
}

static double Calculator_GetDisplayValue(const Calculator *calc)
{
    return wcstod(calc->display, NULL);
}

static void Calculator_BeginFreshEntryIfNeeded(Calculator *calc)
{
    if (calc->enteringNew && calc->lastWasEquals && !calc->hasPending) {
        calc->pendingOp = CALC_OPERATOR_NONE;
        calc->accumulator = 0.0;
        calc->lastOperand = 0.0;
        calc->lastWasEquals = FALSE;
    }
}

static CalculatorResult Calculator_PerformOperation(CalculatorOperator op, double left, double right, double *result)
{
    switch (op) {
        case CALC_OPERATOR_ADD:
            *result = left + right;
            return CALC_OK;
        case CALC_OPERATOR_SUBTRACT:
            *result = left - right;
            return CALC_OK;
        case CALC_OPERATOR_MULTIPLY:
            *result = left * right;
            return CALC_OK;
        case CALC_OPERATOR_DIVIDE:
            if (right == 0.0) {
                return CALC_ERROR_DIVIDE_BY_ZERO;
            }
            *result = left / right;
            return CALC_OK;
        default:
            return CALC_OK;
    }
}

void Calculator_Init(Calculator *calc)
{
    if (!calc) {
        return;
    }

    calc->accumulator = 0.0;
    calc->lastOperand = 0.0;
    calc->pendingOp = CALC_OPERATOR_NONE;
    calc->hasPending = FALSE;
    calc->enteringNew = TRUE;
    calc->lastWasEquals = FALSE;
    Calculator_SetDisplay(calc, L"0");
}

void Calculator_Clear(Calculator *calc)
{
    Calculator_Init(calc);
}

void Calculator_InputDigit(Calculator *calc, int digit)
{
    if (!calc || digit < 0 || digit > 9) {
        return;
    }

    Calculator_BeginFreshEntryIfNeeded(calc);

    if (calc->enteringNew) {
        swprintf(calc->display, ARRAYSIZE(calc->display), L"%d", digit);
        calc->enteringNew = FALSE;
    } else {
        size_t length = wcslen(calc->display);
        if (length == 1 && calc->display[0] == L'0' && wcschr(calc->display, L'.') == NULL) {
            calc->display[0] = (wchar_t)(L'0' + digit);
            calc->display[1] = L'\0';
        } else if (length < ARRAYSIZE(calc->display) - 1) {
            calc->display[length] = (wchar_t)(L'0' + digit);
            calc->display[length + 1] = L'\0';
        }
    }

    calc->lastWasEquals = FALSE;
}

void Calculator_InputDecimal(Calculator *calc)
{
    if (!calc) {
        return;
    }

    Calculator_BeginFreshEntryIfNeeded(calc);

    if (calc->enteringNew) {
        Calculator_SetDisplay(calc, L"0.");
        calc->enteringNew = FALSE;
    } else if (wcschr(calc->display, L'.') == NULL) {
        size_t length = wcslen(calc->display);
        if (length < ARRAYSIZE(calc->display) - 1) {
            calc->display[length] = L'.';
            calc->display[length + 1] = L'\0';
        }
    }

    calc->lastWasEquals = FALSE;
}

CalculatorResult Calculator_ApplyOperator(Calculator *calc, CalculatorOperator op)
{
    if (!calc || op == CALC_OPERATOR_NONE) {
        return CALC_OK;
    }

    double current = Calculator_GetDisplayValue(calc);

    if (!calc->hasPending) {
        calc->accumulator = current;
        calc->hasPending = TRUE;
    } else {
        double result = 0.0;
        CalculatorResult status = Calculator_PerformOperation(calc->pendingOp, calc->accumulator, current, &result);
        if (status != CALC_OK) {
            return status;
        }
        calc->accumulator = result;
        Calculator_SetDisplayFromDouble(calc, result);
    }

    calc->pendingOp = op;
    calc->enteringNew = TRUE;
    calc->lastWasEquals = FALSE;
    return CALC_OK;
}

CalculatorResult Calculator_ApplyEquals(Calculator *calc)
{
    if (!calc) {
        return CALC_OK;
    }

    double current = Calculator_GetDisplayValue(calc);

    if (calc->pendingOp == CALC_OPERATOR_NONE) {
        if (!calc->lastWasEquals) {
            calc->accumulator = current;
            calc->lastOperand = current;
        }
        Calculator_SetDisplayFromDouble(calc, current);
        calc->enteringNew = TRUE;
        calc->lastWasEquals = TRUE;
        calc->hasPending = FALSE;
        return CALC_OK;
    }

    if (calc->lastWasEquals) {
        current = calc->lastOperand;
    } else {
        calc->lastOperand = current;
    }

    double result = 0.0;
    CalculatorResult status = Calculator_PerformOperation(calc->pendingOp, calc->accumulator, current, &result);
    if (status != CALC_OK) {
        return status;
    }

    calc->accumulator = result;
    Calculator_SetDisplayFromDouble(calc, result);
    calc->enteringNew = TRUE;
    calc->lastWasEquals = TRUE;
    calc->hasPending = FALSE;
    return CALC_OK;
}

const wchar_t *Calculator_GetDisplay(const Calculator *calc)
{
    return calc ? calc->display : L"";
}
