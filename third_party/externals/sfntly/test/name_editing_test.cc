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

// Must include this before ICU to avoid stdint redefinition issue.
#include "sfntly/port/type.h"

#include <unicode/ustring.h>
#include <unicode/unistr.h>

#include "gtest/gtest.h"
#include "sfntly/data/memory_byte_array.h"
#include "sfntly/font.h"
#include "sfntly/font_factory.h"
#include "sfntly/port/memory_input_stream.h"
#include "sfntly/port/memory_output_stream.h"
#include "sfntly/table/core/name_table.h"
#include "sfntly/tag.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace sfntly {

static ByteVector input_buffer;

void LoadTestFile(FontFactory* factory, FontBuilderArray* font_builders) {
  assert(factory);
  assert(font_builders);
  if (input_buffer.empty()) {
    LoadFile(SAMPLE_TTF_FILE, &input_buffer);
  }
  factory->LoadFontsForBuilding(&input_buffer, font_builders);
}

bool TestChangeOneName() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  LoadTestFile(factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];

  NameTableBuilderPtr name_builder = down_cast<NameTable::Builder*>(
      font_builder->GetTableBuilder(Tag::name));

  // Change the font name.
  NameEntryBuilderPtr neb =
      name_builder->NameBuilder(PlatformId::kWindows,
                                WindowsEncodingId::kUnicodeUCS2,
                                WindowsLanguageId::kEnglish_UnitedStates,
                                NameId::kFontFamilyName);
  U_STRING_DECL(new_name, "Timothy", 7);
  neb->SetName(new_name);

  // Build the font.
  FontPtr font;
  font.Attach(font_builder->Build());

  // Serialize and reload the serialized font.
  MemoryOutputStream os;
  factory->SerializeFont(font, &os);
  MemoryInputStream is;
  is.Attach(os.Get(), os.Size());
  FontArray font_array;
  factory->LoadFonts(&is, &font_array);
  FontPtr new_font = font_array[0];

  // Check the font name.
  NameTablePtr name_table = down_cast<NameTable*>(font->GetTable(Tag::name));
  UChar* name = name_table->Name(PlatformId::kWindows,
                                 WindowsEncodingId::kUnicodeUCS2,
                                 WindowsLanguageId::kEnglish_UnitedStates,
                                 NameId::kFontFamilyName);
  EXPECT_TRUE(name != NULL);
  EXPECT_EQ(u_strcmp(name, new_name), 0);
  delete[] name;
  return true;
}

bool TestModifyNameTableAndRevert() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  LoadTestFile(factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];

  NameTableBuilderPtr name_builder = down_cast<NameTable::Builder*>(
      font_builder->GetTableBuilder(Tag::name));

  // Change the font name.
  NameEntryBuilderPtr neb =
      name_builder->NameBuilder(PlatformId::kWindows,
                                WindowsEncodingId::kUnicodeUCS2,
                                WindowsLanguageId::kEnglish_UnitedStates,
                                NameId::kFontFamilyName);
  NameTable::NameEntry* neb_entry = neb->name_entry();
  UChar* original_name = neb_entry->Name();
  EXPECT_TRUE(original_name != NULL);

  U_STRING_DECL(new_name, "Timothy", 7);
  neb->SetName(new_name);
  name_builder->RevertNames();

  // Build the font.
  FontPtr font;
  font.Attach(font_builder->Build());

  // Serialize and reload the serialized font.
  MemoryOutputStream os;
  factory->SerializeFont(font, &os);
  MemoryInputStream is;
  is.Attach(os.Get(), os.Size());
  FontArray font_array;
  factory->LoadFonts(&is, &font_array);
  FontPtr new_font = font_array[0];

  // Check the font name.
  NameTablePtr name_table = down_cast<NameTable*>(font->GetTable(Tag::name));
  UChar* name = name_table->Name(PlatformId::kWindows,
                                 WindowsEncodingId::kUnicodeUCS2,
                                 WindowsLanguageId::kEnglish_UnitedStates,
                                 NameId::kFontFamilyName);

  EXPECT_EQ(u_strcmp(name, original_name), 0);
  delete[] name;
  delete[] original_name;

  return true;
}

