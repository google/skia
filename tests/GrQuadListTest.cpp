/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "src/gpu/geometry/GrQuadList.h"

#define ASSERT(cond) REPORTER_ASSERT(r, cond)
#define ASSERTF(cond, ...) REPORTER_ASSERT(r, cond, __VA_ARGS__)
#define TEST(name) DEF_TEST(GrQuadList##name, r)

struct TestData {
    int fItem1;
    float fItem2;
};

// Simple factories to make placeholder quads used in the tests. The 2D quads
// will have the kRect quad type.
static GrQuad make_2d_quad() {
    return GrQuad(SkRect::MakeLTRB(1.f, 2.f, 3.f, 4.f));
}
static bool is_2d_quad(const GrQuad& quad) {
    return quad.x(0) == 1.f && quad.x(1) == 1.f && quad.x(2) == 3.f && quad.x(3) == 3.f &&
           quad.y(0) == 2.f && quad.y(1) == 4.f && quad.y(2) == 2.f && quad.y(3) == 4.f &&
           quad.w(0) == 1.f && quad.w(1) == 1.f && quad.w(2) == 1.f && quad.w(3) == 1.f;
}

static GrQuad make_3d_quad() {
    // This perspective matrix leaves x and y unmodified, and sets w to the persp2 value
    SkMatrix p = SkMatrix::I();
    p[SkMatrix::kMPersp2] = 13.f;
    SkASSERT(p.hasPerspective()); // Sanity check
    return GrQuad::MakeFromRect(SkRect::MakeLTRB(9.f, 10.f, 11.f, 12.f), p);
}
static bool is_3d_quad(const GrQuad& quad) {
    return quad.x(0) == 9.f && quad.x(1) == 9.f && quad.x(2) == 11.f && quad.x(3) == 11.f &&
           quad.y(0) == 10.f && quad.y(1) == 12.f && quad.y(2) == 10.f && quad.y(3) == 12.f &&
           quad.w(0) == 13.f && quad.w(1) == 13.f && quad.w(2) == 13.f && quad.w(3) == 13.f;
}

TEST(Add2D) {
    GrQuadList list2D;
    // Add a plain quad and then a 3D persp quad, then read back and make sure
    // the coordinates make sense (including that the type was lifted to perspective).
    list2D.push_back(make_2d_quad());

    // Check 2D state of the list
    ASSERTF(list2D.count() == 1, "Unexpected count: %d", list2D.count());
    ASSERTF(list2D.quadType() == GrQuad::Type::kAxisAligned, "Unexpected quad type: %d",
            (uint32_t) list2D.quadType());
    ASSERTF(is_2d_quad(list2D[0]), "Incorrect quad at i=0");

    // Force the 2D quads to be updated to store ws by adding a perspective quad
    list2D.push_back(make_3d_quad());
    ASSERTF(list2D.quadType() == GrQuad::Type::kPerspective,
            "Expected 2D list to be upgraded to perspective");

    // Re-check full state of list after type upgrade
    ASSERTF(list2D.count() == 2, "Unexpected count: %d", list2D.count());
    ASSERTF(is_2d_quad(list2D[0]), "Incorrect quad at i=0 after upgrade");
    ASSERTF(is_3d_quad(list2D[1]), "Incorrect quad at i=1");
}

TEST(Add3D) {
    // Now make a list that starts with a 3D persp quad, then has conventional quads added to it
    // and make sure its state is correct
    GrQuadList list3D;
    list3D.push_back(make_3d_quad());
    list3D.push_back(make_2d_quad());

    ASSERTF(list3D.count() == 2, "Unexpected count: %d", list3D.count());
    ASSERTF(is_3d_quad(list3D[0]), "Incorrect quad at i=0");
    ASSERTF(is_2d_quad(list3D[1]), "Incorrect quad at i=2");
}

