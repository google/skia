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

#include <map>
#include <string>
#include "test/test_xml_utils.h"
#include "test/tinyxml/tinyxml.h"

namespace sfntly {
void InternalGetNodesWithName(const TiXmlNode* node, const std::string& name,
                              TiXmlNodeVector* wanted_nodes) {
  if (node->ValueStr() == name)
    wanted_nodes->push_back(node);
  for (const TiXmlNode* child = node->FirstChild();
       child != NULL; child = child->NextSibling()) {
    InternalGetNodesWithName(child, name, wanted_nodes);
  }
}

TiXmlNodeVector* GetNodesWithName(const TiXmlNode* node,
                                  const std::string& name) {
  TiXmlNodeVector* wanted_nodes = new TiXmlNodeVector;
  InternalGetNodesWithName(node, name, wanted_nodes);
  return wanted_nodes;
}

const TiXmlAttribute* GetAttribute(const TiXmlNode* node,
                                   const std::string& name) {
  for (const TiXmlAttribute* attribute = node->ToElement()->FirstAttribute();
       attribute != NULL; attribute = attribute->Next()) {
    if (attribute->Name() == name) {
      return attribute;
    }
  }
  return NULL;
}
}