bool TestRemoveOneName() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  LoadTestFile(factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];

  NameTableBuilderPtr name_builder = down_cast<NameTable::Builder*>(
      font_builder->GetTableBuilder(Tag::name));

  EXPECT_TRUE(name_builder->Has(PlatformId::kWindows,
                                WindowsEncodingId::kUnicodeUCS2,
                                WindowsLanguageId::kEnglish_UnitedStates,
                                NameId::kFontFamilyName));
  EXPECT_TRUE(name_builder->Remove(PlatformId::kWindows,
                                   WindowsEncodingId::kUnicodeUCS2,
                                   WindowsLanguageId::kEnglish_UnitedStates,
                                   NameId::kFontFamilyName));

  // Build the font.
  FontPtr font;
  font.Attach(font_builder->Build());

  // Serialize and reload the serialized font.
  MemoryOutputStream os;
  factory->SerializeFont(font, &os);
  MemoryInputStream is;
  is.Attach(os.Get(), os.Size());
  FontArray font_array;
  factory->LoadFonts(&is, &font_array);
  FontPtr new_font = font_array[0];

  // Check the font name.
  NameTablePtr name_table = down_cast<NameTable*>(font->GetTable(Tag::name));
  UChar* name = name_table->Name(PlatformId::kWindows,
                                 WindowsEncodingId::kUnicodeUCS2,
                                 WindowsLanguageId::kEnglish_UnitedStates,
                                 NameId::kFontFamilyName);
  EXPECT_TRUE(name == NULL);

  return true;
}

// Note: Function is not implemented but the test case is built.  Uncomment
//       when NameTable::clear() is implemented.
/*
bool TestClearAllNamesAndSetOne() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontBuilderArray font_builder_array;
  LoadTestFile(factory, &font_builder_array);
  FontBuilderPtr font_builder = font_builder_array[0];

  NameTableBuilderPtr name_builder = down_cast<NameTable::Builder*>(
      font_builder->GetTableBuilder(Tag::name));

  EXPECT_GT(name_builder->builderCount(), 0);
  name_builder->clear();
  EXPECT_EQ(name_builder->builderCount(), 0);

  // Change the font name.
  NameEntryBuilderPtr neb =
      name_builder->NameBuilder(PlatformId::kWindows,
                                WindowsEncodingId::kUnicodeUCS2,
                                WindowsLanguageId::kEnglish_UnitedStates,
                                NameId::kFontFamilyName);
  U_STRING_DECL(new_name, "Fred", 4);
  neb->SetName(new_name);

  // Build the font.
  FontPtr font = font_builder->Build();

  // Serialize and reload the serialized font.
  MemoryOutputStream os;
  factory->SerializeFont(font, &os);
  FontArray font_array;
  ByteArrayPtr new_ba = new MemoryByteArray(os.Get(), os.Size());
  factory->LoadFonts(new_ba, &font_array);
  FontPtr new_font = font_array[0];

  // Check the font name.
  NameTablePtr name_table = down_cast<NameTable*>(font->table(Tag::name));
  UChar* name = name_table->Name(PlatformId::kWindows,
                                 WindowsEncodingId::kUnicodeUCS2,
                                 WindowsLanguageId::kEnglish_UnitedStates,
                                 NameId::kFontFamilyName);
  EXPECT_EQ(name_table->NameCount(), 1);
  EXPECT_EQ(u_strcmp(name, new_name), 0);

  delete[] name;
  return true;
}
*/

}  // namespace sfntly

TEST(NameEditing, All) {
  EXPECT_TRUE(sfntly::TestChangeOneName());
  EXPECT_TRUE(sfntly::TestModifyNameTableAndRevert());
  EXPECT_TRUE(sfntly::TestRemoveOneName());
}
