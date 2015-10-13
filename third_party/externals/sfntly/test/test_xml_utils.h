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

#include "sfntly/port/refcount.h"
#include "test/tinyxml/tinyxml.h"

#ifndef SFNTLY_CPP_SRC_TEST_TEST_XML_UTILS_H_
#define SFNTLY_CPP_SRC_TEST_TEST_XML_UTILS_H_

namespace sfntly {
typedef std::map<std::string, std::string> AttributeMap;
typedef std::vector<const TiXmlNode*> TiXmlNodeVector;

TiXmlNodeVector* GetNodesWithName(const TiXmlNode* node,
                                  const std::string& name);
const TiXmlAttribute* GetAttribute(const TiXmlNode* node,
                                   const std::string& name);
}  // namespace sfntly

#endif  // SFNTLY_CPP_SRC_TEST_TEST_XML_UTILS_H_
