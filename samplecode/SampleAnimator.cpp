
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

#include "SkAnimator.h"
#include "SkStream.h"
#include "SkDOM.h"

///////////////////////////////////////////////////////////////////////////////

class SkAnimatorView : public SkView {
public:
    SkAnimatorView();
    virtual ~SkAnimatorView();

    void setURIBase(const char dir[]);

    SkAnimator* getAnimator() const { return fAnimator; }

    bool    decodeFile(const char path[]);
    bool    decodeMemory(const void* buffer, size_t size);
    bool    decodeStream(SkStream* stream);

protected:
    // overrides
    virtual void onDraw(SkCanvas*);
    virtual bool onQuery(SkEvent* evt);

private:
    SkString fBaseURI;
    SkAnimator* fAnimator;

    typedef SkView INHERITED;
};

SkAnimatorView::SkAnimatorView() : fAnimator(NULL) {}

SkAnimatorView::~SkAnimatorView() {
    delete fAnimator;
}

void SkAnimatorView::setURIBase(const char dir[]) {
    fBaseURI.set(dir);
}

bool SkAnimatorView::decodeFile(const char path[]) {
    SkAutoTDelete<SkStream> is(SkStream::NewFromFile(path));
    return is.get() != NULL && this->decodeStream(is);
}

bool SkAnimatorView::decodeMemory(const void* buffer, size_t size) {
    SkMemoryStream is(buffer, size);
    return this->decodeStream(&is);
}

static const SkDOMNode* find_nodeID(const SkDOM& dom,
                        const SkDOMNode* node, const char name[]) {
    if (NULL == node) {
        node = dom.getRootNode();
    }
    do {
        const char* idval = dom.findAttr(node, "id");
        if (idval && !strcmp(idval, name)) {
            return node;
        }
        const SkDOMNode* child = dom.getFirstChild(node);
        if (child) {
            const SkDOMNode* found = find_nodeID(dom, child, name);
            if (found) {
                return found;
            }
        }
    } while ((node = dom.getNextSibling(node)) != NULL);
    return NULL;
}

bool SkAnimatorView::decodeStream(SkStream* stream) {
    delete fAnimator;
    fAnimator = new SkAnimator;
    fAnimator->setURIBase(fBaseURI.c_str());
#if 0
    if (!fAnimator->decodeStream(stream)) {
        delete fAnimator;
        fAnimator = NULL;
        return false;
    }
#else
    size_t len = stream->getLength();
    char* text = (char*)sk_malloc_throw(len);
    stream->read(text, len);
    SkDOM dom;
    const SkDOM::Node* root = dom.build(text, len);
    if (NULL == root) {
        return false;
    }
    if (!fAnimator->decodeDOM(dom, root)) {
        delete fAnimator;
        fAnimator = NULL;
        return false;
    }
    for (int i = 0; i <= 10; i++) {
        SkString name("glyph");
        name.appendS32(i);
        const SkDOM::Node* node = find_nodeID(dom, NULL, name.c_str());
        SkASSERT(node);
        SkRect r;
        dom.findScalar(node, "left", &r.fLeft);
        dom.findScalar(node, "top", &r.fTop);
        dom.findScalar(node, "width", &r.fRight); r.fRight += r.fLeft;
        dom.findScalar(node, "height", &r.fBottom); r.fBottom += r.fTop;
        SkDebugf("--- %s [%g %g %g %g]\n", name.c_str(),
                 r.fLeft, r.fTop, r.fRight, r.fBottom);
    }
#endif
    return true;
}

#include "SkTime.h"

void SkAnimatorView::onDraw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);
    if (fAnimator) {
        fAnimator->draw(canvas, 0);
#if 0
        canvas->save();
        canvas->translate(120, 30);
        canvas->scale(0.5, 0.5);
        fAnimator->draw(canvas, 0);
        canvas->restore();

        canvas->save();
        canvas->translate(190, 40);
        canvas->scale(0.25, 0.25);
        fAnimator->draw(canvas, 0);
        canvas->restore();

        this->inval(NULL);
#endif
    }
}

bool SkAnimatorView::onQuery(SkEvent* evt) {
    if (SampleCode::TitleQ(*evt)) {
        SampleCode::TitleR(evt, "Animator");
        return true;
    }
    return this->INHERITED::onQuery(evt);
}

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    SkAnimatorView* av = new SkAnimatorView;
//    av->decodeFile("/skimages/test.xml");
#if 0
    av->setURIBase("/skia/trunk/animations/");
    av->decodeFile("/skia/trunk/animations/checkbox.xml");
#else
    av->setURIBase("/");
    av->decodeFile("/testanim.txt");
#endif
    return av;
}

static SkViewRegister reg(MyFactory);
