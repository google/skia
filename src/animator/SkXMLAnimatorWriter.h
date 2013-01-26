
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkXMLAnimatorWriter_DEFINED
#define SkXMLAnimatorWriter_DEFINED

#include "SkXMLWriter.h"

class SkAnimator;
class SkDisplayXMLParser;

class SkXMLAnimatorWriter : public SkXMLWriter {
public:
    SkXMLAnimatorWriter(SkAnimator*);
    virtual ~SkXMLAnimatorWriter();
    virtual void    writeHeader();
    SkDEBUGCODE(static void UnitTest(class SkCanvas* canvas);)
protected:
    virtual void onAddAttributeLen(const char name[], const char value[], size_t length);
    virtual void onEndElement();
    virtual void onStartElementLen(const char elem[], size_t length);
private:
    SkAnimator* fAnimator;
    SkDisplayXMLParser* fParser;
};

#endif // SkXMLAnimatorWriter_DEFINED
