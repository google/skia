/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmXCodeObject.h"
#include "cmSystemTools.h"

#include <CoreFoundation/CoreFoundation.h> // CFUUIDCreate

//----------------------------------------------------------------------------
const char* cmXCodeObject::PBXTypeNames[] = {
    "PBXGroup", "PBXBuildStyle", "PBXProject", "PBXHeadersBuildPhase",
    "PBXSourcesBuildPhase", "PBXFrameworksBuildPhase", "PBXNativeTarget",
    "PBXFileReference", "PBXBuildFile", "PBXContainerItemProxy",
    "PBXTargetDependency", "PBXShellScriptBuildPhase",
    "PBXResourcesBuildPhase", "PBXApplicationReference",
    "PBXExecutableFileReference", "PBXLibraryReference", "PBXToolTarget",
    "PBXLibraryTarget", "PBXAggregateTarget", "XCBuildConfiguration",
    "XCConfigurationList",
    "PBXCopyFilesBuildPhase",
    "None"
  };

//----------------------------------------------------------------------------
cmXCodeObject::~cmXCodeObject()
{
  this->Version = 15;
}

//----------------------------------------------------------------------------
cmXCodeObject::cmXCodeObject(PBXType ptype, Type type)
{
  this->Version = 15;
  this->Target = 0;
  this->Object =0;

  this->IsA = ptype;

  if(type == OBJECT)
    {
    // Set the Id of an Xcode object to a unique string for each instance.
    // However the Xcode user file references certain Ids: for those cases,
    // override the generated Id using SetId().
    //
    char cUuid[40] = {0};
    CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
    CFStringRef s = CFUUIDCreateString(kCFAllocatorDefault, uuid);
    CFStringGetCString(s, cUuid, sizeof(cUuid), kCFStringEncodingUTF8);
    this->Id = cUuid;
    CFRelease(s);
    CFRelease(uuid);
    }
  else
    {
    this->Id =
      "Temporary cmake object, should not be referred to in Xcode file";
    }

  cmSystemTools::ReplaceString(this->Id, "-", "");
  if(this->Id.size() > 24)
    {
    this->Id = this->Id.substr(0, 24);
    }

  this->TypeValue = type;
  if(this->TypeValue == OBJECT)
    {
    this->AddAttribute("isa", 0);
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Indent(int level, std::ostream& out)
{
  while(level)
    {
    out << "\t";
    level--;
    }
}

//----------------------------------------------------------------------------
void cmXCodeObject::Print(std::ostream& out)
{
  std::string separator = "\n";
  int indentFactor = 1;
  cmXCodeObject::Indent(2*indentFactor, out);
  if(this->Version > 15
     && (this->IsA == PBXFileReference || this->IsA == PBXBuildFile))
    {
    separator = " ";
    indentFactor = 0;
    }
  out << this->Id;
  this->PrintComment(out);
  out << " = {";
  if(separator == "\n")
    {
    out << separator;
    }
  std::map<std::string, cmXCodeObject*>::iterator i;
  cmXCodeObject::Indent(3*indentFactor, out);
  out << "isa = " << PBXTypeNames[this->IsA]  << ";" << separator;
  for(i = this->ObjectAttributes.begin();
      i != this->ObjectAttributes.end(); ++i)
    {
    cmXCodeObject* object = i->second;
    if(i->first != "isa")
      {
      cmXCodeObject::Indent(3*indentFactor, out);
      }
    else
      {
      continue;
      }
    if(object->TypeValue == OBJECT_LIST)
      {
      out << i->first << " = (" << separator;
      for(unsigned int k = 0; k < i->second->List.size(); k++)
        {
        cmXCodeObject::Indent(4*indentFactor, out);
        out << i->second->List[k]->Id;
        i->second->List[k]->PrintComment(out);
        out << "," << separator;
        }
      cmXCodeObject::Indent(3*indentFactor, out);
      out << ");" << separator;
      }
    else if(object->TypeValue == ATTRIBUTE_GROUP)
      {
      std::map<std::string, cmXCodeObject*>::iterator j;
      out << i->first << " = {";
      if(separator == "\n")
        {
        out << separator;
        }
      for(j = object->ObjectAttributes.begin(); j !=
            object->ObjectAttributes.end(); ++j)
        {
        cmXCodeObject::Indent(4 *indentFactor, out);

        if(j->second->TypeValue == STRING)
          {
          cmXCodeObject::PrintString(out,j->first);
          out << " = ";
          j->second->PrintString(out);
          out << ";";
          }
        else if(j->second->TypeValue == OBJECT_LIST)
          {
          cmXCodeObject::PrintString(out,j->first);
          out << " = (";
          for(unsigned int k = 0; k < j->second->List.size(); k++)
            {
            if(j->second->List[k]->TypeValue == STRING)
              {
              j->second->List[k]->PrintString(out);
              out << ", ";
              }
            else
              {
              out << "List_" << k << "_TypeValue_IS_NOT_STRING, ";
              }
            }
          out << ");";
          }
        else
          {
          cmXCodeObject::PrintString(out,j->first);
          out << " = error_unexpected_TypeValue_" <<
            j->second->TypeValue << ";";
          }

        out << separator;
        }
      cmXCodeObject::Indent(3 *indentFactor, out);
      out << "};" << separator;
      }
    else if(object->TypeValue == OBJECT_REF)
      {
      cmXCodeObject::PrintString(out,i->first);
      out << " = " << object->Object->Id;
      if(object->Object->HasComment() && i->first != "remoteGlobalIDString")
        {
        object->Object->PrintComment(out);
        }
      out << ";" << separator;
      }
    else if(object->TypeValue == STRING)
      {
      cmXCodeObject::PrintString(out,i->first);
      out << " = ";
      object->PrintString(out);
      out << ";" << separator;
      }
    else
      {
      out << "what is this?? " << i->first << "\n";
      }
    }
  cmXCodeObject::Indent(2*indentFactor, out);
  out << "};\n";
}

//----------------------------------------------------------------------------
void cmXCodeObject::PrintList(std::vector<cmXCodeObject*> const& objs,
                              std::ostream& out)
{
  cmXCodeObject::Indent(1, out);
  out << "objects = {\n";
  for(unsigned int i = 0; i < objs.size(); ++i)
    {
    if(objs[i]->TypeValue == OBJECT)
      {
      objs[i]->Print(out);
      }
    }
  cmXCodeObject::Indent(1, out);
  out << "};\n";
}

//----------------------------------------------------------------------------
void cmXCodeObject::CopyAttributes(cmXCodeObject* copy)
{
  this->ObjectAttributes = copy->ObjectAttributes;
  this->List = copy->List;
  this->String = copy->String;
  this->Object = copy->Object;
}

//----------------------------------------------------------------------------
void cmXCodeObject::PrintString(std::ostream& os,std::string String)
{
  // The string needs to be quoted if it contains any characters
  // considered special by the Xcode project file parser.
  bool needQuote =
    (String.empty() ||
     String.find("//") != String.npos ||
     String.find_first_of(" <>+-*=@[](){},~") != String.npos);
  const char* quote = needQuote? "\"" : "";

  // Print the string, quoted and escaped as necessary.
  os << quote;
  for(std::string::const_iterator i = String.begin();
      i != String.end(); ++i)
    {
    if(*i == '"')
      {
      // Escape double-quotes.
      os << '\\';
      }
    os << *i;
    }
  os << quote;
}

void cmXCodeObject::PrintString(std::ostream& os) const
{
  cmXCodeObject::PrintString(os,this->String);
}

//----------------------------------------------------------------------------
void cmXCodeObject::SetString(const std::string& s)
{
  this->String = s;
}
