/*
 * This file is part of cf4ocl (C Framework for OpenCL).
 *
 * cf4ocl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * cf4ocl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with cf4ocl. If not, see
 * <http://www.gnu.org/licenses/>.
 * */

 /**
 * @file
 *
 * Definition of a wrapper class and its methods for OpenCL image
 * objects.
 *
 * @author Nuno Fachada
 * @date 2014
 * @copyright [GNU Lesser General Public License version 3 (LGPLv3)](http://www.gnu.org/licenses/lgpl.html)
 * */

#ifndef _CCL_IMAGE_WRAPPER_H_
#define _CCL_IMAGE_WRAPPER_H_

#include "memobj_wrapper.h"

/* Forward definition of CCLBuffer. */
typedef struct ccl_buffer CCLBuffer;

/**
 * @defgroup IMAGE_WRAPPER Image wrapper
 *
 * The image wrapper module provides functionality for simple
 * handling of OpenCL image objects.
 *
 * All the functions in this module are direct wrappers of the
 * respective OpenCL image functions. The ::ccl_image_new() constructor
 * accepts a variable list of arguments which describe the image to be
 * created. There is also the ::ccl_image_new_v(), which accepts the
 * image description arguments given in a ::CCLImageDesc* object. Both
 * constructors will automatically use the old style
 * clCreateImage2D()/clCreateImage3D() constructors if the underlying
 * platform OpenCL version is less or equal than 1.1, or the new
 * clCreateImage() constructor otherwise. Instantiation and destruction
 * of image wrappers follows the _cf4ocl_
 * @ref ug_new_destroy "new/destroy" rule; as such, images should be
 * freed with the ::ccl_image_destroy() destructor.
 *
 * Information about image objects can be fetched using the
 * @ref ug_getinfo "info macros":
 *
 * * ::ccl_image_get_info_scalar()
 * * ::ccl_image_get_info_array()
 * * ::ccl_image_get_info()
 *
 * If the information to be fetched is relative to the memory object
 * super-class (e.g. `CL_MEM_TYPE` or `CL_MEM_FLAGS`), then the
 * @ref MEMOBJ_WRAPPER "memory object module" info macros should be used
 * instead:
 *
 * * ::ccl_memobj_get_info_scalar()
 * * ::ccl_memobj_get_info_array()
 * * ::ccl_memobj_get_info()
 *
 * _Example:_
 *
 * @code{.c}
 * CCLQueue* queue;
 * CCLImage* img_in;
 * CCLImage* img_out;
 * cl_uchar4 host_img[IMG_X * IMG_Y];
 * cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
 * size_t origin[3] = { 0, 0, 0 };
 * size_t region[3] = { IMG_X, IMG_Y, 1 };
 * @endcode
 * @code{.c}
 * img_in = ccl_image_new(ctx, CL_MEM_READ_ONLY, &image_format, NULL, &err,
 *     "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
 *     "image_width", (size_t) IMG_X,
 *     "image_height", (size_t) IMG_Y,
 *     NULL);
 * img_out = ccl_image_new(ctx, CL_MEM_WRITE_ONLY, &image_format, NULL, &err,
 *     "image_type", (cl_mem_object_type) CL_MEM_OBJECT_IMAGE2D,
 *     "image_width", (size_t) IMG_X,
 *     "image_height", (size_t) IMG_Y,
 *     NULL);
 * @endcode
 * @code{.c}
 * ccl_image_enqueue_write(queue, img_in, CL_TRUE, origin, region,
 *     0, 0, host_img, NULL, NULL);
 * @endcode
 * _enqueue some image processing kernel..._
 * @code{.c}
 * ccl_image_enqueue_read(queue, img_out, CL_TRUE, origin, region,
 *     0, 0, host_img, NULL, NULL);
 * @endcode
 * @code{.c}
 * ccl_image_destroy(img_in);
 * ccl_image_destroy(img_out);
 * @endcode
 *
 * @{
 */

