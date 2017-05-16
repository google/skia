/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmXCode21Object.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmXCode21Object::cmXCode21Object(PBXType ptype, Type type)
  :cmXCodeObject(ptype, type)
{
  this->Version = 21;
}

//----------------------------------------------------------------------------
void cmXCode21Object::PrintComment(std::ostream& out)
{
  if(this->Comment.size() == 0)
    {
    cmXCodeObject* n = this->GetObject("name");
    if(n)
      {
      this->Comment = n->GetString();
      cmSystemTools::ReplaceString(this->Comment, "\"", "");
      }
    }
  if(this->Comment.empty())
    {
    return;
    }
  out << " /* ";
  out << this->Comment;
  out << " */";
}

//----------------------------------------------------------------------------
void cmXCode21Object::PrintList(std::vector<cmXCodeObject*> const& v,
                                std::ostream& out, PBXType t)
{
  bool hasOne = false;
  for(std::vector<cmXCodeObject*>::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    cmXCodeObject* obj = *i;
    if(obj->GetType() == OBJECT && obj->GetIsA() == t)
      {
      hasOne = true;
      break;
      }
    }
  if(!hasOne)
    {
    return;
    }
  out << "\n/* Begin " <<  PBXTypeNames[t] << " section */\n";
  for(std::vector<cmXCodeObject*>::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    cmXCodeObject* obj = *i;
    if(obj->GetType() == OBJECT && obj->GetIsA() == t)
      {
        obj->Print(out);
      }
    }
  out << "/* End " <<  PBXTypeNames[t] << " section */\n";
}

//----------------------------------------------------------------------------
void cmXCode21Object::PrintList(std::vector<cmXCodeObject*> const& v,
                                std::ostream& out)
{
  cmXCodeObject::Indent(1, out);
  out << "objects = {\n";
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXAggregateTarget);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXBuildFile);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXBuildStyle);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXContainerItemProxy);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXFileReference);
  cmXCode21Object::PrintList(v, out,
                             cmXCode21Object::PBXFrameworksBuildPhase);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXGroup);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXHeadersBuildPhase);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXNativeTarget);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXProject);
  cmXCode21Object::PrintList(v, out,
                             cmXCode21Object::PBXShellScriptBuildPhase);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXResourcesBuildPhase);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXSourcesBuildPhase);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXCopyFilesBuildPhase);
  cmXCode21Object::PrintList(v, out,
                             cmXCode21Object::PBXApplicationReference);
  cmXCode21Object::PrintList(v, out,
                             cmXCode21Object::PBXExecutableFileReference);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXLibraryReference);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXToolTarget);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXLibraryTarget);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::PBXTargetDependency);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::XCBuildConfiguration);
  cmXCode21Object::PrintList(v, out, cmXCode21Object::XCConfigurationList);
  cmXCodeObject::Indent(1, out);
  out << "};\n";
}
