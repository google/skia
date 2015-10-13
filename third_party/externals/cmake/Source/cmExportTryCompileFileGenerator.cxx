/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExportTryCompileFileGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpressionDAGChecker.h"

//----------------------------------------------------------------------------
bool cmExportTryCompileFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::set<cmTarget const*> emitted;
  std::set<cmTarget const*> emittedDeps;
  while(!this->Exports.empty())
    {
    cmTarget const* te = this->Exports.back();
    this->Exports.pop_back();
    if (emitted.insert(te).second)
      {
      emittedDeps.insert(te);
      this->GenerateImportTargetCode(os, te);

      ImportPropertyMap properties;

#define FIND_TARGETS(PROPERTY) \
      this->FindTargets("INTERFACE_" #PROPERTY, te, emittedDeps);

      CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(FIND_TARGETS)

#undef FIND_TARGETS

      this->PopulateProperties(te, properties, emittedDeps);

      this->GenerateInterfaceProperties(te, os, properties);
      }
    }
  return true;
}

std::string cmExportTryCompileFileGenerator::FindTargets(
                                          const std::string& propName,
                                          cmTarget const* tgt,
                                          std::set<cmTarget const*> &emitted)
{
  const char *prop = tgt->GetProperty(propName);
  if(!prop)
    {
    return std::string();
    }

  cmGeneratorExpression ge;

  cmGeneratorExpressionDAGChecker dagChecker(
                                      tgt->GetName(),
                                      propName, 0, 0);

  cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

  cmTarget dummyHead;
  dummyHead.SetType(cmTarget::EXECUTABLE, "try_compile_dummy_exe");
  dummyHead.SetMakefile(tgt->GetMakefile());

  std::string result = cge->Evaluate(tgt->GetMakefile(), this->Config,
                                     false, &dummyHead, tgt, &dagChecker);

  const std::set<cmTarget const*> &allTargets = cge->GetAllTargetsSeen();
  for(std::set<cmTarget const*>::const_iterator li = allTargets.begin();
      li != allTargets.end(); ++li)
    {
    if(emitted.insert(*li).second)
      {
      this->Exports.push_back(*li);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
void
cmExportTryCompileFileGenerator::PopulateProperties(cmTarget const* target,
                                                ImportPropertyMap& properties,
                                          std::set<cmTarget const*> &emitted)
{
  cmPropertyMap props = target->GetProperties();
  for(cmPropertyMap::const_iterator i = props.begin(); i != props.end(); ++i)
    {
    properties[i->first] = i->second.GetValue();

    if(i->first.find("IMPORTED_LINK_INTERFACE_LIBRARIES") == 0
        || i->first.find("IMPORTED_LINK_DEPENDENT_LIBRARIES") == 0
        || i->first.find("INTERFACE_LINK_LIBRARIES") == 0)
      {
      const std::string libs = i->second.GetValue();

      std::string evalResult = this->FindTargets(i->first,
                                                 target, emitted);

      std::vector<std::string> depends;
      cmSystemTools::ExpandListArgument(evalResult, depends);
      for(std::vector<std::string>::const_iterator li = depends.begin();
          li != depends.end(); ++li)
        {
        cmTarget *tgt = target->GetMakefile()->FindTargetToUse(*li);
        if(tgt && emitted.insert(tgt).second)
          {
          this->Exports.push_back(tgt);
          }
        }
      }
    }
}
std::string
cmExportTryCompileFileGenerator::InstallNameDir(cmTarget* target,
                                                const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->GetMakefile();
  if(mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    install_name_dir =
      target->GetInstallNameDirForBuildTree(config);
    }

  return install_name_dir;
}