/**
 * This macro can be used to set a ::CCLImageDesc object to all zeros.
 * For example:
 *
 * @code{.c}
 * CCLImageDesc image_desc = CCL_IMAGE_DESC_BLANK;
 * @endcode
 * */
#define CCL_IMAGE_DESC_BLANK {0, 0, 0, 0, 0, 0, 0, 0, 0, NULL}

/**
 * Image wrapper class
 *
 * @extends ccl_memobj
 * */
typedef struct ccl_image CCLImage;

/**
 * Describes the type and dimensions of the image or image array
 * independently of the OpenCL version. Should be initialized with the
 * ::CCL_IMAGE_DESC_BLANK macro.
 * */
typedef struct ccl_image_desc {

	/**
	 * The type of image, must be supported by the underlying OpenCL
	 * version. Only `CL_MEM_OBJECT_IMAGE2D` and `CL_MEM_OBJECT_IMAGE3D`
	 * are supported in all OpenCL versions.
	 * @public
	 * */
	cl_mem_object_type image_type;

	/**
	 * Width of the image in pixels. Used for all image types.
	 * @public
	 * */
	size_t image_width;

	/**
	 * Height of the image in pixels. Used for 2D and 3D images and
	 * 2D image arrays.
	 * @public
	 * */
	size_t image_height;

	/**
	 * Depth of the image in pixels. Used only for 3D images.
	 * @public
	 * */
	size_t image_depth;

	/**
	 * Number of images in the image array. Used for 1D and 2D image
	 * arrays.
	 * @public
	 * */
	size_t image_array_size;

	/**
	 * Scan-line pitch in bytes. Must be 0 if `host_ptr` is `NULL`.
	 * @public
	 * */
	size_t image_row_pitch;

	/**
	 * Size in bytes of each 2D slice in the 3D image or the size in
	 * bytes of each image in a 1D or 2D image array. Must be 0 if
	 * `host_ptr` is `NULL`.
	 * @public
	 * */
	size_t image_slice_pitch;

	/**
	 * Unused up to OpenCL 2.0.
	 * @public
	 * */
	cl_uint num_mip_levels;

	/**
	 * Unused up to OpenCL 2.0.
	 * @public
	 * */
	cl_uint num_samples;

	/**
	 * A memory object wrapper. In OpenCL 1.2, it refers to a buffer
	 * wrapper and is used for 1D image buffers. In OpenCL 2.0 it can
	 * also be used with 2D images to create a new 2D image from data
	 * shared with the specified memory object (either a buffer or a
	 * 2D image).
	 * @public
	 * */
	CCLMemObj* memobj;

} CCLImageDesc;

/* Get the image wrapper for the given OpenCL image. */
CCLImage* ccl_image_new_wrap(cl_mem mem_object);

/* Decrements the reference count of the wrapper object. If it
 * reaches 0, the wrapper object is destroyed. */
void ccl_image_destroy(CCLImage* img);

/* Creates a new image wrapper object. */
CCLImage* ccl_image_new_v(CCLContext* ctx, cl_mem_flags flags,
	const cl_image_format* image_format, const CCLImageDesc* img_dsc,
	void* host_ptr, GError** err);

/* Creates a new image wrapper object using a variable list of key-value
 * pairs which describe the image.  */
CCLImage* ccl_image_new(CCLContext* ctx, cl_mem_flags flags,
	const cl_image_format* image_format, void* host_ptr, GError** err,
	...);