TEST(AddWithMetadata2D) {
    // As above, but also make sure that the metadata is saved and read properly
    GrTQuadList<TestData> list2D;
    // Add two plain quads, and then a 3D persp quad, then read back and make sure
    // the coordinates make sense (including that the type was lifted to perspective).
    list2D.push_back(make_2d_quad(), {1, 1.f});
    list2D.push_back(make_2d_quad(), {2, 2.f});

    // Check 2D state of the list
    ASSERTF(list2D.count() == 2, "Unexpected count: %d", list2D.count());
    ASSERTF(list2D.quadType() == GrQuad::Type::kAxisAligned, "Unexpected quad type: %d",
            (uint32_t) list2D.quadType());
    ASSERTF(is_2d_quad(list2D[0]), "Incorrect quad at i=0");
    ASSERTF(list2D.metadata(0).fItem1 == 1 && list2D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_2d_quad(list2D[1]), "Incorrect quad at i=1");
    ASSERTF(list2D.metadata(1).fItem1 == 2 && list2D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");

    // Force the 2D quads to be updated to store ws by adding a perspective quad
    list2D.push_back(make_3d_quad(), {3, 3.f});
    ASSERTF(list2D.quadType() == GrQuad::Type::kPerspective,
            "Expected 2D list to be upgraded to perspective");

    // Re-check full state of list after type upgrade
    ASSERTF(list2D.count() == 3, "Unexpected count: %d", list2D.count());
    ASSERTF(is_2d_quad(list2D[0]), "Incorrect quad at i=0 after upgrade");
    ASSERTF(list2D.metadata(0).fItem1 == 1 && list2D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_2d_quad(list2D[1]), "Incorrect quad at i=1 after upgrade");
    ASSERTF(list2D.metadata(1).fItem1 == 2 && list2D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
    ASSERTF(is_3d_quad(list2D[2]), "Incorrect quad at i=2");
    ASSERTF(list2D.metadata(2).fItem1 == 3 && list2D.metadata(2).fItem2 == 3.f,
            "Incorrect metadata at i=2");
}

TEST(AddWithMetadata3D) {
    // Now make a list that starts with a 3D persp quad, then has conventional quads added to it
    // and make sure its state is correct
    GrTQuadList<TestData> list3D;
    list3D.push_back(make_3d_quad(), {3, 3.f});
    list3D.push_back(make_2d_quad(), {2, 2.f});
    list3D.push_back(make_2d_quad(), {1, 1.f});

    ASSERTF(list3D.count() == 3, "Unexpected count: %d", list3D.count());
    ASSERTF(is_3d_quad(list3D[0]), "Incorrect quad at i=0");
    ASSERTF(list3D.metadata(0).fItem1 == 3 && list3D.metadata(0).fItem2 == 3.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_2d_quad(list3D[1]), "Incorrect quad at i=1");
    ASSERTF(list3D.metadata(1).fItem1 == 2 && list3D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
    ASSERTF(is_2d_quad(list3D[2]), "Incorrect quad at i=2");
    ASSERTF(list3D.metadata(2).fItem1 == 1 && list3D.metadata(2).fItem2 == 1.f,
            "Incorrect metadata at i=2");
}

TEST(Concat2DWith2D) {
    GrQuadList a2D;
    a2D.push_back(make_2d_quad());
    GrQuadList b2D;
    b2D.push_back(make_2d_quad());

    a2D.concat(b2D);

    ASSERTF(a2D.count() == 2, "Unexpected count: %d", a2D.count());
    ASSERTF(is_2d_quad(a2D[0]), "Incorrect quad at i=0");
    ASSERTF(is_2d_quad(a2D[1]), "Incorrect quad at i=1");
}

TEST(Concat2DWith3D) {
    GrQuadList a2D;
    a2D.push_back(make_2d_quad());
    GrQuadList b3D;
    b3D.push_back(make_3d_quad());

    a2D.concat(b3D);

    ASSERTF(a2D.count() == 2, "Unexpected count: %d", a2D.count());
    ASSERTF(is_2d_quad(a2D[0]), "Incorrect quad at i=0");
    ASSERTF(is_3d_quad(a2D[1]), "Incorrect quad at i=1");
}

TEST(Concat3DWith2D) {
    GrQuadList a3D;
    a3D.push_back(make_3d_quad());
    GrQuadList b2D;
    b2D.push_back(make_2d_quad());

    a3D.concat(b2D);

    ASSERTF(a3D.count() == 2, "Unexpected count: %d", a3D.count());
    ASSERTF(is_3d_quad(a3D[0]), "Incorrect quad at i=0");
    ASSERTF(is_2d_quad(a3D[1]), "Incorrect quad at i=1");
}

TEST(Concat3DWith3D) {
    GrQuadList a3D;
    a3D.push_back(make_3d_quad());
    GrQuadList b3D;
    b3D.push_back(make_3d_quad());

    a3D.concat(b3D);

    ASSERTF(a3D.count() == 2, "Unexpected count: %d", a3D.count());
    ASSERTF(is_3d_quad(a3D[0]), "Incorrect quad at i=0");
    ASSERTF(is_3d_quad(a3D[1]), "Incorrect quad at i=1");
}

TEST(Concat2DWith2DMetadata) {
    GrTQuadList<TestData> a2D;
    a2D.push_back(make_2d_quad(), {1, 1.f});
    GrTQuadList<TestData> b2D;
    b2D.push_back(make_2d_quad(), {2, 2.f});

    a2D.concat(b2D);

    ASSERTF(a2D.count() == 2, "Unexpected count: %d", a2D.count());
    ASSERTF(is_2d_quad(a2D[0]), "Incorrect quad at i=0");
    ASSERTF(a2D.metadata(0).fItem1 == 1 && a2D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_2d_quad(a2D[1]), "Incorrect quad at i=1");
    ASSERTF(a2D.metadata(1).fItem1 == 2 && a2D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
}

TEST(Concat2DWith3DMetadata) {
    GrTQuadList<TestData> a2D;
    a2D.push_back(make_2d_quad(), {1, 1.f});
    GrTQuadList<TestData> b3D;
    b3D.push_back(make_3d_quad(), {2, 2.f});

    a2D.concat(b3D);

    ASSERTF(a2D.count() == 2, "Unexpected count: %d", a2D.count());
    ASSERTF(is_2d_quad(a2D[0]), "Incorrect quad at i=0");
    ASSERTF(a2D.metadata(0).fItem1 == 1 && a2D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_3d_quad(a2D[1]), "Incorrect quad at i=1");
    ASSERTF(a2D.metadata(1).fItem1 == 2 && a2D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
}

TEST(Concat3DWith2DMetadata) {
    GrTQuadList<TestData> a3D;
    a3D.push_back(make_3d_quad(), {1, 1.f});
    GrTQuadList<TestData> b2D;
    b2D.push_back(make_2d_quad(), {2, 2.f});

    a3D.concat(b2D);

    ASSERTF(a3D.count() == 2, "Unexpected count: %d", a3D.count());
    ASSERTF(is_3d_quad(a3D[0]), "Incorrect quad at i=0");
    ASSERTF(a3D.metadata(0).fItem1 == 1 && a3D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_2d_quad(a3D[1]), "Incorrect quad at i=1");
    ASSERTF(a3D.metadata(1).fItem1 == 2 && a3D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
}

TEST(Concat3DWith3DMetadata) {
    GrTQuadList<TestData> a3D;
    a3D.push_back(make_3d_quad(), {1, 1.f});
    GrTQuadList<TestData> b3D;
    b3D.push_back(make_3d_quad(), {2, 2.f});

    a3D.concat(b3D);

    ASSERTF(a3D.count() == 2, "Unexpected count: %d", a3D.count());
    ASSERTF(is_3d_quad(a3D[0]), "Incorrect quad at i=0");
    ASSERTF(a3D.metadata(0).fItem1 == 1 && a3D.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0");
    ASSERTF(is_3d_quad(a3D[1]), "Incorrect quad at i=1");
    ASSERTF(a3D.metadata(1).fItem1 == 2 && a3D.metadata(1).fItem2 == 2.f,
            "Incorrect metadata at i=1");
}

TEST(WriteMetadata) {
    GrTQuadList<TestData> list;
    list.push_back(make_2d_quad(), {1, 1.f});
    ASSERTF(list.metadata(0).fItem1 == 1 && list.metadata(0).fItem2 == 1.f,
            "Incorrect metadata at i=0"); // Sanity check

    // Rewrite metadata within the list and read back
    list.metadata(0).fItem1 = 2;
    list.metadata(0).fItem2 = 2.f;
    ASSERTF(list.metadata(0).fItem1 == 2 && list.metadata(0).fItem2 == 2.f,
            "Incorrect metadata at i=0 after edit");
}
