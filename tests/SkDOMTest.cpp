/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if defined(SK_XML)

#include "SkDOM.h"

DEF_TEST(SkDOM_test, r) {
    static const char gDoc[] =
        "<root a='1' b='2'>"
            "<elem1 c='3' />"
            "<elem2 d='4' />"
            "<elem3 e='5'>"
                "<subelem1/>"
                "<subelem2 f='6' g='7'/>"
            "</elem3>"
            "<elem4 h='8'/>"
        "</root>"
        ;

    SkDOM   dom;
    REPORTER_ASSERT(r, !dom.getRootNode());

    const SkDOM::Node* root = dom.build(gDoc, sizeof(gDoc) - 1);
    REPORTER_ASSERT(r, root && dom.getRootNode() == root);

    const char* v = dom.findAttr(root, "a");
    REPORTER_ASSERT(r, v && !strcmp(v, "1"));
    v = dom.findAttr(root, "b");
    REPORTER_ASSERT(r, v && !strcmp(v, "2"));
    v = dom.findAttr(root, "c");
    REPORTER_ASSERT(r, v == nullptr);

    REPORTER_ASSERT(r, dom.getFirstChild(root, "elem1"));
    REPORTER_ASSERT(r, !dom.getFirstChild(root, "subelem1"));
}

#endif // SK_XML
