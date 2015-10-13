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
// File is originally from Chromium third_party/sfntly/src/subsetter.
// Use as test case in sfntly so that problems can be caught in upstream early.

#ifndef SFNTLY_CPP_SRC_TEST_SUBSETTER_IMPL_H_
#define SFNTLY_CPP_SRC_TEST_SUBSETTER_IMPL_H_

#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/truetype/loca_table.h"
#include "sfntly/tag.h"

namespace sfntly {

// Smart pointer usage in sfntly:
//
// sfntly carries a smart pointer implementation like COM.  Ref-countable object
// type inherits from RefCounted<>, which have AddRef and Release just like
// IUnknown (but no QueryInterface).  Use a Ptr<> based smart pointer to hold
// the object so that the object ref count is handled correctly.
//
// class Foo : public RefCounted<Foo> {
//  public:
//   static Foo* CreateInstance() {
//     Ptr<Foo> obj = new Foo();  // ref count = 1
//     return obj.detach();
//   }
// };
// typedef Ptr<Foo> FooPtr;  // common short-hand notation
// FooPtr obj;
// obj.attach(Foo::CreatedInstance());  // ref count = 1
// {
//   FooPtr obj2 = obj;  // ref count = 2
// }  // ref count = 1, obj2 out of scope
// obj.release();  // ref count = 0, object destroyed

class SubsetterImpl {
 public:
  SubsetterImpl();
  ~SubsetterImpl();

  bool LoadFont(const char* font_name,
                const unsigned char* original_font,
                size_t font_size);
  int SubsetFont(const unsigned int* glyph_ids,
                 size_t glyph_count,
                 unsigned char** output_buffer);

 private:
  CALLER_ATTACH Font* Subset(const IntegerSet& glyph_ids,
                             GlyphTable* glyf, LocaTable* loca);

  FontFactoryPtr factory_;
  FontPtr font_;
};

}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_SUBSETTER_IMPL_H_
