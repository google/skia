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

#include <stdio.h>

#include "sfntly/font.h"
#include "sfntly/table/table.h"
#include "sfntly/tag.h"
#include "subtly/stats.h"

namespace subtly {
using namespace sfntly;

int32_t TotalFontSize(Font* font) {
  int32_t size = 0;
  const TableMap* table_map = font->GetTableMap();
  for (TableMap::const_iterator it = table_map->begin(),
           e = table_map->end(); it != e; ++it) {
    size += it->second->DataLength();
  }
  return size;
}

double TableSizePercent(Font* font, int32_t tag) {
  TablePtr table = font->GetTable(tag);
  return static_cast<double>(table->DataLength()) / TotalFontSize(font) * 100;
}

void PrintComparison(FILE* out, Font* font, Font* new_font) {
  fprintf(out, "====== Table Comparison (original v. subset) ======\n");
  const TableMap* tables = font->GetTableMap();
  for (TableMap::const_iterator it = tables->begin(),
           e = tables->end(); it != e; ++it) {
    char *name = TagToString(it->first);
    int32_t size = it->second->DataLength();
    fprintf(out, "-- %s: %d (%lf%%) ", name, size,
            TableSizePercent(font, it->first));
    delete[] name;

    Ptr<FontDataTable> new_table = new_font->GetTable(it->first);
    int32_t new_size = 0;
    double size_percent = 0;
    if (new_table) {
      new_size = new_table->DataLength();
      size_percent = subtly::TableSizePercent(new_font, it->first);
    }

    if (new_size == size) {
      fprintf(out, "| same size\n");
    } else {
      fprintf(out, "-> %d (%lf%%) | %lf%% of original\n", new_size,
              size_percent, static_cast<double>(new_size) / size * 100);
    }
  }
}

void PrintStats(FILE* out, Font* font) {
  fprintf(out, "====== Table Stats ======\n");
  const TableMap* tables = font->GetTableMap();
  for (TableMap::const_iterator it = tables->begin(),
           e = tables->end(); it != e; ++it) {
    char *name = TagToString(it->first);
    int32_t size = it->second->DataLength();
    fprintf(out, "-- %s: %d (%lf%%)\n", name, size,
            TableSizePercent(font, it->first));
    delete[] name;
  }
}
}
