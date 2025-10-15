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

FOSSIL_TEST_CASE(c_math_test_calc_env_init_free) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    ASSUME_ITS_TRUE(env.vars == NULL);
    ASSUME_ITS_TRUE(env.funcs == NULL);
    ASSUME_ITS_TRUE(env.var_count == 0);
    ASSUME_ITS_TRUE(env.func_count == 0);
    fossil_math_calc_env_free(&env);
    ASSUME_ITS_TRUE(env.vars == NULL);
    ASSUME_ITS_TRUE(env.funcs == NULL);
    ASSUME_ITS_TRUE(env.var_count == 0);
    ASSUME_ITS_TRUE(env.func_count == 0);
}

FOSSIL_TEST_CASE(c_math_test_calc_set_var) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    int ret = fossil_math_calc_set_var(&env, "x", 42.0);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(env.var_count == 1);
    ASSUME_ITS_EQUAL_F64(env.vars[0].value, 42.0, FOSSIL_TEST_FLOAT_EPSILON);
    ret = fossil_math_calc_set_var(&env, "x", 24.0);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(env.var_count == 1);
    ASSUME_ITS_EQUAL_F64(env.vars[0].value, 24.0, FOSSIL_TEST_FLOAT_EPSILON);
    ret = fossil_math_calc_set_var(&env, "y", 100.0);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(env.var_count == 2);
    ASSUME_ITS_EQUAL_F64(env.vars[1].value, 100.0, FOSSIL_TEST_FLOAT_EPSILON);
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
    ASSUME_ITS_TRUE(env.func_count == 1);
    ASSUME_ITS_TRUE(env.funcs[0].func == test_add_func);
    ASSUME_ITS_TRUE(env.funcs[0].argc == 2);
    ret = fossil_math_calc_register_func(&env, "add", test_add_func, 2);
    ASSUME_ITS_TRUE(ret == 0); // Should update, not add new
    ASSUME_ITS_TRUE(env.func_count == 1);
    ret = fossil_math_calc_register_func(&env, "mul", test_add_func, 2);
    ASSUME_ITS_TRUE(ret == 0);
    ASSUME_ITS_TRUE(env.func_count == 2);
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

FOSSIL_TEST_CASE(c_math_test_calc_eval_operators) {
    fossil_math_calc_env_t env;
    fossil_math_calc_env_init(&env);
    double result = fossil_math_calc_eval("2 + 3", &env);
    ASSUME_ITS_EQUAL_F64(result, 5.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_calc_set_var(&env, "x", 4.0);
    result = fossil_math_calc_eval("x * 2", &env);
    ASSUME_ITS_EQUAL_F64(result, 8.0, FOSSIL_TEST_FLOAT_EPSILON);
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
