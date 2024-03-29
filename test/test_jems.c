/**
 * @file test_jems.c
 *
 * MIT License
 *
 * Copyright (c) 2022 R. Dunbar Poor
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/**
To run the tests (on a POSIX / gcc style environment):

gcc -g -Wall -I.. -o test_jems test_jems.c ../jems.c && ./test_jems && rm ./test_jems

*/

// *****************************************************************************
// Includes

#include "jems.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// *****************************************************************************
// Private types and definitions

#define MAX_LEVEL 10
#define TEST_STRING_LENGTH 120

#define PI_100                                                                 \
  "3.1415926535"                                                               \
  "8979323846"                                                                 \
  "2643383279"                                                                 \
  "5028841971"                                                                 \
  "6939937510"                                                                 \
  "5820974944"                                                                 \
  "5923078164"                                                                 \
  "0628620899"                                                                 \
  "8628034825"                                                                 \
  "3421170679"

#define ASSERT(e) assert(e, #e, __FILE__, __LINE__)

// *****************************************************************************
// Private (static) storage

static jems_t s_jems;

static jems_level_t s_levels[MAX_LEVEL];

static char s_test_string[TEST_STRING_LENGTH];

static int s_test_idx; // index into next char of s_test_string[]

// *****************************************************************************
// Private (static, forward) declarations

/**
 * @brief Print an error message on stdout if expr is false.
 */
static void assert(bool expr, const char *str, const char *file, int line);

/**
 * @brief Set up for another test.
 */
static void test_reset(void);

/**
 * @brief Write one character to the test string.
 */
static void test_writer(char c, uintptr_t arg);

/**
 * @brief Return true if the test string equals the expected string.
 */
static bool test_result(const char *expected);

// *****************************************************************************
// Public code

int main(void) {
    printf("Starting test_jems...\n");

    ASSERT(jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0) == &s_jems);
    ASSERT(jems_reset(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 0);

    test_reset();
    ASSERT(jems_object_open(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 0);
    ASSERT(jems_object_close(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("{}"));

    test_reset();
    ASSERT(jems_array_open(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 0);
    ASSERT(jems_array_close(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("[]"));

    test_reset();
    ASSERT(jems_number(&s_jems, 1.5) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("1.500000"));

    test_reset();
    ASSERT(jems_number(&s_jems, 2.0) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("2"));

    test_reset();
    ASSERT(jems_integer(&s_jems, -2) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("-2"));

    test_reset();
    ASSERT(jems_string(&s_jems, "woof") == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("\"woof\""));

    test_reset();
    ASSERT(jems_bool(&s_jems, true) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("true"));

    test_reset();
    ASSERT(jems_bool(&s_jems, false) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("false"));

    test_reset();
    ASSERT(jems_true(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("true"));

    test_reset();
    ASSERT(jems_false(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("false"));

    test_reset();
    ASSERT(jems_null(&s_jems) == &s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("null"));

    test_reset();
    jems_literal(&s_jems, PI_100, strlen(PI_100));
    ASSERT(test_result(PI_100));

    // Test string escaping (tip of the hat to Latex95)
    test_reset();
    jems_string(&s_jems, "say \"hey\"!");
    ASSERT(test_result("\"say \\\"hey\\\"!\""));

    test_reset();
    jems_string(&s_jems, "forward / and back \\ slash");
    ASSERT(test_result("\"forward / and back \\\\ slash\""));

    test_reset();
    jems_string(&s_jems, "newline \n and return \r oh my");
    ASSERT(test_result("\"newline \\u000a and return \\u000d oh my\""));

    // test \uxxxx style encoding
    do {
        test_reset();
        const char str[] = {0x01, 0x20, 0x7e, 0x7f, 0x80, 0x00};
        jems_string(&s_jems, str);
        ASSERT(test_result("\"\\u0001 ~\\u007f\\u0080\""));
    } while (false);

    do {
        test_reset();
        const char str[] = {0x00, 0x01, 0x20, 0x7e, 0x7f, 0x80};
        jems_bytes(&s_jems, (uint8_t *)str, sizeof(str));
        ASSERT(test_result("\"\\u0000\\u0001 ~\\u007f\\u0080\""));
    } while (false);

    // key:value pairs
    test_reset();
    jems_object_open(&s_jems);
    jems_key_object_open(&s_jems, "key");
    jems_object_close(&s_jems);
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":{}}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_array_open(&s_jems, "key");
    jems_array_close(&s_jems);
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":[]}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_number(&s_jems, "key", 1.234);
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":1.234000}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_integer(&s_jems, "key", 1234);
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":1234}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_string(&s_jems, "key", "value");
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":\"value\"}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_bytes(&s_jems, "key", (uint8_t *)"value", strlen("value"));
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":\"value\"}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_bool(&s_jems, "key", true);
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":true}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_true(&s_jems, "key");
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":true}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_false(&s_jems, "key");
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":false}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_null(&s_jems, "key");
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"key\":null}"));

    test_reset();
    jems_object_open(&s_jems);
    jems_key_literal(&s_jems, "pi", PI_100, strlen(PI_100));
    jems_object_close(&s_jems);
    ASSERT(test_result("{\"pi\":" PI_100 "}"));

    test_reset();
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 0);
    jems_object_open(&s_jems);
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 0);
    jems_string(&s_jems, "colors");
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 1);
    jems_array_open(&s_jems);
    ASSERT(jems_curr_level(&s_jems) == 2);
    ASSERT(jems_item_count(&s_jems) == 0);
    jems_integer(&s_jems, 1);
    ASSERT(jems_curr_level(&s_jems) == 2);
    ASSERT(jems_item_count(&s_jems) == 1);
    jems_integer(&s_jems, 2);
    ASSERT(jems_curr_level(&s_jems) == 2);
    ASSERT(jems_item_count(&s_jems) == 2);
    jems_integer(&s_jems, 3);
    ASSERT(jems_curr_level(&s_jems) == 2);
    ASSERT(jems_item_count(&s_jems) == 3);
    jems_array_close(&s_jems);
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 2);
    jems_string(&s_jems, "valid");
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 3);
    jems_true(&s_jems);
    ASSERT(jems_curr_level(&s_jems) == 1);
    ASSERT(jems_item_count(&s_jems) == 4);
    jems_object_close(&s_jems);
    ASSERT(jems_curr_level(&s_jems) == 0);
    ASSERT(jems_item_count(&s_jems) == 1);
    ASSERT(test_result("{\"colors\":[1,2,3],\"valid\":true}"));

    printf("\n... Finished test_jems\n");
}

// *****************************************************************************
// Private (static) code

static void assert(bool expr, const char *str, const char *file, int line) {
    if (!expr) {
        printf("\nassertion %s failed at %s:%d", str, file, line);
    }
}

static void test_reset(void) {
    jems_init(&s_jems, s_levels, MAX_LEVEL, test_writer, 0);
    s_test_idx = 0;
}

static void test_writer(char c, uintptr_t arg) {
    (void)arg;
    if (s_test_idx < sizeof(s_test_string)) {
        s_test_string[s_test_idx++] = c;
    }
}

static bool test_result(const char *expected) {
    s_test_string[s_test_idx] = '\0';
    printf("\nrendered %s", s_test_string);
    // printf("\nexpected %s", expected);
    return strcmp(s_test_string, expected) == 0;
}

// *****************************************************************************
// End of file
