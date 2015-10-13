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

/***************************************************************************
* Module form_request_name                                                 *
* Routines to handle external names of menu requests                       *
***************************************************************************/

#include "form.priv.h"

MODULE_ID("$Id$")

static const char *request_names[ MAX_FORM_COMMAND - MIN_FORM_COMMAND + 1 ] = {
  "NEXT_PAGE"	 ,
  "PREV_PAGE"	 ,
  "FIRST_PAGE"	 ,
  "LAST_PAGE"	 ,

  "NEXT_FIELD"	 ,
  "PREV_FIELD"	 ,
  "FIRST_FIELD"	 ,
  "LAST_FIELD"	 ,
  "SNEXT_FIELD"	 ,
  "SPREV_FIELD"	 ,
  "SFIRST_FIELD" ,
  "SLAST_FIELD"	 ,
  "LEFT_FIELD"	 ,
  "RIGHT_FIELD"	 ,
  "UP_FIELD"	 ,
  "DOWN_FIELD"	 ,

  "NEXT_CHAR"	 ,
  "PREV_CHAR"	 ,
  "NEXT_LINE"	 ,
  "PREV_LINE"	 ,
  "NEXT_WORD"	 ,
  "PREV_WORD"	 ,
  "BEG_FIELD"	 ,
  "END_FIELD"	 ,
  "BEG_LINE"	 ,
  "END_LINE"	 ,
  "LEFT_CHAR"	 ,
  "RIGHT_CHAR"	 ,
  "UP_CHAR"	 ,
  "DOWN_CHAR"	 ,

  "NEW_LINE"	 ,
  "INS_CHAR"	 ,
  "INS_LINE"	 ,
  "DEL_CHAR"	 ,
  "DEL_PREV"	 ,
  "DEL_LINE"	 ,
  "DEL_WORD"	 ,
  "CLR_EOL"	 ,
  "CLR_EOF"	 ,
  "CLR_FIELD"	 ,
  "OVL_MODE"	 ,
  "INS_MODE"	 ,
  "SCR_FLINE"	 ,
  "SCR_BLINE"	 ,
  "SCR_FPAGE"	 ,
  "SCR_BPAGE"	 ,
  "SCR_FHPAGE"   ,
  "SCR_BHPAGE"   ,
  "SCR_FCHAR"    ,
  "SCR_BCHAR"    ,
  "SCR_HFLINE"   ,
  "SCR_HBLINE"   ,
  "SCR_HFHALF"   ,
  "SCR_HBHALF"   ,

  "VALIDATION"	 ,
  "NEXT_CHOICE"	 ,
  "PREV_CHOICE"	 
};
#define A_SIZE (sizeof(request_names)/sizeof(request_names[0]))

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  const char * form_request_name (int request);
|   
|   Description   :  Get the external name of a form request.
|
|   Return Values :  Pointer to name      - on success
|                    NULL                 - on invalid request code
+--------------------------------------------------------------------------*/
const char *form_request_name( int request )
{
  if ( (request < MIN_FORM_COMMAND) || (request > MAX_FORM_COMMAND) )
    {
      SET_ERROR (E_BAD_ARGUMENT);
      return (const char *)0;
    }
  else
    return request_names[ request - MIN_FORM_COMMAND ];
}


/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int form_request_by_name (const char *str);
|   
|   Description   :  Search for a request with this name.
|
|   Return Values :  Request Id       - on success
|                    E_NO_MATCH       - request not found
+--------------------------------------------------------------------------*/
int form_request_by_name( const char *str )
{ 
  /* because the table is so small, it doesn't really hurt
     to run sequentially through it.
  */
  unsigned int i = 0;
  char buf[16];
  
  if (str)
    {
      strncpy(buf,str,sizeof(buf));
      while( (i<sizeof(buf)) && (buf[i] != '\0') )
	{
	  buf[i] = toupper((int)(buf[i]));
	  i++;
	}
      
      for (i=0; i < A_SIZE; i++)
	{
	  if (strncmp(request_names[i],buf,sizeof(buf))==0)
	    return MIN_FORM_COMMAND + i;
	} 
    }
  RETURN(E_NO_MATCH);
}

/* frm_req_name.c ends here */
