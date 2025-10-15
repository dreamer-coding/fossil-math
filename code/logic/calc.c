/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/math/calc.h"
#include <math.h>

// ============================================================================
// Internal Helpers
// ============================================================================

typedef struct fossil_math_token_t {
    char type;      // 'n' = number, 'v' = variable, 'o' = operator, 'f' = function, '(' or ')', ','
    double value;
    char name[32];
    size_t argc;    // Only used for functions
} fossil_math_token_t;

// Forward declarations
static int fossil_math_tokenize(const char* expr, fossil_math_token_t* tokens, size_t* count);
static double fossil_math_eval_rpn(fossil_math_token_t* tokens, size_t count, const fossil_math_calc_env_t* env);
static int fossil_math_is_operator(char c);
static int fossil_math_precedence(char op);
static double fossil_math_apply_operator(char op, double a, double b);
static double fossil_math_lookup_var(const fossil_math_calc_env_t* env, const char* name);
static const fossil_math_calc_func_entry_t* fossil_math_lookup_func(const fossil_math_calc_env_t* env, const char* name);

// ============================================================================
// Environment Management
// ============================================================================

void fossil_math_calc_env_init(fossil_math_calc_env_t* env) {
    if (!env) return;
    env->vars = NULL;
    env->funcs = NULL;
    env->var_count = 0;
    env->func_count = 0;
}

void fossil_math_calc_env_free(fossil_math_calc_env_t* env) {
    if (!env) return;
    if (env->vars) {
        free(env->vars);
        env->vars = NULL;
    }
    if (env->funcs) {
        free(env->funcs);
        env->funcs = NULL;
    }
    env->var_count = 0;
    env->func_count = 0;
}

int fossil_math_calc_set_var(fossil_math_calc_env_t* env, const char* name, double value) {
    if (!env || !name) return -1;
    for (size_t i = 0; i < env->var_count; ++i) {
        if (strcmp(env->vars[i].name, name) == 0) {
            env->vars[i].value = value;
            return 0;
        }
    }
    fossil_math_calc_var_t* new_vars =
        (env->var_count == 0)
            ? malloc(sizeof(fossil_math_calc_var_t))
            : realloc(env->vars, (env->var_count + 1) * sizeof(fossil_math_calc_var_t));
    if (!new_vars) return -1;
    env->vars = new_vars;
    strncpy(env->vars[env->var_count].name, name, sizeof(env->vars[env->var_count].name) - 1);
    env->vars[env->var_count].name[31] = '\0';
    env->vars[env->var_count].value = value;
    env->var_count++;
    return 0;
}

int fossil_math_calc_register_func(
    fossil_math_calc_env_t* env,
    const char* name,
    fossil_math_calc_func_t func,
    size_t argc
) {
    if (!env || !name || !func) return -1;
    fossil_math_calc_func_entry_t* new_funcs =
        (env->func_count == 0)
            ? malloc(sizeof(fossil_math_calc_func_entry_t))
            : realloc(env->funcs, (env->func_count + 1) * sizeof(fossil_math_calc_func_entry_t));
    if (!new_funcs) return -1;
    env->funcs = new_funcs;
    strncpy(env->funcs[env->func_count].name, name, sizeof(env->funcs[env->func_count].name) - 1);
    env->funcs[env->func_count].name[31] = '\0';
    env->funcs[env->func_count].func = func;
    env->funcs[env->func_count].argc = argc;
    env->func_count++;
    return 0;
}

// ============================================================================
// Evaluation
// ============================================================================

double fossil_math_calc_eval(const char* expr, const fossil_math_calc_env_t* env) {
    if (!expr || !env) return NAN;

    fossil_math_token_t tokens[128];
    size_t token_count = 0;
    if (fossil_math_tokenize(expr, tokens, &token_count) != 0)
        return NAN;

    return fossil_math_eval_rpn(tokens, token_count, env);
}

// ============================================================================
// Tokenization (simple parser, now supports functions)
// ============================================================================

