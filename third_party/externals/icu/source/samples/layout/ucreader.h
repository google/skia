/*
 *
 * (C) Copyright IBM Corp. 1998-2007 - All Rights Reserved
 *
 */

#ifndef __UCREADER_H
#define __UCREADER_H

#include "unicode/utypes.h"
#include "gsupport.h"

U_CDECL_BEGIN

const UChar *uc_readFile(const char *fileName, gs_guiSupport *guiSupport, int32_t *charCount);

U_CDECL_END

#endif
