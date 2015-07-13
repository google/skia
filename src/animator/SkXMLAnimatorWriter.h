
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
    void writeHeader() override;
    SkDEBUGCODE(static void UnitTest(class SkCanvas* canvas);)

protected:
    void onAddAttributeLen(const char name[], const char value[], size_t length) override;
    void onEndElement() override;
    void onStartElementLen(const char elem[], size_t length) override;
    void onAddText(const char text[], size_t length) override;

private:
    SkAnimator* fAnimator;
    SkDisplayXMLParser* fParser;
};

#endif // SkXMLAnimatorWriter_DEFINED
