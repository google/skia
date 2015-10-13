/*
*******************************************************************************
* Copyright (C) 2011, International Business Machines Corporation and
* others. All Rights Reserved.                                                *
*******************************************************************************
*
* File NUMSYS_IMPL.H
*
*******************************************************************************
*/

#ifndef __NUMSYS_IMPL_H__
#define __NUMSYS_IMPL_H__

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING
    
#include "unicode/numsys.h"
#include "uvector.h"
#include "unicode/strenum.h"

U_NAMESPACE_BEGIN

class NumsysNameEnumeration : public StringEnumeration {
public:
    NumsysNameEnumeration(UVector *fNumsysNames, UErrorCode& status);
    virtual ~NumsysNameEnumeration();
    static UClassID U_EXPORT2 getStaticClassID(void);
    virtual UClassID getDynamicClassID(void) const;
    virtual const UnicodeString* snext(UErrorCode& status);
    virtual void reset(UErrorCode& status);
    virtual int32_t count(UErrorCode& status) const;
private:
    int32_t pos;
    UVector *fNumsysNames;
};

U_NAMESPACE_END

#endif

#endif
