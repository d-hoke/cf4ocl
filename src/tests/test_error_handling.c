/*   
 * This file is part of cf4ocl (C Framework for OpenCL).
 * 
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * cf4ocl is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
 * */
 
/** 
 * @file
 * 
 * Tests error handling in cf4ocl.
 * 
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */
 
#include "common.h"

/** Resolves to error category identifying string, in this case an error
 * in the GErrorf tests. */
#define TEST_CCL_ERROR test_error_handling_error_quark()

/**
 * Test error codes.
 * */ 
enum test_error_handling_error_codes {
	TEST_CCL_SUCCESS = 0,
	TEST_CCL_ERROR_1 = -1,
	TEST_CCL_ERROR_2 = -2
};

/** 
 * Resolves to error category identifying string, in this case an
 * error in the GErrorf tests.
 * 
 * @return A GQuark structure defined by category identifying string, 
 * which identifies the error as a gerrof tests generated error.
 */
GQuark test_error_handling_error_quark() {
	return g_quark_from_static_string("test-error-handling-error-quark");
}

/* ************** */
/* Aux. functions */
/* ************** */

int errorL2Aux(int code, const char* xtramsg, GError **err) {
	ccl_if_err_create_goto(*err, TEST_CCL_ERROR, 
		code != TEST_CCL_SUCCESS, code, error_handler, 
		"Big error in level %d function: %s", 2, xtramsg);
	goto finish;
error_handler:
	g_assert(*err != NULL);
finish:
	return code;
}

int errorL1Aux(int code, GError **err) {
	int status = errorL2Aux(code, "called by errorL1Aux", err);
	ccl_if_err_goto(*err, error_handler);
	goto finish;
error_handler:
	g_assert(*err != NULL);
	status = (*err)->code;
finish:
	return status;
}

/* ************** */
/* Test functions */
/* ************** */

/**
 * Test one level error handling.
 * */
static void errorOneLevelTest() {
	GError *err = NULL;

	int status = errorL2Aux(TEST_CCL_ERROR_1, 
		"called by errorOneLevelTest", &err);
	ccl_if_err_goto(err, error_handler);
	status = status; /* Avoid compiler warnings. */

	g_assert_not_reached();
	goto cleanup;

error_handler:
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_1);
	g_assert_cmpstr(err->message, ==, 
		"Big error in level 2 function: called by errorOneLevelTest");
	g_error_free(err);

cleanup:
	return;
	
}

/**
 * Test two level error handling.
 * */
static void errorTwoLevelTest() {
	GError *err = NULL;

	int status = errorL1Aux(TEST_CCL_ERROR_2, &err);
	ccl_if_err_goto(err, error_handler);
	status = status; /* Avoid compiler warnings. */

	g_assert_not_reached();
	goto cleanup;

error_handler:
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_2);
	g_assert_cmpstr(err->message, ==, 
		"Big error in level 2 function: called by errorL1Aux");
	g_error_free(err);

cleanup:
	return;
}

/**
 * Test no errors.
 * */
static void errorNoneTest() {
	GError *err = NULL;

	int status = errorL2Aux(TEST_CCL_SUCCESS, 
		"called by errorOneLevelTest", &err);
	ccl_if_err_goto(err, error_handler);
	status = status; /* Avoid compiler warnings. */

	goto cleanup;

error_handler:
	g_assert_not_reached();
	g_error_free(err);

cleanup:
	g_assert_no_error(err);
	return;
}

/**
 * Test an error without additional arguments.
 * */
static void errorNoVargsTest() {
	GError *err = NULL;

	ccl_if_err_create_goto(err, TEST_CCL_ERROR, 1, 
		TEST_CCL_ERROR_1, error_handler, 
		"I have no additional arguments");

	g_assert_not_reached();
	goto cleanup;

error_handler:
	g_assert_error(err, TEST_CCL_ERROR, TEST_CCL_ERROR_1);
	g_assert_cmpstr(err->message, ==, "I have no additional arguments");
	g_error_free(err);

cleanup:
	return;
}

int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/utils/error-onelevel", errorOneLevelTest);
	g_test_add_func("/utils/error-twolevel", errorTwoLevelTest);
	g_test_add_func("/utils/error-none", errorNoneTest);
	g_test_add_func("/utils/error-novargs", errorNoVargsTest);
	return g_test_run();
}