/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackDragNDropGenerator.h"
#include "cmCPackLog.h"
#include "cmSystemTools.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/FStream.hxx>

static const char* SLAHeader =
"data 'LPic' (5000) {\n"
"    $\"0002 0011 0003 0001 0000 0000 0002 0000\"\n"
"    $\"0008 0003 0000 0001 0004 0000 0004 0005\"\n"
"    $\"0000 000E 0006 0001 0005 0007 0000 0007\"\n"
"    $\"0008 0000 0047 0009 0000 0034 000A 0001\"\n"
"    $\"0035 000B 0001 0020 000C 0000 0011 000D\"\n"
"    $\"0000 005B 0004 0000 0033 000F 0001 000C\"\n"
"    $\"0010 0000 000B 000E 0000\"\n"
"};\n"
"\n";

static const char* SLASTREnglish =
"resource 'STR#' (5002, \"English\") {\n"
"    {\n"
"        \"English\",\n"
"        \"Agree\",\n"
"        \"Disagree\",\n"
"        \"Print\",\n"
"        \"Save...\",\n"
"        \"You agree to the License Agreement terms when you click \"\n"
"        \"the \\\"Agree\\\" button.\",\n"
"        \"Software License Agreement\",\n"
"        \"This text cannot be saved.  This disk may be full or locked, "
"or the \"\n"
"        \"file may be locked.\",\n"
"        \"Unable to print.  Make sure you have selected a printer.\"\n"
"    }\n"
"};\n"
"\n";

//----------------------------------------------------------------------
cmCPackDragNDropGenerator::cmCPackDragNDropGenerator()
{
  // default to one package file for components
  this->componentPackageMethod = ONE_PACKAGE;
}

//----------------------------------------------------------------------
cmCPackDragNDropGenerator::~cmCPackDragNDropGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackDragNDropGenerator::InitializeInternal()
{
  // Starting with Xcode 4.3, look in "/Applications/Xcode.app" first:
  //
  std::vector<std::string> paths;
  paths.push_back("/Applications/Xcode.app/Contents/Developer/Tools");
  paths.push_back("/Developer/Tools");

  const std::string hdiutil_path = cmSystemTools::FindProgram("hdiutil",
    std::vector<std::string>(), false);
  if(hdiutil_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot locate hdiutil command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_HDIUTIL", hdiutil_path.c_str());

  const std::string setfile_path = cmSystemTools::FindProgram("SetFile",
    paths, false);
  if(setfile_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot locate SetFile command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_SETFILE", setfile_path.c_str());

  const std::string rez_path = cmSystemTools::FindProgram("Rez",
    paths, false);
  if(rez_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot locate Rez command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_REZ", rez_path.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
const char* cmCPackDragNDropGenerator::GetOutputExtension()
{
  return ".dmg";
}

//----------------------------------------------------------------------
int cmCPackDragNDropGenerator::PackageFiles()
{
  // gather which directories to make dmg files for
  // multiple directories occur if packaging components or groups separately

  // monolith
  if(this->Components.empty())
    {
    return this->CreateDMG(toplevel, packageFileNames[0]);
    }

  // component install
  std::vector<std::string> package_files;

  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt=this->Components.begin();
       compIt!=this->Components.end(); ++compIt )
    {
    std::string name = GetComponentInstallDirNameSuffix(compIt->first);
    package_files.push_back(name);
    }
  std::sort(package_files.begin(), package_files.end());
  package_files.erase(std::unique(package_files.begin(),
                      package_files.end()),
                      package_files.end());


  // loop to create dmg files
  packageFileNames.clear();
  for(size_t i=0; i<package_files.size(); i++)
    {
    std::string full_package_name = std::string(toplevel) + std::string("/");
    if(package_files[i] == "ALL_IN_ONE")
      {
      full_package_name += this->GetOption("CPACK_PACKAGE_FILE_NAME");
      }
    else
      {
      full_package_name += package_files[i];
      }
    full_package_name += std::string(GetOutputExtension());
    packageFileNames.push_back(full_package_name);

    std::string src_dir = toplevel;
    src_dir += "/";
    src_dir += package_files[i];

    if(0 == this->CreateDMG(src_dir, full_package_name))
      {
      return 0;
      }
    }
  return 1;
}

//----------------------------------------------------------------------
bool cmCPackDragNDropGenerator::CopyFile(std::ostringstream& source,
  std::ostringstream& target)
{
  if(!cmSystemTools::CopyFileIfDifferent(
    source.str().c_str(),
    target.str().c_str()))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error copying "
      << source.str()
      << " to "
      << target.str()
      << std::endl);

    return false;
    }

  return true;
}

//----------------------------------------------------------------------
bool cmCPackDragNDropGenerator::RunCommand(std::ostringstream& command,
  std::string* output)
{
  int exit_code = 1;

  bool result = cmSystemTools::RunSingleCommand(
    command.str().c_str(),
    output, output,
    &exit_code,
    0,
    this->GeneratorVerbose,
    0);

  if(!result || exit_code)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error executing: "
      << command.str()
      << std::endl);

    return false;
    }

  return true;
}