static int fossil_math_tokenize(const char* expr, fossil_math_token_t* tokens, size_t* count) {
    size_t i = 0, n = 0;
    while (expr[i] && n < 128) {
        if (isspace((unsigned char)expr[i])) {
            i++;
            continue;
        }
        if (isdigit((unsigned char)expr[i]) || expr[i] == '.') {
            char* end;
            tokens[n].type = 'n';
            tokens[n].value = strtod(&expr[i], &end);
            i = (size_t)(end - expr);
            n++;
            continue;
        }
        if (isalpha((unsigned char)expr[i])) {
            size_t j = 0;
            while (isalnum((unsigned char)expr[i]) && j < 31)
                tokens[n].name[j++] = expr[i++];
            tokens[n].name[j] = '\0';

            // Check for function call: lookahead for '('
            size_t lookahead = i;
            while (isspace((unsigned char)expr[lookahead])) lookahead++;
            if (expr[lookahead] == '(') {
                tokens[n].type = 'f';
                tokens[n].argc = 0; // Will be set during RPN conversion/eval
            } else {
                tokens[n].type = 'v';
                // Use macro constants directly
                if (strcmp(tokens[n].name, "PI") == 0)
                    tokens[n].value = FOSSIL_MATH_PI;
                else if (strcmp(tokens[n].name, "E") == 0)
                    tokens[n].value = FOSSIL_MATH_E;
                else if (strcmp(tokens[n].name, "TWO_PI") == 0)
                    tokens[n].value = FOSSIL_MATH_TWO_PI;
                else if (strcmp(tokens[n].name, "HALF_PI") == 0)
                    tokens[n].value = FOSSIL_MATH_HALF_PI;
                else if (strcmp(tokens[n].name, "LOG2E") == 0)
                    tokens[n].value = FOSSIL_MATH_LOG2E;
                else if (strcmp(tokens[n].name, "LOG10E") == 0)
                    tokens[n].value = FOSSIL_MATH_LOG10E;
                else if (strcmp(tokens[n].name, "LN2") == 0)
                    tokens[n].value = FOSSIL_MATH_LN2;
                else if (strcmp(tokens[n].name, "LN10") == 0)
                    tokens[n].value = FOSSIL_MATH_LN10;
                else if (strcmp(tokens[n].name, "SQRT2") == 0)
                    tokens[n].value = FOSSIL_MATH_SQRT2;
                else if (strcmp(tokens[n].name, "SQRT1_2") == 0)
                    tokens[n].value = FOSSIL_MATH_SQRT1_2;
                else if (strcmp(tokens[n].name, "DEG2RAD") == 0)
                    tokens[n].value = FOSSIL_MATH_DEG2RAD;
                else if (strcmp(tokens[n].name, "RAD2DEG") == 0)
                    tokens[n].value = FOSSIL_MATH_RAD2DEG;
                else
                    tokens[n].value = 0.0;
            }
            n++;
            continue;
        }
        if (expr[i] == ',') {
            tokens[n].type = ',';
            tokens[n].name[0] = ',';
            tokens[n].name[1] = '\0';
            n++;
            i++;
            continue;
        }
        if (fossil_math_is_operator(expr[i]) || expr[i] == '(' || expr[i] == ')') {
            tokens[n].type = expr[i];
            tokens[n].name[0] = expr[i];
            tokens[n].name[1] = '\0';
            n++;
            i++;
            continue;
        }
        return -1;
    }
    *count = n;
    return 0;
}

// ============================================================================
// Operator Helpers
// ============================================================================

static int fossil_math_is_operator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

static int fossil_math_precedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}

static double fossil_math_apply_operator(char op, double a, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0.0 ? a / b : NAN;
        case '^': return pow(a, b);
        default: return NAN;
    }
}

// ============================================================================
// Variable Lookup
// ============================================================================

static double fossil_math_lookup_var(const fossil_math_calc_env_t* env, const char* name) {
    // Use macro constants directly
    if (strcmp(name, "PI") == 0) return FOSSIL_MATH_PI;
    if (strcmp(name, "E") == 0) return FOSSIL_MATH_E;
    if (strcmp(name, "TWO_PI") == 0) return FOSSIL_MATH_TWO_PI;
    if (strcmp(name, "HALF_PI") == 0) return FOSSIL_MATH_HALF_PI;
    if (strcmp(name, "LOG2E") == 0) return FOSSIL_MATH_LOG2E;
    if (strcmp(name, "LOG10E") == 0) return FOSSIL_MATH_LOG10E;
    if (strcmp(name, "LN2") == 0) return FOSSIL_MATH_LN2;
    if (strcmp(name, "LN10") == 0) return FOSSIL_MATH_LN10;
    if (strcmp(name, "SQRT2") == 0) return FOSSIL_MATH_SQRT2;
    if (strcmp(name, "SQRT1_2") == 0) return FOSSIL_MATH_SQRT1_2;
    if (strcmp(name, "DEG2RAD") == 0) return FOSSIL_MATH_DEG2RAD;
    if (strcmp(name, "RAD2DEG") == 0) return FOSSIL_MATH_RAD2DEG;

    if (!env || !env->vars) return NAN;
    for (size_t i = 0; i < env->var_count; ++i) {
        if (strcmp(env->vars[i].name, name) == 0)
            return env->vars[i].value;
    }
    return NAN;
}

// ============================================================================
// Function Lookup
// ============================================================================

