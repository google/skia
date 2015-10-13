/*
 * Copyright 2011 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#define ENABLE_OBJECT_COUNTER
#include "sfntly/port/refcount.h"

using sfntly::RefCounted;
using sfntly::Ptr;

class Foo : public RefCounted<Foo> {
public:  // put in something to make sure it's not empty
  int foo_;
  int foo() { return foo_; }
};

bool TestSmartPointer() {
  // scope out allocation
  {
    Ptr<Foo> p1;
    p1 = new Foo();
    EXPECT_EQ(size_t(1), p1->ref_count_);
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);

    Ptr<Foo> p2;
    p2 = p1;
    EXPECT_EQ(size_t(2), p1->ref_count_);
    EXPECT_EQ(size_t(2), p2->ref_count_);
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);

    Ptr<Foo> p3;
    p3 = p1;
    EXPECT_EQ(size_t(3), p1->ref_count_);
    EXPECT_EQ(size_t(3), p2->ref_count_);
    EXPECT_EQ(size_t(3), p3->ref_count_);
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);

    p2 = new Foo();
    EXPECT_EQ(size_t(2), p1->ref_count_);
    EXPECT_EQ(size_t(1), p2->ref_count_);
    EXPECT_EQ(size_t(2), p3->ref_count_);
    EXPECT_EQ(size_t(2), RefCounted<Foo>::object_counter_);

    p3.Release();
    EXPECT_EQ(size_t(1), p1->ref_count_);
    EXPECT_EQ(NULL, p3.p_);
    EXPECT_EQ(size_t(2), RefCounted<Foo>::object_counter_);

    p2 = NULL;
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);

    p1 = p1;
    EXPECT_EQ(size_t(1), p1->ref_count_);
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);

    p1 = &(*p1);
    EXPECT_EQ(size_t(1), p1->ref_count_);
    EXPECT_EQ(size_t(1), RefCounted<Foo>::object_counter_);
  }
  EXPECT_EQ(size_t(0), RefCounted<Foo>::object_counter_);
  return true;
}

TEST(SmartPointer, All) {
  ASSERT_TRUE(TestSmartPointer());
}