//----------------------------------------------------------------------
int cmCPackDragNDropGenerator::CreateDMG(const std::string& src_dir,
                                         const std::string& output_file)
{
  // Get optional arguments ...
  const std::string cpack_package_icon = this->GetOption("CPACK_PACKAGE_ICON")
    ? this->GetOption("CPACK_PACKAGE_ICON") : "";

  const std::string cpack_dmg_volume_name =
    this->GetOption("CPACK_DMG_VOLUME_NAME")
    ? this->GetOption("CPACK_DMG_VOLUME_NAME")
        : this->GetOption("CPACK_PACKAGE_FILE_NAME");

  const std::string cpack_dmg_format =
    this->GetOption("CPACK_DMG_FORMAT")
    ? this->GetOption("CPACK_DMG_FORMAT") : "UDZO";

  // Get optional arguments ...
  std::string cpack_license_file =
    this->GetOption("CPACK_RESOURCE_FILE_LICENSE") ?
    this->GetOption("CPACK_RESOURCE_FILE_LICENSE") : "";

  const std::string cpack_dmg_background_image =
    this->GetOption("CPACK_DMG_BACKGROUND_IMAGE")
    ? this->GetOption("CPACK_DMG_BACKGROUND_IMAGE") : "";

  const std::string cpack_dmg_ds_store =
    this->GetOption("CPACK_DMG_DS_STORE")
    ? this->GetOption("CPACK_DMG_DS_STORE") : "";

  // only put license on dmg if is user provided
  if(!cpack_license_file.empty() &&
      cpack_license_file.find("CPack.GenericLicense.txt") != std::string::npos)
  {
    cpack_license_file = "";
  }

  // The staging directory contains everything that will end-up inside the
  // final disk image ...
  std::ostringstream staging;
  staging << src_dir;

  // Add a symlink to /Applications so users can drag-and-drop the bundle
  // into it
  std::ostringstream application_link;
  application_link << staging.str() << "/Applications";
  cmSystemTools::CreateSymlink("/Applications",
    application_link.str().c_str());

  // Optionally add a custom volume icon ...
  if(!cpack_package_icon.empty())
    {
    std::ostringstream package_icon_source;
    package_icon_source << cpack_package_icon;

    std::ostringstream package_icon_destination;
    package_icon_destination << staging.str() << "/.VolumeIcon.icns";

    if(!this->CopyFile(package_icon_source, package_icon_destination))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error copying disk volume icon.  "
                    "Check the value of CPACK_PACKAGE_ICON."
        << std::endl);

      return 0;
      }
    }

  // Optionally add a custom .DS_Store file
  // (e.g. for setting background/layout) ...
  if(!cpack_dmg_ds_store.empty())
    {
    std::ostringstream package_settings_source;
    package_settings_source << cpack_dmg_ds_store;

    std::ostringstream package_settings_destination;
    package_settings_destination << staging.str() << "/.DS_Store";

    if(!this->CopyFile(package_settings_source, package_settings_destination))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error copying disk volume settings file.  "
                    "Check the value of CPACK_DMG_DS_STORE."
        << std::endl);

      return 0;
      }
    }

  // Optionally add a custom background image ...
  if(!cpack_dmg_background_image.empty())
    {
    std::ostringstream package_background_source;
    package_background_source << cpack_dmg_background_image;

    std::ostringstream package_background_destination;
    package_background_destination << staging.str() << "/background.png";

    if(!this->CopyFile(package_background_source,
        package_background_destination))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error copying disk volume background image.  "
                    "Check the value of CPACK_DMG_BACKGROUND_IMAGE."
        << std::endl);

      return 0;
      }

    std::ostringstream temp_background_hiding_command;
    temp_background_hiding_command << this->GetOption("CPACK_COMMAND_SETFILE");
    temp_background_hiding_command << " -a V \"";
    temp_background_hiding_command << package_background_destination.str();
    temp_background_hiding_command << "\"";

    if(!this->RunCommand(temp_background_hiding_command))
      {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Error setting attributes on disk volume background image."
          << std::endl);

      return 0;
      }
    }

  // Create a temporary read-write disk image ...
  std::string temp_image = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  temp_image += "/temp.dmg";

  std::ostringstream temp_image_command;
  temp_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  temp_image_command << " create";
  temp_image_command << " -ov";
  temp_image_command << " -srcfolder \"" << staging.str() << "\"";
  temp_image_command << " -volname \""
    << cpack_dmg_volume_name << "\"";
  temp_image_command << " -format UDRW";
  temp_image_command << " \"" << temp_image << "\"";

  if(!this->RunCommand(temp_image_command))
    {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error generating temporary disk image."
        << std::endl);

    return 0;
    }

  // Optionally set the custom icon flag for the image ...
  if(!cpack_package_icon.empty())
    {
    std::ostringstream temp_mount;

    std::ostringstream attach_command;
    attach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    attach_command << " attach";
    attach_command << " \"" << temp_image << "\"";

    std::string attach_output;
    if(!this->RunCommand(attach_command, &attach_output))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error attaching temporary disk image."
        << std::endl);

      return 0;
      }

    cmsys::RegularExpression mountpoint_regex(".*(/Volumes/[^\n]+)\n.*");
    mountpoint_regex.find(attach_output.c_str());
    temp_mount << mountpoint_regex.match(1);

    std::ostringstream setfile_command;
    setfile_command << this->GetOption("CPACK_COMMAND_SETFILE");
    setfile_command << " -a C";
    setfile_command << " \"" << temp_mount.str() << "\"";

    if(!this->RunCommand(setfile_command))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error assigning custom icon to temporary disk image."
        << std::endl);

      return 0;
      }

    std::ostringstream detach_command;
    detach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    detach_command << " detach";
    detach_command << " \"" << temp_mount.str() << "\"";

    if(!this->RunCommand(detach_command))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error detaching temporary disk image."
        << std::endl);

      return 0;
      }
    }

  if(!cpack_license_file.empty())
  {
    std::string sla_r = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    sla_r += "/sla.r";

    cmsys::ifstream ifs;
    ifs.open(cpack_license_file.c_str());
    if(ifs.is_open())
    {
      cmGeneratedFileStream osf(sla_r.c_str());
      osf << "#include <CoreServices/CoreServices.r>\n\n";
      osf << SLAHeader;
      osf << "\n";
      osf << "data 'TEXT' (5002, \"English\") {\n";
      while(ifs.good())
      {
        std::string line;
        std::getline(ifs, line);
        // escape quotes
        std::string::size_type pos = line.find('\"');
        while(pos != std::string::npos)
        {
          line.replace(pos, 1, "\\\"");
          pos = line.find('\"', pos+2);
        }
        // break up long lines to avoid Rez errors
        std::vector<std::string> lines;
        const size_t max_line_length = 512;
        for(size_t i=0; i<line.size(); i+= max_line_length)
          {
          int line_length = max_line_length;
          if(i+max_line_length > line.size())
            line_length = line.size()-i;
          lines.push_back(line.substr(i, line_length));
          }

        for(size_t i=0; i<lines.size(); i++)
          {
          osf << "        \"" << lines[i] << "\"\n";
          }
        osf << "        \"\\n\"\n";
      }
      osf << "};\n";
      osf << "\n";
      osf << SLASTREnglish;
      ifs.close();
      osf.close();
    }

    // convert to UDCO
    std::string temp_udco = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    temp_udco += "/temp-udco.dmg";

    std::ostringstream udco_image_command;
    udco_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    udco_image_command << " convert \"" << temp_image << "\"";
    udco_image_command << " -format UDCO";
    udco_image_command << " -ov -o \"" << temp_udco << "\"";

    std::string error;
    if(!this->RunCommand(udco_image_command, &error))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error converting to UDCO dmg for adding SLA." << std::endl
        << error
        << std::endl);
      return 0;
      }

    // unflatten dmg
    std::ostringstream unflatten_command;
    unflatten_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    unflatten_command << " unflatten ";
    unflatten_command << "\"" << temp_udco << "\"";

    if(!this->RunCommand(unflatten_command, &error))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error unflattening dmg for adding SLA." << std::endl
        << error
        << std::endl);
      return 0;
      }

    // Rez the SLA
    std::ostringstream embed_sla_command;
    embed_sla_command << this->GetOption("CPACK_COMMAND_REZ");
    const char* sysroot = this->GetOption("CPACK_OSX_SYSROOT");
    if(sysroot && sysroot[0] != '\0')
      {
      embed_sla_command << " -isysroot \"" << sysroot << "\"";
      }
    embed_sla_command << " \"" << sla_r << "\"";
    embed_sla_command << " -a -o ";
    embed_sla_command << "\"" << temp_udco << "\"";

    if(!this->RunCommand(embed_sla_command, &error))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error adding SLA." << std::endl
        << error
        << std::endl);
      return 0;
      }

    // flatten dmg
    std::ostringstream flatten_command;
    flatten_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    flatten_command << " flatten ";
    flatten_command << "\"" << temp_udco << "\"";

    if(!this->RunCommand(flatten_command, &error))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error flattening dmg for adding SLA." << std::endl
        << error
        << std::endl);
      return 0;
      }

    temp_image = temp_udco;
  }


  // Create the final compressed read-only disk image ...
  std::ostringstream final_image_command;
  final_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  final_image_command << " convert \"" << temp_image << "\"";
  final_image_command << " -format ";
  final_image_command << cpack_dmg_format;
  final_image_command << " -imagekey";
  final_image_command << " zlib-level=9";
  final_image_command << " -o \"" << output_file << "\"";

  if(!this->RunCommand(final_image_command))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error compressing disk image."
      << std::endl);

    return 0;
    }

  return 1;
}

