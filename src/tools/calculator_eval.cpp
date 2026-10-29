
#include "tools/calculator.h"

#include <stack>

namespace {

using Kind = CalcToken::Kind;

// Return operator precedence.
int precedence(Kind op) {
    if (op == Kind::Times || op == Kind::Divide) {
        return 2;
    }

    if (op == Kind::Plus || op == Kind::Minus) {
        return 1;
    }

    return 0;
}

// Apply the operator on top of the operator stack.
bool apply(
    std::stack<double>& values,
    std::stack<Kind>& ops,
    std::string* error) {

    if (ops.empty() || values.size() < 2) {
        *error = "invalid expression";
        return false;
    }

    Kind op = ops.top();
    ops.pop();

    double right = values.top();
    values.pop();

    double left = values.top();
    values.pop();

    double result = 0.0;

    switch (op) {
        case Kind::Plus:
            result = left + right;
            break;

        case Kind::Minus:
            result = left - right;
            break;

        case Kind::Times:
            result = left * right;
            break;

        case Kind::Divide:
            if (right == 0.0) {
                *error = "division by zero";
                return false;
            }
            result = left / right;
            break;

        default:
            *error = "invalid operator";
            return false;
    }

    values.push(result);
    return true;
}

} // namespace

bool calc_evaluate(
    const std::vector<CalcToken>& tokens,
    double* out,
    std::string* error) {

    std::stack<double> values;
    std::stack<Kind> ops;

    for (const CalcToken& token : tokens) {
        Kind kind = token.kind;

        // Numbers go onto the value stack.
        if (kind == Kind::Number) {
            values.push(token.value);
        }

        // Opening parentheses go onto the operator stack.
        else if (kind == Kind::LParen) {
            ops.push(kind);
        }

        // Closing parenthesis: evaluate until '('.
        else if (kind == Kind::RParen) {
            while (!ops.empty() && ops.top() != Kind::LParen) {
                if (!apply(values, ops, error)) {
                    return false;
                }
            }

            if (ops.empty()) {
                *error = "unmatched parenthesis";
                return false;
            }

            ops.pop();
        }

        // Handle arithmetic operators.
        else {
            if (precedence(kind) == 0) {
                *error = "invalid operator";
                return false;
            }

            while (!ops.empty() &&
                   ops.top() != Kind::LParen &&
                   precedence(ops.top()) >= precedence(kind)) {

                if (!apply(values, ops, error)) {
                    return false;
                }
            }

            ops.push(kind);
        }
    }

    // Apply any remaining operators.
    while (!ops.empty()) {
        if (ops.top() == Kind::LParen) {
            *error = "unmatched parenthesis";
            return false;
        }

        if (!apply(values, ops, error)) {
            return false;
        }
    }

    // Exactly one value should remain.
    if (values.size() != 1) {
        *error = "invalid expression";
        return false;
    }

    *out = values.top();
    return true;
}
