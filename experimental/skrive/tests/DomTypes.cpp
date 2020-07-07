/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"
#include "tests/Test.h"

template <typename T, typename U>
void check_type(skiatest::Reporter* reporter, const sk_sp<U>& node, bool expected) {
    REPORTER_ASSERT(reporter, node->template is<T>() == expected);
    REPORTER_ASSERT(reporter, !!static_cast<const T*>(*node.get()) == expected);
}

DEF_TEST(SkRive_DomTypes, reporter) {
    {
        auto node = sk_make_sp<skrive::Node>();

        check_type<skrive::Component             >(reporter, node, true);
        check_type<skrive::TransformableComponent>(reporter, node, true);
        check_type<skrive::Node                  >(reporter, node, true);
        check_type<skrive::Drawable              >(reporter, node, false);
        check_type<skrive::Shape                 >(reporter, node, false);
    }

    {
        auto node = sk_make_sp<skrive::Shape>();

        check_type<skrive::Component             >(reporter, node, true);
        check_type<skrive::TransformableComponent>(reporter, node, true);
        check_type<skrive::Node                  >(reporter, node, true);
        check_type<skrive::Drawable              >(reporter, node, true);
        check_type<skrive::Shape                 >(reporter, node, true);
    }

    {
        auto node = sk_make_sp<skrive::ColorPaint>(SkPaint::Style::kFill_Style);

        check_type<skrive::Component             >(reporter, node, true);
        check_type<skrive::TransformableComponent>(reporter, node, false);
        check_type<skrive::Node                  >(reporter, node, false);
        check_type<skrive::Drawable              >(reporter, node, false);
        check_type<skrive::Shape                 >(reporter, node, false);
        check_type<skrive::Paint                 >(reporter, node, true );
        check_type<skrive::ColorPaint            >(reporter, node, true );
    }

    {
        auto node = sk_make_sp<skrive::Ellipse>();

        check_type<skrive::Component             >(reporter, node, true);
        check_type<skrive::TransformableComponent>(reporter, node, true);
        check_type<skrive::Node                  >(reporter, node, true);
        check_type<skrive::Drawable              >(reporter, node, false);
        check_type<skrive::Shape                 >(reporter, node, false);
        check_type<skrive::Paint                 >(reporter, node, false );
        check_type<skrive::ColorPaint            >(reporter, node, false );
        check_type<skrive::Geometry              >(reporter, node, true);
        check_type<skrive::Ellipse               >(reporter, node, true);
    }
}