bool cmCPackDragNDropGenerator::SupportsComponentInstallation() const
{
  return true;
}

std::string
cmCPackDragNDropGenerator::GetComponentInstallDirNameSuffix(
                           const std::string& componentName)
{
  // we want to group components together that go in the same dmg package
  std::string package_file_name = this->GetOption("CPACK_PACKAGE_FILE_NAME");

  // we have 3 mutually exclusive modes to work in
  // 1. all components in one package
  // 2. each group goes in its own package with left over
  //    components in their own package
  // 3. ignore groups - if grouping is defined, it is ignored
  //    and each component goes in its own package

  if(this->componentPackageMethod == ONE_PACKAGE)
    {
    return "ALL_IN_ONE";
    }

  if(this->componentPackageMethod == ONE_PACKAGE_PER_GROUP)
    {
    // We have to find the name of the COMPONENT GROUP
    // the current COMPONENT belongs to.
    std::string groupVar = "CPACK_COMPONENT_" +
                         cmSystemTools::UpperCase(componentName) + "_GROUP";
    const char* _groupName = GetOption(groupVar.c_str());
    if (_groupName)
      {
      std::string groupName = _groupName;

      groupName = GetComponentPackageFileName(package_file_name,
                                              groupName, true);
      return groupName;
      }
    }

  return GetComponentPackageFileName(package_file_name, componentName, false);
}
