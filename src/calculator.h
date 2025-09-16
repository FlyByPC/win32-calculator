#pragma once

#include <windows.h>

typedef enum CalculatorOperator {
    CALC_OPERATOR_NONE = 0,
    CALC_OPERATOR_ADD,
    CALC_OPERATOR_SUBTRACT,
    CALC_OPERATOR_MULTIPLY,
    CALC_OPERATOR_DIVIDE
} CalculatorOperator;

typedef enum CalculatorResult {
    CALC_OK = 0,
    CALC_ERROR_DIVIDE_BY_ZERO
} CalculatorResult;

typedef struct Calculator {
    double accumulator;
    double lastOperand;
    CalculatorOperator pendingOp;
    BOOL hasPending;
    BOOL enteringNew;
    BOOL lastWasEquals;
    wchar_t display[64];
} Calculator;

void Calculator_Init(Calculator *calc);
void Calculator_Clear(Calculator *calc);
void Calculator_InputDigit(Calculator *calc, int digit);
void Calculator_InputDecimal(Calculator *calc);
CalculatorResult Calculator_ApplyOperator(Calculator *calc, CalculatorOperator op);
CalculatorResult Calculator_ApplyEquals(Calculator *calc);
const wchar_t *Calculator_GetDisplay(const Calculator *calc);
