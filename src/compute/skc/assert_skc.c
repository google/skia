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

#include "assert_skc.h"

//
//
//

#define SKC_ERR_TO_STR(err) case err: return #err

//
//
//

char const *
skc_get_error_string(skc_err const err)
{
  switch(err)
    {
      SKC_ERR_TO_STR(SKC_ERR_SUCCESS);
      SKC_ERR_TO_STR(SKC_ERR_NOT_IMPLEMENTED);
      SKC_ERR_TO_STR(SKC_ERR_POOL_EMPTY);
      SKC_ERR_TO_STR(SKC_ERR_CONDVAR_WAIT);
      SKC_ERR_TO_STR(SKC_ERR_LAYER_ID_INVALID);
      SKC_ERR_TO_STR(SKC_ERR_LAYER_NOT_EMPTY);
      SKC_ERR_TO_STR(SKC_ERR_TRANSFORM_WEAKREF_INVALID);
      SKC_ERR_TO_STR(SKC_ERR_STROKE_STYLE_WEAKREF_INVALID);
      SKC_ERR_TO_STR(SKC_ERR_COMMAND_NOT_READY);
      SKC_ERR_TO_STR(SKC_ERR_COMMAND_NOT_COMPLETED);
      SKC_ERR_TO_STR(SKC_ERR_COMMAND_NOT_STARTED);
      SKC_ERR_TO_STR(SKC_ERR_COMMAND_NOT_READY_OR_COMPLETED);
      SKC_ERR_TO_STR(SKC_ERR_COMPOSITION_SEALED);
      SKC_ERR_TO_STR(SKC_ERR_STYLING_SEALED);
      SKC_ERR_TO_STR(SKC_ERR_HANDLE_INVALID);
      SKC_ERR_TO_STR(SKC_ERR_HANDLE_OVERFLOW);

    default:
      return "UNKNOWN SKC ERROR CODE";
    }
}

//
//
//

skc_err
assert_skc(skc_err const err, char const * const file, int const line, bool const abort)
{
  if (err != SKC_ERR_SUCCESS)
    {
      char const * const skc_err_str = skc_get_error_string(err);

      fprintf(stderr,
              "\"%s\", line %d: skc_assert (%d) = \"%s\"",
              file,line,err,skc_err_str);

      if (abort)
        {
          // stop profiling and reset device here if necessary
          exit(err);
        }
    }

  return err;
}

//
//
//
