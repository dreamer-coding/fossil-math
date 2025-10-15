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
#include <fossil/pizza/framework.h>
#include "fossil/math/framework.h"


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(c_calc_fixture);

FOSSIL_SETUP(c_calc_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_calc_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE(c_math_test_tokenize_basic) {
    fossil_math_token_t tokens[16];
    size_t count = 0;
    int ret = fossil_math_tokenize("2 + x", tokens, &count);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(count == 3);
    ASSUME_ITS_TRUE(tokens[0].type == 'n');
    ASSUME_ITS_EQUAL_F64(tokens[0].value, 2.0, FOSSIL_TEST_FLOAT_EPSILON);
    ASSUME_ITS_TRUE(tokens[1].type == '+');
    ASSUME_ITS_TRUE(tokens[2].type == 'v');
    ASSUME_ITS_TRUE(strcmp(tokens[2].name, "x") == 0);
}

FOSSIL_TEST_CASE(c_math_test_tokenize_function) {
    fossil_math_token_t tokens[16];
    size_t count = 0;
    int ret = fossil_math_tokenize("add(1,2)", tokens, &count);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(count == 6);
    ASSUME_ITS_TRUE(tokens[0].type == 'f');
    ASSUME_ITS_TRUE(strcmp(tokens[0].name, "add") == 0);
    ASSUME_ITS_TRUE(tokens[1].type == '(');
    ASSUME_ITS_TRUE(tokens[2].type == 'n');
    ASSUME_ITS_EQUAL_F64(tokens[2].value, 1.0, FOSSIL_TEST_FLOAT_EPSILON);
    ASSUME_ITS_TRUE(tokens[3].type == ',');
    ASSUME_ITS_TRUE(tokens[4].type == 'n');
    ASSUME_ITS_EQUAL_F64(tokens[4].value, 2.0, FOSSIL_TEST_FLOAT_EPSILON);
    ASSUME_ITS_TRUE(tokens[5].type == ')');
}

FOSSIL_TEST_CASE(c_math_test_tokenize_constants) {
    fossil_math_token_t tokens[8];
    size_t count = 0;
    int ret = fossil_math_tokenize("PI + E", tokens, &count);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(count == 3);
    ASSUME_ITS_TRUE(tokens[0].type == 'v');
    ASSUME_ITS_TRUE(strcmp(tokens[0].name, "PI") == 0);
    ASSUME_ITS_EQUAL_F64(tokens[0].value, FOSSIL_MATH_PI, FOSSIL_TEST_FLOAT_EPSILON);
    ASSUME_ITS_TRUE(tokens[2].type == 'v');
    ASSUME_ITS_TRUE(strcmp(tokens[2].name, "E") == 0);
    ASSUME_ITS_EQUAL_F64(tokens[2].value, FOSSIL_MATH_E, FOSSIL_TEST_FLOAT_EPSILON);
}

FOSSIL_TEST_CASE(c_math_test_eval_rpn_basic) {
    fossil_math_token_t tokens[3];
    tokens[0].type = 'n'; tokens[0].value = 2.0;
    tokens[1].type = 'n'; tokens[1].value = 3.0;
    tokens[2].type = '+'; strcpy(tokens[2].name, "+");
    double result = fossil_math_eval_rpn(tokens, 3, NULL);
    ASSUME_ITS_EQUAL_F64(result, 5.0, FOSSIL_TEST_FLOAT_EPSILON);
}

FOSSIL_TEST_CASE(c_math_test_eval_rpn_variable) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    fossil_math_calc_set_var(&env, "x", 7.0);
    fossil_math_token_t tokens[3];
    tokens[0].type = 'n'; tokens[0].value = 2.0;
    tokens[1].type = 'v'; strcpy(tokens[1].name, "x");
    tokens[2].type = '*'; strcpy(tokens[2].name, "*");
    double result = fossil_math_eval_rpn(tokens, 3, &env);
    ASSUME_ITS_EQUAL_F64(result, 14.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_eval_rpn_function) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    fossil_math_calc_register_func(&env, "add", test_add_func, 2);
    fossil_math_token_t tokens[4];
    tokens[0].type = 'n'; tokens[0].value = 2.0;
    tokens[1].type = 'n'; tokens[1].value = 3.0;
    tokens[2].type = 'f'; strcpy(tokens[2].name, "add"); tokens[2].argc = 2;
    tokens[3].type = ')';
    // Simulate RPN for add(2,3): [2][3][add][)]
    double result = fossil_math_eval_rpn(tokens, 4, &env);
    ASSUME_ITS_EQUAL_F64(result, 5.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_eval_rpn_constants) {
    fossil_math_token_t tokens[3];
    tokens[0].type = 'v'; strcpy(tokens[0].name, "PI"); tokens[0].value = FOSSIL_MATH_PI;
    tokens[1].type = 'v'; strcpy(tokens[1].name, "E"); tokens[1].value = FOSSIL_MATH_E;
    tokens[2].type = '+'; strcpy(tokens[2].name, "+");
    double result = fossil_math_eval_rpn(tokens, 3, NULL);
    ASSUME_ITS_EQUAL_F64(result, FOSSIL_MATH_PI + FOSSIL_MATH_E, FOSSIL_TEST_FLOAT_EPSILON);
}

FOSSIL_TEST_CASE(c_math_test_calc_env_init_free) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    ASSUME_ITS_TRUE(env.vars == NULL);
    ASSUME_ITS_TRUE(env.funcs == NULL);
    fossil_math_calc_env_free(&env);
    ASSUME_ITS_TRUE(env.vars == NULL);
    ASSUME_ITS_TRUE(env.funcs == NULL);
}

