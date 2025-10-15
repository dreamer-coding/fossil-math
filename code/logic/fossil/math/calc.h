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
#ifndef FOSSIL_MATH_CALC_H
#define FOSSIL_MATH_CALC_H

#include "math.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ============================================================================
// Calculator Function Types and Structures
// ============================================================================

/**
 * @brief Function pointer type for calculator functions.
 *
 * Each function takes an array of double arguments and the argument count,
 * and returns a double result.
 */
typedef double (*fossil_math_calc_func_t)(double* args, size_t argc);

/**
 * @brief Structure representing a calculator variable.
 *
 * Stores the variable name and its value.
 */
typedef struct fossil_math_calc_var_t {
    char name[32];      ///< Variable name
    double value;       ///< Variable value
} fossil_math_calc_var_t;

/**
 * @brief Structure representing a calculator function.
 *
 * Stores the function name, pointer, and argument count.
 */
typedef struct fossil_math_calc_func_entry_t {
    char name[32];                  ///< Function name
    fossil_math_calc_func_t func;        ///< Function pointer
    size_t argc;                    ///< Number of arguments
} fossil_math_calc_func_entry_t;

/**
 * @brief Calculator environment holding variables and functions.
 *
 * - vars:      Array of variables.
 * - funcs:     Array of functions.
 * - var_count: Number of variables.
 * - func_count:Number of functions.
 */
typedef struct fossil_math_calc_env_t {
    fossil_math_calc_var_t* vars;                ///< Array of variables
    fossil_math_calc_func_entry_t* funcs;        ///< Array of functions
    size_t var_count;                       ///< Number of variables
    size_t func_count;                      ///< Number of functions
} fossil_math_calc_env_t;

// ============================================================================
// Calculator Functions
// ============================================================================

/**
 * @brief Evaluates an expression string using the given calculator environment.
 *
 * @param expr Expression string to evaluate.
 * @param env Pointer to calculator environment.
 * @return Numeric result of the evaluation.
 */
double fossil_math_calc_eval(const char* expr, const fossil_math_calc_env_t* env);

/**
 * @brief Sets the value of a variable in the calculator environment.
 *
 * @param env Pointer to calculator environment.
 * @param name Variable name.
 * @param value Value to set.
 * @return 0 on success, non-zero on error.
 */
int fossil_math_calc_set_var(fossil_math_calc_env_t* env, const char* name, double value);

/**
 * @brief Registers a function in the calculator environment.
 *
 * @param env Pointer to calculator environment.
 * @param name Function name.
 * @param func Function pointer.
 * @param argc Number of arguments the function takes.
 * @return 0 on success, non-zero on error.
 */
int fossil_math_calc_register_func(fossil_math_calc_env_t* env, const char* name, fossil_math_calc_func_t func, size_t argc);

/**
 * @brief Initializes the calculator environment.
 *
 * Allocates memory for variables and functions.
 *
 * @param env Pointer to calculator environment.
 */
void fossil_math_calc_env_init(fossil_math_calc_env_t* env);

/**
 * @brief Frees resources associated with the calculator environment.
 *
 * @param env Pointer to calculator environment.
 */
void fossil_math_calc_env_free(fossil_math_calc_env_t* env);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

    namespace math {

        /**
         * @brief Calculator utility class providing static methods for calculator operations.
         *
         * This class wraps the C calculator functions in a C++-friendly interface,
         * allowing for easier use in C++ codebases. All methods are static and
         * operate directly on the provided structures.
         */
        class Calculator {
        public:
            /**
             * @brief Evaluates an expression string using the given calculator environment.
             * @param expr Expression string to evaluate.
             * @param env Reference to calculator environment.
             * @return Numeric result of the evaluation.
             */
            static double eval(const std::string& expr, const fossil_math_calc_env_t& env) {
            return fossil_math_calc_eval(expr.c_str(), &env);
            }

            /**
             * @brief Sets the value of a variable in the calculator environment.
             * @param env Reference to calculator environment.
             * @param name Variable name.
             * @param value Value to set.
             * @return 0 on success, non-zero on error.
             */
            static int set_var(fossil_math_calc_env_t& env, const std::string& name, double value) {
            return fossil_math_calc_set_var(&env, name.c_str(), value);
            }

            /**
             * @brief Registers a function in the calculator environment.
             * @param env Reference to calculator environment.
             * @param name Function name.
             * @param func Function pointer.
             * @param argc Number of arguments the function takes.
             * @return 0 on success, non-zero on error.
             */
            static int register_func(fossil_math_calc_env_t& env, const std::string& name, fossil_math_calc_func_t func, size_t argc) {
            return fossil_math_calc_register_func(&env, name.c_str(), func, argc);
            }

            /**
             * @brief Initializes the calculator environment.
             * @param env Reference to calculator environment.
             */
            static void init(fossil_math_calc_env_t& env) {
            fossil_math_calc_env_init(&env);
            }

            /**
             * @brief Frees resources associated with the calculator environment.
             * @param env Reference to calculator environment.
             */
            static void free(fossil_math_calc_env_t& env) {
            fossil_math_calc_env_free(&env);
            }
        };

    } // namespace math

} // namespace fossil

#endif

#endif /* FOSSIL_MATH_GEOM_H */