static const fossil_math_calc_func_entry_t* fossil_math_lookup_func(const fossil_math_calc_env_t* env, const char* name) {
    if (!env || !env->funcs) return NULL;
    for (size_t i = 0; i < env->func_count; ++i) {
        if (strcmp(env->funcs[i].name, name) == 0)
            return &env->funcs[i];
    }
    return NULL;
}

// ============================================================================
// RPN Evaluation (Shunting Yard Algorithm, now supports functions)
// ============================================================================

static double fossil_math_eval_rpn(fossil_math_token_t* tokens, size_t count, const fossil_math_calc_env_t* env) {
    double stack[128];
    size_t sp = 0;
    char ops[128];
    size_t op_sp = 0;
    fossil_math_token_t func_stack[32];
    size_t func_sp = 0;
    size_t arg_count_stack[32];
    size_t arg_sp = 0;

    for (size_t i = 0; i < count; ++i) {
        fossil_math_token_t* t = &tokens[i];

        if (t->type == 'n') {
            stack[sp++] = t->value;
        } else if (t->type == 'v') {
            // If token value is set (for built-in constants), use it
            if (strcmp(t->name, "PI") == 0 || strcmp(t->name, "E") == 0 ||
                strcmp(t->name, "TWO_PI") == 0 || strcmp(t->name, "HALF_PI") == 0 ||
                strcmp(t->name, "LOG2E") == 0 || strcmp(t->name, "LOG10E") == 0 ||
                strcmp(t->name, "LN2") == 0 || strcmp(t->name, "LN10") == 0 ||
                strcmp(t->name, "SQRT2") == 0 || strcmp(t->name, "SQRT1_2") == 0 ||
                strcmp(t->name, "DEG2RAD") == 0 || strcmp(t->name, "RAD2DEG") == 0) {
                stack[sp++] = fossil_math_lookup_var(env, t->name);
            } else if (t->value != 0.0) {
                stack[sp++] = t->value;
            } else {
                double val = fossil_math_lookup_var(env, t->name);
                if (isnan(val)) return NAN;
                stack[sp++] = val;
            }
        } else if (t->type == 'f') {
            func_stack[func_sp++] = *t;
            arg_count_stack[arg_sp++] = 0;
            ops[op_sp++] = '('; // treat function as opening parenthesis
        } else if (fossil_math_is_operator(t->type)) {
            while (op_sp > 0 && fossil_math_precedence(ops[op_sp - 1]) >= fossil_math_precedence(t->type)) {
                if (sp < 2) return NAN;
                double b = stack[--sp];
                double a = stack[--sp];
                stack[sp++] = fossil_math_apply_operator(ops[--op_sp], a, b);
            }
            ops[op_sp++] = t->type;
        } else if (t->type == '(') {
            ops[op_sp++] = '(';
        } else if (t->type == ',') {
            // Argument separator for functions
            while (op_sp > 0 && ops[op_sp - 1] != '(') {
                if (sp < 2) return NAN;
                double b = stack[--sp];
                double a = stack[--sp];
                stack[sp++] = fossil_math_apply_operator(ops[--op_sp], a, b);
            }
            if (arg_sp > 0)
                arg_count_stack[arg_sp - 1]++;
        } else if (t->type == ')') {
            while (op_sp > 0 && ops[op_sp - 1] != '(') {
                if (sp < 2) return NAN;
                double b = stack[--sp];
                double a = stack[--sp];
                stack[sp++] = fossil_math_apply_operator(ops[--op_sp], a, b);
            }
            if (op_sp == 0) return NAN; // Mismatched parenthesis
            op_sp--; // pop '('

            // If there's a function on the stack, apply it
            if (func_sp > 0 && arg_sp > 0) {
                fossil_math_token_t func_token = func_stack[func_sp - 1];
                size_t argc = arg_count_stack[arg_sp - 1] + 1; // arguments = commas + 1
                const fossil_math_calc_func_entry_t* func_entry = fossil_math_lookup_func(env, func_token.name);
                if (!func_entry || func_entry->argc != argc) return NAN;
                if (sp < argc) return NAN;
                double args[8];
                for (size_t k = 0; k < argc; ++k)
                    args[k] = stack[sp - argc + k];
                double result = func_entry->func(args, argc);
                sp -= argc;
                stack[sp++] = result;
                func_sp--;
                arg_sp--;
            }
        }
    }

    while (op_sp > 0) {
        if (sp < 2) return NAN;
        double b = stack[--sp];
        double a = stack[--sp];
        stack[sp++] = fossil_math_apply_operator(ops[--op_sp], a, b);
    }

    return sp == 1 ? stack[0] : NAN;
}
