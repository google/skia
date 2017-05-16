/****************************************************************************
 * Copyright (c) 1998 Free Software Foundation, Inc.                        *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 *   Author: Juergen Pfeifer <juergen.pfeifer@gmx.net> 1995,1997            *
 ****************************************************************************/

#include "form.priv.h"

MODULE_ID("$Id$")

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_field_status(FIELD *field, bool status)
|   
|   Description   :  Set or clear the 'changed' indication flag for that
|                    fields primary buffer.
|
|   Return Values :  E_OK            - success
+--------------------------------------------------------------------------*/
int set_field_status(FIELD * field, bool status)
{
  Normalize_Field( field );

  if (status)
    field->status |= _CHANGED;
  else
    field->status &= ~_CHANGED;

  return(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  bool field_status(const FIELD *field)
|   
|   Description   :  Retrieve the value of the 'changed' indication flag
|                    for that fields primary buffer. 
|
|   Return Values :  TRUE  - buffer has been changed
|                    FALSE - buffer has not been changed
+--------------------------------------------------------------------------*/
bool field_status(const FIELD * field)
{
  return ((Normalize_Field(field)->status & _CHANGED) ? TRUE : FALSE);
}

/* fld_stat.c ends here */
