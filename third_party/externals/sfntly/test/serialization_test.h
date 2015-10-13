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

#ifndef SFNTLY_CPP_SRC_TEST_SERIALIZATION_TEST_H_
#define SFNTLY_CPP_SRC_TEST_SERIALIZATION_TEST_H_

#include "sfntly/table/table.h"

namespace sfntly {

bool VerifyHHEA(Table* original, Table* target);
bool VerifyGLYF(Table* original, Table* target);
bool VerifyHMTX(Table* original, Table* target);
bool VerifyLOCA(Table* original, Table* target);
bool VerifyMAXP(Table* original, Table* target);
bool VerifyNAME(Table* original, Table* target);
bool VerifyOS_2(Table* original, Table* target);

}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_SERIALIZATION_TEST_H_