FOSSIL_TEST_CASE(c_math_test_calc_set_var) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    int ret = fossil_math_calc_set_var(&env, "x", 42.0);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_EQUAL_F64(env.vars[0].value, 42.0, FOSSIL_TEST_FLOAT_EPSILON);
    ret = fossil_math_calc_set_var(&env, "x", 24.0);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_EQUAL_F64(env.vars[0].value, 24.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

static double test_add_func(double* args, size_t argc) {
    return argc == 2 ? args[0] + args[1] : 0.0;
}

FOSSIL_TEST_CASE(c_math_test_calc_register_func) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    int ret = fossil_math_calc_register_func(&env, "add", test_add_func, 2);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(env.funcs[0].func == test_add_func);
    ASSUME_ITS_TRUE(env.funcs[0].argc == 2);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_simple) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    fossil_math_calc_set_var(&env, "x", 5.0);
    fossil_math_calc_register_func(&env, "add", test_add_func, 2);
    double result = fossil_math_calc_eval("add(x, 3)", &env);
    ASSUME_ITS_EQUAL_F64(result, 8.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_constants) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    double result = fossil_math_calc_eval("PI + E", &env);
    ASSUME_ITS_EQUAL_F64(result, FOSSIL_MATH_PI + FOSSIL_MATH_E, FOSSIL_TEST_FLOAT_EPSILON);
    result = fossil_math_calc_eval("TWO_PI - HALF_PI", &env);
    ASSUME_ITS_EQUAL_F64(result, FOSSIL_MATH_TWO_PI - FOSSIL_MATH_HALF_PI, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_operators) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    double result = fossil_math_calc_eval("2 + 3 * 4", &env);
    ASSUME_ITS_EQUAL_F64(result, 2 + 3 * 4, FOSSIL_TEST_FLOAT_EPSILON);
    result = fossil_math_calc_eval("(2 + 3) * 4", &env);
    ASSUME_ITS_EQUAL_F64(result, (2 + 3) * 4, FOSSIL_TEST_FLOAT_EPSILON);
    result = fossil_math_calc_eval("2 ^ 3", &env);
    ASSUME_ITS_EQUAL_F64(result, pow(2, 3), FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

static double test_mul_func(double* args, size_t argc) {
    return argc == 2 ? args[0] * args[1] : 0.0;
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_function_args) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    fossil_math_calc_register_func(&env, "mul", test_mul_func, 2);
    double result = fossil_math_calc_eval("mul(6, 7)", &env);
    ASSUME_ITS_EQUAL_F64(result, 42.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_nested_functions) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    fossil_math_calc_register_func(&env, "add", test_add_func, 2);
    fossil_math_calc_register_func(&env, "mul", test_mul_func, 2);
    double result = fossil_math_calc_eval("add(mul(2, 3), 4)", &env);
    ASSUME_ITS_EQUAL_F64(result, 2 * 3 + 4, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_env_free(&env);
}

FOSSIL_TEST_CASE(c_math_test_calc_eval_invalid_expr) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    double result = fossil_math_calc_eval("2 +", &env);
    ASSUME_ITS_TRUE(isnan(result));
    result = fossil_math_calc_eval("add(1)", &env); // wrong argc
    ASSUME_ITS_TRUE(isnan(result));
    fossil_math_calc_env_free(&env);
}


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_calc_tests) {
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_env_init_free);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_set_var);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_register_func);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_simple);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_constants);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_operators);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_function_args);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_nested_functions);
    FOSSIL_TEST_ADD(c_calc_fixture, c_math_test_calc_eval_invalid_expr);

    FOSSIL_TEST_REGISTER(c_calc_fixture);
} // end of tests
