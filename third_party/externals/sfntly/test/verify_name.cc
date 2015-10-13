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
#include "sfntly/font.h"
#include "sfntly/table/core/name_table.h"
#include "test/serialization_test.h"

namespace sfntly {

const int32_t NAME_FORMAT = 0;
const int32_t NAME_COUNT = 75;
const NameTable::NameEntryId NAME_IDS[] = {
    NameTable::NameEntryId(1, 0, 0, 0),  // 0
    NameTable::NameEntryId(1, 0, 0, 1),  // 1
    NameTable::NameEntryId(1, 0, 0, 2),  // 2
    NameTable::NameEntryId(1, 0, 0, 3),  // 3
    NameTable::NameEntryId(1, 0, 0, 4),  // 4
    NameTable::NameEntryId(1, 0, 0, 5),  // 5
    NameTable::NameEntryId(1, 0, 0, 6),  // 6
    NameTable::NameEntryId(1, 0, 0, 9),  // 7
    NameTable::NameEntryId(1, 0, 0, 11),  // 8
    NameTable::NameEntryId(1, 0, 0, 12),  // 9
};
const int32_t NAME_IDS_TEST = 10;

static bool VerifyNAME(Table* table) {
  // TODO(arthurhsu): Better testing can be done here.  Right now we just
  //                  iterate through the entries and get entry ids.
  NameTablePtr name = down_cast<NameTable*>(table);
  if (name == NULL) {
    return false;
  }

  EXPECT_EQ(name->Format(), NAME_FORMAT);
  EXPECT_EQ(name->NameCount(), NAME_COUNT);
  fprintf(stderr, "checking name entry: ");
  for (int32_t i = 0; i < NAME_IDS_TEST; ++i) {
    fprintf(stderr, "%d ", i);
    EXPECT_EQ(name->PlatformId(i), NAME_IDS[i].platform_id());
    EXPECT_EQ(name->EncodingId(i), NAME_IDS[i].encoding_id());
    EXPECT_EQ(name->LanguageId(i), NAME_IDS[i].language_id());
    EXPECT_EQ(name->NameId(i), NAME_IDS[i].name_id());
  }
  fprintf(stderr, "\n");
  return true;
}

bool VerifyNAME(Table* original, Table* target) {
  EXPECT_TRUE(VerifyNAME(original));
  EXPECT_TRUE(VerifyNAME(target));
  return true;
}

}  // namespace sfntly
