/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include <stdlib.h>
#include <stdio.h>

//
//
//

#include "assert_cl.h"

//
//
//

#define CL_VAL_TO_STRING(err) case err: return #err

//
//
//

#define CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR  -1000

//
//
//

char const *
cl_get_error_string(cl_int const err)
{
  switch (err)
    {
      CL_VAL_TO_STRING(CL_SUCCESS);
      CL_VAL_TO_STRING(CL_DEVICE_NOT_FOUND);
      CL_VAL_TO_STRING(CL_DEVICE_NOT_AVAILABLE);
      CL_VAL_TO_STRING(CL_COMPILER_NOT_AVAILABLE);
      CL_VAL_TO_STRING(CL_MEM_OBJECT_ALLOCATION_FAILURE);
      CL_VAL_TO_STRING(CL_OUT_OF_RESOURCES);
      CL_VAL_TO_STRING(CL_OUT_OF_HOST_MEMORY);
      CL_VAL_TO_STRING(CL_PROFILING_INFO_NOT_AVAILABLE);
      CL_VAL_TO_STRING(CL_MEM_COPY_OVERLAP);
      CL_VAL_TO_STRING(CL_IMAGE_FORMAT_MISMATCH);
      CL_VAL_TO_STRING(CL_IMAGE_FORMAT_NOT_SUPPORTED);
      CL_VAL_TO_STRING(CL_BUILD_PROGRAM_FAILURE);
      CL_VAL_TO_STRING(CL_MAP_FAILURE);
      CL_VAL_TO_STRING(CL_MISALIGNED_SUB_BUFFER_OFFSET);
      CL_VAL_TO_STRING(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
      CL_VAL_TO_STRING(CL_COMPILE_PROGRAM_FAILURE);
      CL_VAL_TO_STRING(CL_LINKER_NOT_AVAILABLE);
      CL_VAL_TO_STRING(CL_LINK_PROGRAM_FAILURE);
      CL_VAL_TO_STRING(CL_DEVICE_PARTITION_FAILED);
      CL_VAL_TO_STRING(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
      CL_VAL_TO_STRING(CL_INVALID_VALUE);
      CL_VAL_TO_STRING(CL_INVALID_DEVICE_TYPE);
      CL_VAL_TO_STRING(CL_INVALID_PLATFORM);
      CL_VAL_TO_STRING(CL_INVALID_DEVICE);
      CL_VAL_TO_STRING(CL_INVALID_CONTEXT);
      CL_VAL_TO_STRING(CL_INVALID_QUEUE_PROPERTIES);
      CL_VAL_TO_STRING(CL_INVALID_COMMAND_QUEUE);
      CL_VAL_TO_STRING(CL_INVALID_HOST_PTR);
      CL_VAL_TO_STRING(CL_INVALID_MEM_OBJECT);
      CL_VAL_TO_STRING(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
      CL_VAL_TO_STRING(CL_INVALID_IMAGE_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_SAMPLER);
      CL_VAL_TO_STRING(CL_INVALID_BINARY);
      CL_VAL_TO_STRING(CL_INVALID_BUILD_OPTIONS);
      CL_VAL_TO_STRING(CL_INVALID_PROGRAM);
      CL_VAL_TO_STRING(CL_INVALID_PROGRAM_EXECUTABLE);
      CL_VAL_TO_STRING(CL_INVALID_KERNEL_NAME);
      CL_VAL_TO_STRING(CL_INVALID_KERNEL_DEFINITION);
      CL_VAL_TO_STRING(CL_INVALID_KERNEL);
      CL_VAL_TO_STRING(CL_INVALID_ARG_INDEX);
      CL_VAL_TO_STRING(CL_INVALID_ARG_VALUE);
      CL_VAL_TO_STRING(CL_INVALID_ARG_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_KERNEL_ARGS);
      CL_VAL_TO_STRING(CL_INVALID_WORK_DIMENSION);
      CL_VAL_TO_STRING(CL_INVALID_WORK_GROUP_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_WORK_ITEM_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_GLOBAL_OFFSET);
      CL_VAL_TO_STRING(CL_INVALID_EVENT_WAIT_LIST);
      CL_VAL_TO_STRING(CL_INVALID_EVENT);
      CL_VAL_TO_STRING(CL_INVALID_OPERATION);
      CL_VAL_TO_STRING(CL_INVALID_GL_OBJECT);
      CL_VAL_TO_STRING(CL_INVALID_BUFFER_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_MIP_LEVEL);
      CL_VAL_TO_STRING(CL_INVALID_GLOBAL_WORK_SIZE);
      CL_VAL_TO_STRING(CL_INVALID_PROPERTY);
      CL_VAL_TO_STRING(CL_INVALID_IMAGE_DESCRIPTOR);
      CL_VAL_TO_STRING(CL_INVALID_COMPILER_OPTIONS);
      CL_VAL_TO_STRING(CL_INVALID_LINKER_OPTIONS);
      CL_VAL_TO_STRING(CL_INVALID_DEVICE_PARTITION_COUNT);
      // CL_VAL_TO_STRING(CL_INVALID_PIPE_SIZE);
      // CL_VAL_TO_STRING(CL_INVALID_DEVICE_QUEUE);

      //
      // Extensions:
      //
      //   cl_khr_gl_sharing
      //
      CL_VAL_TO_STRING(CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR);

      //
      //
      //
    default:
      return "UNKNOWN ERROR CODE";
    }
}

//
//
//

cl_int
assert_cl(cl_int const code, char const * const file, int const line, bool const abort)
{
  if (code != CL_SUCCESS)
    {
      char const * const cl_err_str = cl_get_error_string(code);

      fprintf(stderr,
              "\"%s\", line %d: assert_cl( %d ) = \"%s\"",
              file,line,code,cl_err_str);

      if (abort)
        {
          // stop profiling and reset device here if necessary
          exit(code);
        }
    }

  return code;
}

//
//
//

void
cl_get_event_info(cl_event                event,
                  cl_int          * const status,
                  cl_command_type * const type)
{
  if (status != NULL) {
    cl(GetEventInfo(event,
                    CL_EVENT_COMMAND_EXECUTION_STATUS,
                    sizeof(*status),
                    status,
                    NULL));
  }

  if (type != NULL) {
    cl(GetEventInfo(event,
                    CL_EVENT_COMMAND_TYPE,
                    sizeof(*type),
                    type,
                    NULL));
  }
}


char const *
cl_get_event_command_status_string(cl_int const status)
{
  switch (status)
    {
      CL_VAL_TO_STRING(CL_QUEUED);
      CL_VAL_TO_STRING(CL_SUBMITTED);
      CL_VAL_TO_STRING(CL_RUNNING);
      CL_VAL_TO_STRING(CL_COMPLETE);

    default:
      return "UNKNOWN COMMAND STATUS";
    }
}

char const *
cl_get_event_command_type_string(cl_command_type const type)
{
  switch (type)
    {
      CL_VAL_TO_STRING(CL_COMMAND_NDRANGE_KERNEL);
      CL_VAL_TO_STRING(CL_COMMAND_TASK);
      CL_VAL_TO_STRING(CL_COMMAND_NATIVE_KERNEL);
      CL_VAL_TO_STRING(CL_COMMAND_READ_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_WRITE_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_COPY_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_READ_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_WRITE_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_COPY_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_MAP_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_MAP_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_UNMAP_MEM_OBJECT);
      CL_VAL_TO_STRING(CL_COMMAND_MARKER);
      CL_VAL_TO_STRING(CL_COMMAND_ACQUIRE_GL_OBJECTS);
      CL_VAL_TO_STRING(CL_COMMAND_RELEASE_GL_OBJECTS);
      CL_VAL_TO_STRING(CL_COMMAND_READ_BUFFER_RECT);
      CL_VAL_TO_STRING(CL_COMMAND_WRITE_BUFFER_RECT);
      CL_VAL_TO_STRING(CL_COMMAND_COPY_BUFFER_RECT);
      CL_VAL_TO_STRING(CL_COMMAND_USER);
      CL_VAL_TO_STRING(CL_COMMAND_BARRIER);
      CL_VAL_TO_STRING(CL_COMMAND_MIGRATE_MEM_OBJECTS);
      CL_VAL_TO_STRING(CL_COMMAND_FILL_BUFFER);
      CL_VAL_TO_STRING(CL_COMMAND_FILL_IMAGE);
      CL_VAL_TO_STRING(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR);

    default:
      return "UNKNOWN EVENT COMMAND TYPE";
    }
}
