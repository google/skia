/* libs/graphics/ports/SkXMLParser_tinyxml.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkXMLParser.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "tinyxml.h"

static void walk_elem(SkXMLParser* parser, const TiXmlElement* elem)
{
    //printf("walk_elem(%s) ", elem->Value());

    parser->startElement(elem->Value());

    const TiXmlAttribute* attr = elem->FirstAttribute();
    while (attr)
    {
        //printf("walk_elem_attr(%s=\"%s\") ", attr->Name(), attr->Value());
    
        parser->addAttribute(attr->Name(), attr->Value());
        attr = attr->Next();
    }
    //printf("\n");
    
    const TiXmlNode* node = elem->FirstChild();
    while (node)
    {
        if (node->ToElement())
            walk_elem(parser, node->ToElement());
        else if (node->ToText())
            parser->text(node->Value(), strlen(node->Value()));
        node = node->NextSibling();
    }
    
    parser->endElement(elem->Value());
}

static bool load_buf(SkXMLParser* parser, const char buf[])
{
    TiXmlDocument                   doc;

    (void)doc.Parse(buf);
    if (doc.Error())
    {
        printf("tinyxml error: <%s> row[%d] col[%d]\n", doc.ErrorDesc(), doc.ErrorRow(), doc.ErrorCol());
        return false;
    }
    
    walk_elem(parser, doc.RootElement());
    return true;
}

bool SkXMLParser::parse(SkStream& stream)
{
    size_t size = stream.read(NULL, 0);
    
    SkAutoMalloc    buffer(size + 1);
    char*           buf = (char*)buffer.get();
    
    stream.read(buf, size);
    buf[size] = 0;
    
    return load_buf(this, buf);
}

bool SkXMLParser::parse(const char doc[], size_t len)
{
    SkAutoMalloc    buffer(len + 1);
    char*           buf = (char*)buffer.get();
    
    memcpy(buf, doc, len);
    buf[len] = 0;
    
    return load_buf(this, buf);
}

void SkXMLParser::GetNativeErrorString(int error, SkString* str)
{
    if (str)
        str->set("GetNativeErrorString not implemented for TinyXml");
}