/* Read from an image or image array object to host memory. */
CCLEvent* ccl_image_enqueue_read(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_read, const size_t* origin, const size_t* region,
	size_t row_pitch, size_t slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Write to an image or image array object from host memory. */
CCLEvent* ccl_image_enqueue_write(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_read, const size_t* origin, const size_t* region,
	size_t input_row_pitch, size_t input_slice_pitch, void *ptr,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Copy image objects. This function wraps the clEnqueueCopyImage()
 * OpenCL function. */
CCLEvent* ccl_image_enqueue_copy(CCLQueue* cq, CCLImage* src_img,
	CCLImage* dst_img, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Copy an image object to a buffer object. */
CCLEvent* ccl_image_enqueue_copy_to_buffer(CCLQueue* cq,
	CCLImage* src_img, CCLBuffer* dst_buf, const size_t *src_origin,
	const size_t *region, size_t dst_offset,
	CCLEventWaitList* evt_wait_lst, GError** err);

/* Map a region of the image into the host address space and return a
 * pointer to this mapped region. */
void* ccl_image_enqueue_map(CCLQueue* cq, CCLImage* img,
	cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin,
	const size_t* region, size_t *image_row_pitch,
	size_t *image_slice_pitch, CCLEventWaitList* evt_wait_lst,
	CCLEvent** evt, GError** err);

#ifdef CL_VERSION_1_2

/* Fill an image object with a specified color. This function wraps the
 * clEnqueueFillImage() OpenCL function. */
CCLEvent* ccl_image_enqueue_fill(CCLQueue* cq, CCLImage* img,
	const void *fill_color, const size_t *origin, const size_t *region,
	CCLEventWaitList* evt_wait_lst, GError** err);

#endif

/**
 * Get a ::CCLWrapperInfo image information object.
 *
 * @param[in] img The image wrapper object.
 * @param[in] param_name Name of information/parameter to get.
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested image information object. This object will
 * be automatically freed when the image wrapper object is
 * destroyed. If an error occurs, NULL is returned.
 * */
#define ccl_image_get_info(img, param_name, err) \
	ccl_wrapper_get_info((CCLWrapper*) img, NULL, param_name, \
		(ccl_wrapper_info_fp) clGetImageInfo, CL_TRUE, err)

/**
 * Macro which returns a scalar image information value.
 *
 * Use with care. In case an error occurs, zero is returned, which
 * might be ambiguous if zero is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] img The image wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. cl_uint, size_t, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested image information value. This value will be
 * automatically freed when the image wrapper object is destroyed.
 * If an error occurs, zero is returned.
 * */
#define ccl_image_get_info_scalar(img, param_name, param_type, err) \
	*((param_type*) ccl_wrapper_get_info_value((CCLWrapper*) img, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetImageInfo, \
		CL_TRUE, err))

/**
 * Macro which returns an array image information value.
 *
 * Use with care. In case an error occurs, NULL is returned, which
 * might be ambiguous if NULL is a valid return value. In this case, it
 * is necessary to check the error object.
 *
 * @param[in] img The image wrapper object.
 * @param[in] param_name Name of information/parameter to get value of.
 * @param[in] param_type Type of parameter (e.g. char*, size_t*, etc.).
 * @param[out] err Return location for a GError, or `NULL` if error
 * reporting is to be ignored.
 * @return The requested image information value. This value will be
 * automatically freed when the image wrapper object is destroyed.
 * If an error occurs, NULL is returned.
 * */
#define ccl_image_get_info_array(img, param_name, param_type, err) \
	(param_type) ccl_wrapper_get_info_value((CCLWrapper*) img, \
		NULL, param_name, (ccl_wrapper_info_fp) clGetImageInfo, \
		CL_TRUE, err)

/**
 * Increase the reference count of the image wrapper object.
 *
 * @param[in] img The image wrapper object.
 * */
#define ccl_image_ref(img) \
	ccl_wrapper_ref((CCLWrapper*) img)

/**
 * Alias to ccl_image_destroy().
 *
 * @param[in] img image wrapper object to destroy if reference count
 * is 1, otherwise just decrement the reference count.
 * */
#define ccl_image_unref(img) ccl_image_destroy(img)

/**
 * Get the OpenCL image memory object.
 *
 * @param[in] img The image wrapper object.
 * @return The OpenCL image memory object.
 * */
#define ccl_image_unwrap(img) \
	((cl_mem) ccl_wrapper_unwrap((CCLWrapper*) img))

/** @} */

#endif




