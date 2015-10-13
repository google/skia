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
#include "sfntly/table/core/horizontal_device_metrics_table.h"
#include "test/test_data.h"
#include "test/test_font_utils.h"

namespace sfntly {

const int32_t HDMX_VERSION = 0;
const int32_t HDMX_NUM_RECORDS = 4;
const int32_t HDMX_RECORD_SIZE = 628;
const int32_t HDMX_PIXEL_SIZE[] = {10, 11, 12, 13};
const int32_t HDMX_MAX_WIDTH[] = {5, 6, 7, 7};

bool TestReadingHdmxTable() {
  FontFactoryPtr factory;
  factory.Attach(FontFactory::GetInstance());
  FontArray font_array;
  LoadFont(SAMPLE_BITMAP_FONT, factory, &font_array);
  FontPtr font = font_array[0];

  HorizontalDeviceMetricsTablePtr hdmx_table =
      down_cast<HorizontalDeviceMetricsTable*>(font->GetTable(Tag::hdmx));

  EXPECT_FALSE(hdmx_table == NULL);

  EXPECT_EQ(hdmx_table->Version(), HDMX_VERSION);
  EXPECT_EQ(hdmx_table->NumRecords(), HDMX_NUM_RECORDS);
  EXPECT_EQ(hdmx_table->RecordSize(), HDMX_RECORD_SIZE);

  for (int32_t i = 0; i < HDMX_NUM_RECORDS; ++i) {
    EXPECT_EQ(hdmx_table->PixelSize(i), HDMX_PIXEL_SIZE[i]);
    EXPECT_EQ(hdmx_table->MaxWidth(i), HDMX_MAX_WIDTH[i]);
  }

  EXPECT_EQ(hdmx_table->Width(0, 0), HDMX_MAX_WIDTH[0]);
  EXPECT_EQ(hdmx_table->Width(0, 19), HDMX_MAX_WIDTH[0]);
  EXPECT_EQ(hdmx_table->Width(0, 623), HDMX_MAX_WIDTH[0]);
  EXPECT_EQ(hdmx_table->Width(1, 0), HDMX_MAX_WIDTH[1]);
  EXPECT_EQ(hdmx_table->Width(1, 19), HDMX_MAX_WIDTH[1]);
  EXPECT_EQ(hdmx_table->Width(1, 623), HDMX_MAX_WIDTH[1]);
  EXPECT_EQ(hdmx_table->Width(2, 0), HDMX_MAX_WIDTH[2]);
  EXPECT_EQ(hdmx_table->Width(2, 19), HDMX_MAX_WIDTH[2]);
  EXPECT_EQ(hdmx_table->Width(2, 623), HDMX_MAX_WIDTH[2]);
  EXPECT_EQ(hdmx_table->Width(3, 0), HDMX_MAX_WIDTH[3]);
  EXPECT_EQ(hdmx_table->Width(3, 19), HDMX_MAX_WIDTH[3]);
  EXPECT_EQ(hdmx_table->Width(3, 623), HDMX_MAX_WIDTH[3]);

#if defined(SFNTLY_NO_EXCEPTION)
  EXPECT_EQ(hdmx_table->PixelSize(4), -1);
  EXPECT_EQ(hdmx_table->PixelSize(-1), -1);
  EXPECT_EQ(hdmx_table->MaxWidth(4), -1);
  EXPECT_EQ(hdmx_table->MaxWidth(-1), -1);
  EXPECT_EQ(hdmx_table->Width(0, 624), -1);
  EXPECT_EQ(hdmx_table->Width(1, -1), -1);
  EXPECT_EQ(hdmx_table->Width(-1, 0), -1);
  EXPECT_EQ(hdmx_table->Width(-1, -1), -1);
#endif
  return true;
}

}  // namespace sfntly

TEST(HdmxTable, All) {
  ASSERT_TRUE(sfntly::TestReadingHdmxTable());
}
