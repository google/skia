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
#if _MSC_VER > 12
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#endif

#include "sample/subsetter/subset_util.h"

int main(int argc, char** argv) {
#ifdef _CRTDBG_MAP_ALLOC
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  if (argc < 3) {
    printf("Usage: subsetter <font file> <output file>\n");
    return 0;
  }

  sfntly::SubsetUtil subset_util;
  subset_util.Subset(argv[1], argv[2]);

#ifdef _CRTDBG_MAP_ALLOC
  _CrtDumpMemoryLeaks();
#endif

  return 0;
}
