/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "tests/Test.h"

#if defined(SK_XML)

#include "src/xml/SkDOM.h"

static const SkDOM::Node* check_node(skiatest::Reporter* r, const SkDOM& dom,
                                     const SkDOM::Node* node, const char* expectedName,
                                     SkDOM::Type expectedType) {
    REPORTER_ASSERT(r, node);
    if (node) {
        REPORTER_ASSERT(r, !strcmp(dom.getName(node), expectedName));
        REPORTER_ASSERT(r, dom.getType(node) == expectedType);
    }
    return node;
}

DEF_TEST(SkDOM_test, r) {
    static const char gDoc[] =
        "<root a='1' b='2'>"
            "<elem1 c='3' />"
            "<elem2 d='4' />"
            "<elem3 e='5'>"
                "<subelem1>Some text.</subelem1>"
                "<subelem2 f='6' g='7'/>"
                "<subelem3>Some more text.</subelem3>"
            "</elem3>"
            "<elem4 h='8'/>"
        "</root>"
        ;

    SkMemoryStream docStream(gDoc, sizeof(gDoc) - 1);

    SkDOM   dom;
    REPORTER_ASSERT(r, !dom.getRootNode());

    const SkDOM::Node* root = dom.build(docStream);
    REPORTER_ASSERT(r, root && dom.getRootNode() == root);

    const char* v = dom.findAttr(root, "a");
    REPORTER_ASSERT(r, v && !strcmp(v, "1"));
    v = dom.findAttr(root, "b");
    REPORTER_ASSERT(r, v && !strcmp(v, "2"));
    v = dom.findAttr(root, "c");
    REPORTER_ASSERT(r, v == nullptr);

    REPORTER_ASSERT(r, dom.getFirstChild(root, "elem1"));
    REPORTER_ASSERT(r, !dom.getFirstChild(root, "subelem1"));

    {
        const auto* elem1 = check_node(r, dom, dom.getFirstChild(root),
                                       "elem1", SkDOM::kElement_Type);
        const auto* elem2 = check_node(r, dom, dom.getNextSibling(elem1),
                                       "elem2", SkDOM::kElement_Type);
        const auto* elem3 = check_node(r, dom, dom.getNextSibling(elem2),
                                       "elem3", SkDOM::kElement_Type);
        {
            const auto* subelem1 = check_node(r, dom, dom.getFirstChild(elem3),
                                              "subelem1", SkDOM::kElement_Type);
            {
                check_node(r, dom, dom.getFirstChild(subelem1),
                           "Some text.", SkDOM::kText_Type);
            }
            const auto* subelem2 = check_node(r, dom, dom.getNextSibling(subelem1),
                                              "subelem2", SkDOM::kElement_Type);
            const auto* subelem3 = check_node(r, dom, dom.getNextSibling(subelem2),
                                              "subelem3", SkDOM::kElement_Type);
            {
                check_node(r, dom, dom.getFirstChild(subelem3),
                           "Some more text.", SkDOM::kText_Type);
            }
        }
        check_node(r, dom, dom.getNextSibling(elem3),
                   "elem4", SkDOM::kElement_Type);
    }
}

#endif // SK_XML
