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

FOSSIL_TEST_SUITE(c_symbolic_fixture);

FOSSIL_SETUP(c_symbolic_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_symbolic_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

static double test_var_lookup(const char* name) {
    if (strcmp(name, "x") == 0) return 2.0;
    if (strcmp(name, "y") == 0) return 3.0;
    return 0.0;
}

FOSSIL_TEST_CASE(c_math_test_sym_parse_and_to_string) {
    fossil_math_sym_expr_t* expr = fossil_math_sym_parse("x + 2");
    char buf[64];
    size_t len = fossil_math_sym_to_string(expr, buf, sizeof(buf));
    ASSUME_ITS_TRUE(len > 0);
    ASSUME_ITS_TRUE(strstr(buf, "x") != NULL);
    fossil_math_sym_free(expr);
}

FOSSIL_TEST_CASE(c_math_test_sym_eval) {
    fossil_math_sym_expr_t* expr = fossil_math_sym_parse("x * y + 1");
    double val = fossil_math_sym_eval(expr, test_var_lookup);
    ASSUME_ITS_EQUAL_F64(val, 7.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_sym_free(expr);
}

FOSSIL_TEST_CASE(c_math_test_sym_simplify) {
    fossil_math_sym_expr_t* expr = fossil_math_sym_parse("2 + 2");
    fossil_math_sym_expr_t* simp = fossil_math_sym_simplify(expr);
    char buf[32];
    fossil_math_sym_to_string(simp, buf, sizeof(buf));
    ASSUME_ITS_TRUE(strstr(buf, "4") != NULL);
    fossil_math_sym_free(expr);
    fossil_math_sym_free(simp);
}

FOSSIL_TEST_CASE(c_math_test_sym_diff) {
    fossil_math_sym_expr_t* expr = fossil_math_sym_parse("x^2 + 3*x");
    fossil_math_sym_expr_t* diff = fossil_math_sym_diff(expr, "x");
    char buf[64];
    fossil_math_sym_to_string(diff, buf, sizeof(buf));
    ASSUME_ITS_TRUE(strstr(buf, "2*x") != NULL);
    ASSUME_ITS_TRUE(strstr(buf, "3") != NULL);
    fossil_math_sym_free(expr);
    fossil_math_sym_free(diff);
}

FOSSIL_TEST_CASE(c_math_test_sym_substitute) {
    fossil_math_sym_expr_t* expr = fossil_math_sym_parse("x + y");
    fossil_math_sym_expr_t* sub = fossil_math_sym_substitute(expr, "x", 5.0);
    double val = fossil_math_sym_eval(sub, test_var_lookup);
    ASSUME_ITS_EQUAL_F64(val, 8.0, FOSSIL_TEST_FLOAT_EPSILON);
    fossil_math_sym_free(expr);
    fossil_math_sym_free(sub);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_symbolic_tests) {
    FOSSIL_TEST_ADD(c_symbolic_fixture, c_math_test_sym_parse_and_to_string);
    FOSSIL_TEST_ADD(c_symbolic_fixture, c_math_test_sym_eval);
    FOSSIL_TEST_ADD(c_symbolic_fixture, c_math_test_sym_simplify);
    FOSSIL_TEST_ADD(c_symbolic_fixture, c_math_test_sym_diff);
    FOSSIL_TEST_ADD(c_symbolic_fixture, c_math_test_sym_substitute);

    FOSSIL_TEST_REGISTER(c_symbolic_fixture);
} // end of tests
