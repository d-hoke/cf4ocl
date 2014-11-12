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
 * along with cf4ocl. If not, see <http://www.gnu.org/licenses/>.
 * */

/**
 * @file
 * Test the event wrapper class and its methods.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)
 * */

#include <cf4ocl2.h>

#ifdef CL_VERSION_1_1

/**
 * Tests user events.
 * */
static void user_event_test() {

	/* Test variables. */
	CCLEvent* uevt = NULL;
	CCLContext* ctx = NULL;
	GError* err = NULL;
	cl_command_queue clcq = NULL;
	cl_context clctx = NULL;
	cl_command_type clct = 0;
	cl_int exec_status;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Create user event. */
	uevt = ccl_user_event_new(ctx, &err);
	g_assert_no_error(err);

	/* Check that event ref count is 1. */
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Increase ref count, check it is 2. */
	ccl_event_ref(uevt);
	g_assert_cmpuint(2, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Decrease ref count, check it is 1. */
	ccl_event_unref(uevt);
	g_assert_cmpuint(1, ==, ccl_wrapper_ref_count((CCLWrapper*) uevt));

	/* Confirm that event command queue is NULL. */
	clcq = ccl_event_get_info_scalar(uevt, CL_EVENT_COMMAND_QUEUE,
		cl_command_queue, &err);
	g_assert_no_error(err);
	g_assert_cmphex(((cl_ulong) clcq), ==, ((cl_ulong) NULL));

	/* Confirm that event returns the correct context. */
	clctx = ccl_event_get_info_scalar(uevt, CL_EVENT_CONTEXT,
		cl_context, &err);
	g_assert_no_error(err);
	g_assert_cmphex(((cl_ulong) clctx), ==, ((cl_ulong) ccl_context_unwrap(ctx)));

	/* Confirm command type is user event. */
	clct = ccl_event_get_info_scalar(uevt, CL_EVENT_COMMAND_TYPE,
		cl_command_type, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(clct, ==, CL_COMMAND_USER);

	/* Confirm execution status is "submitted". */
	exec_status = ccl_event_get_info_scalar(uevt,
		CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_SUBMITTED);

	/* Change execution status to "complete". */
	ccl_user_event_set_status(uevt, CL_COMPLETE, &err);
	g_assert_no_error(err);

	/* Confirm execution status is "complete". */
	exec_status = ccl_event_get_info_scalar(uevt,
		CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_COMPLETE);

	/* Destroy stuff. */
	ccl_event_destroy(uevt);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Test callback function.
 * */
static void callback_fun(
	cl_event event, cl_int event_command_exec_status, void *user_data) {

	(void)event;

	/* Confirm event is CL_COMPLETE. */
	g_assert_cmpint(event_command_exec_status, ==, CL_COMPLETE);

	/* Set userdata to CL_TRUE, thus providing evidence that the
	 * callback was indeed called. */
	*((cl_bool*) user_data) = CL_TRUE;
}

/**
 * Tests event callbacks.
 * */
static void event_callback_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLQueue* cq = NULL;
	CCLBuffer* buf = NULL;
	CCLEvent* evt = NULL;
	GError* err = NULL;
	GTimer* timer = NULL;
	cl_uint vector[] = {0, 1, 2, 3, 4, 5, 6, 7};
	cl_bool test_var = CL_FALSE;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Create a device buffer. */
	buf = ccl_buffer_new(
		ctx, CL_MEM_READ_WRITE, 8 * sizeof(cl_uint), NULL, &err);
	g_assert_no_error(err);

	/* Write something to buffer and get an event. */
	evt = ccl_buffer_enqueue_write(buf, cq, CL_FALSE, 0,
		8 * sizeof(cl_uint), vector, NULL, &err);
	g_assert_no_error(err);

	/* Add a callback. */
	ccl_event_set_callback(
		evt, CL_COMPLETE, callback_fun, &test_var, &err);
	g_assert_no_error(err);

	/* Wait on host thread for all events to complete. */
	ccl_queue_finish(cq, &err);
	g_assert_no_error(err);

	/* Release wrappers. */
	ccl_buffer_destroy(buf);
	ccl_queue_destroy(cq);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

	/* Wait some more... */
	timer = g_timer_new();
	while (g_timer_elapsed(timer, NULL) < 2.0);
	g_timer_stop(timer);
	g_timer_destroy(timer);

	/* Confirm that test_var is CL_TRUE. */
	g_assert_cmpuint(test_var, ==, CL_TRUE);

}

#endif

/**
 * Event name and type test.
 * */
static void event_name_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLQueue* cq = NULL;
	CCLBuffer* buf = NULL;
	CCLEvent* evt = NULL;
	GError* err = NULL;
	cl_ulong* host_buf;
	CCLEventWaitList ewl = NULL;
	cl_int exec_status = -1;
	cl_command_type ct = 0;
	const char* evt_name = NULL;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create a command queue. */
	cq = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Create a device buffer. */
	buf = ccl_buffer_new(
		ctx, CL_MEM_READ_WRITE, 8 * sizeof(cl_ulong), NULL, &err);
	g_assert_no_error(err);

	/* Map device buffer, get an event and analise it. */
	host_buf = ccl_buffer_enqueue_map(buf, cq, CL_FALSE, CL_MAP_WRITE,
		0, 8 * sizeof(cl_ulong), NULL, &evt, &err);
	g_assert_no_error(err);

	/* Wait that buffer gets mapped. */
	ccl_event_wait_list_add(&ewl, evt, NULL);
	ccl_event_wait(&ewl, &err);
	g_assert_no_error(err);

	/* Check that the event is CL_COMPLETE. */
	exec_status = ccl_event_get_info_scalar(
		evt, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_COMPLETE);

	/* Check that the event is CL_COMMAND_MAP_BUFFER. */
	ct = ccl_event_get_command_type(evt, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(ct, ==, CL_COMMAND_MAP_BUFFER);

	/* Check that final event name is "MAP_BUFFER". */
	evt_name = ccl_event_get_final_name(evt);
	g_assert_cmpstr("MAP_BUFFER", ==, evt_name);

	/* Set another name for the event. */
	ccl_event_set_name(evt, "SomeOtherName");

	/* Get the final event name now. */
	evt_name = ccl_event_get_name(evt);
	g_assert_cmpstr("SomeOtherName", ==, evt_name);

	/* Unmap buffer, get resulting event. */
	evt = ccl_buffer_enqueue_unmap(buf, cq, host_buf, NULL, &err);
	g_assert_no_error(err);

	/* Wait that buffer gets unmapped. */
	ccl_event_wait_list_add(&ewl, evt, NULL);
	ccl_event_wait(&ewl, &err);
	g_assert_no_error(err);

	/* Check that the event is CL_COMPLETE. */
	exec_status = ccl_event_get_info_scalar(
		evt, CL_EVENT_COMMAND_EXECUTION_STATUS, cl_int, &err);
	g_assert_no_error(err);
	g_assert_cmpint(exec_status, ==, CL_COMPLETE);

	/* Check that the event is CL_COMMAND_UNMAP_MEM_OBJECT. */
	ct = ccl_event_get_command_type(evt, &err);
	g_assert_no_error(err);
	g_assert_cmpuint(ct, ==, CL_COMMAND_UNMAP_MEM_OBJECT);

	/* Check that final event name is "UNMAP_MEM_OBJECT". */
	evt_name = ccl_event_get_final_name(evt);
	g_assert_cmpstr("UNMAP_MEM_OBJECT", ==, evt_name);

	/* Release wrappers. */
	ccl_buffer_destroy(buf);
	ccl_queue_destroy(cq);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());
}

/**
 * Event wait lists test.
 * */
static void event_wait_lists_test() {

	/* Test variables. */
	CCLContext* ctx = NULL;
	CCLDevice* dev = NULL;
	CCLQueue* cq1 = NULL;
	CCLQueue* cq2 = NULL;
	CCLBuffer* buf = NULL;
	CCLEvent* evt = NULL;
	GError* err = NULL;
	cl_float host_buf1[8] = { 2.0, 3.5, 4.2, 5.0, 2.2, 199.0, -12.9, -0.01 };
	cl_float host_buf2[8];
	CCLEvent* evt_array[2] = { NULL, NULL };
	CCLEventWaitList ewl = NULL;
	const cl_event* clevent_ptr;

	/* Get a context with any device. */
	ctx = ccl_context_new_any(&err);
	g_assert_no_error(err);

	/* Get first device in context. */
	dev = ccl_context_get_device(ctx, 0, &err);
	g_assert_no_error(err);

	/* Create command queues. */
	cq1 = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	cq2 = ccl_queue_new(ctx, dev, 0, &err);
	g_assert_no_error(err);

	/* Create a device buffer. */
	buf = ccl_buffer_new(
		ctx, CL_MEM_READ_WRITE, 8 * sizeof(cl_float), NULL, &err);
	g_assert_no_error(err);

	/* Write something to buffer using command queue 1, get an event. */
	evt = ccl_buffer_enqueue_write(buf, cq1, CL_FALSE, 0,
		8 * sizeof(cl_float), host_buf1, NULL, &err);
	g_assert_no_error(err);

	/* Read something from buffer using command queue 2, depending on
	 * previous event. */
	evt = ccl_buffer_enqueue_read(buf, cq2, CL_FALSE, 0,
		8 * sizeof(cl_float), host_buf2, ccl_ewl(&ewl, evt, NULL), &err);
	g_assert_no_error(err);

	/* Wait for read event using ccl_event_wait_list_add_v(). */
	evt_array[0] = evt;
	ccl_event_wait_list_add_v(&ewl, evt_array);

	/* Analise event wait list. */
	g_assert_cmpuint(ccl_event_wait_list_get_num_events(&ewl), ==, 1);
	clevent_ptr = ccl_event_wait_list_get_clevents(&ewl);
	g_assert(*clevent_ptr == ccl_event_unwrap(evt));

	/* Wait on last event. */
	ccl_event_wait(&ewl, &err);
	g_assert_no_error(err);

	/* Check that ewl is NULL. */
	g_assert(ewl == NULL);

	/* Check if host buffers contain the same information. */
	for (cl_uint i = 0; i < 8; ++i)
		g_assert_cmpfloat(host_buf1[i], ==, host_buf2[i]);

	/* Re-add completed event to wait list, clear list explicitly. */
	ccl_event_wait_list_add(&ewl, evt, NULL);
	g_assert(ewl != NULL);
	ccl_event_wait_list_clear(&ewl);
	g_assert(ewl == NULL);

	/* Clear it again, should throw no error. */
	ccl_event_wait_list_clear(&ewl);
	g_assert(ewl == NULL);

	/* Release wrappers. */
	ccl_buffer_destroy(buf);
	ccl_queue_destroy(cq1);
	ccl_queue_destroy(cq2);
	ccl_context_destroy(ctx);

	/* Confirm that memory allocated by wrappers has been properly
	 * freed. */
	g_assert(ccl_wrapper_memcheck());

}

/**
 * Main function.
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Command line arguments.
 * @return Result of test run.
 * */
int main(int argc, char** argv) {

	g_test_init(&argc, &argv, NULL);

#ifdef CL_VERSION_1_1

	g_test_add_func(
		"/wrappers/event/user",
		user_event_test);

	g_test_add_func(
		"/wrappers/event/callback",
		event_callback_test);
#endif

	g_test_add_func(
		"/wrappers/event/name-type",
		event_name_test);

	g_test_add_func(
		"/wrappers/event/wait-lists",
		event_wait_lists_test);

	return g_test_run();
}




