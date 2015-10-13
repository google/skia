/*
*******************************************************************************
* Copyright (C) 2013-2014, International Business Machines
* Corporation and others.  All Rights Reserved.
*******************************************************************************
* collationinfo.h
*
* created on: 2013aug05
* created by: Markus W. Scherer
*/

#ifndef __COLLATIONINFO_H__
#define __COLLATIONINFO_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_COLLATION

U_NAMESPACE_BEGIN

/**
 * Collation-related code for tools & demos.
 */
class U_TOOLUTIL_API CollationInfo /* all static */ {
public:
    static void printSizes(int32_t sizeWithHeader, const int32_t indexes[]);

private:
    CollationInfo();  // no constructor

    static int32_t getDataLength(const int32_t indexes[], int32_t startIndex);
};

U_NAMESPACE_END

#endif  // !UCONFIG_NO_COLLATION
#endif  // __COLLATIONINFO_H__
