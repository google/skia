#.rst:
# CPackWIX
# --------
#
# CPack WiX generator specific options
#
# Variables specific to CPack WiX generator
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#
# The following variables are specific to the installers built on
# Windows using WiX.
#
# .. variable:: CPACK_WIX_UPGRADE_GUID
#
#  Upgrade GUID (``Product/@UpgradeCode``)
#
#  Will be automatically generated unless explicitly provided.
#
#  It should be explicitly set to a constant generated gloabally unique
#  identifier (GUID) to allow your installers to replace existing
#  installations that use the same GUID.
#
#  You may for example explicitly set this variable in your
#  CMakeLists.txt to the value that has been generated per default.  You
#  should not use GUIDs that you did not generate yourself or which may
#  belong to other projects.
#
#  A GUID shall have the following fixed length syntax::
#
#   XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
#
#  (each X represents an uppercase hexadecimal digit)
#
# .. variable:: CPACK_WIX_PRODUCT_GUID
#
#  Product GUID (``Product/@Id``)
#
#  Will be automatically generated unless explicitly provided.
#
#  If explicitly provided this will set the Product Id of your installer.
#
#  The installer will abort if it detects a pre-existing installation that
#  uses the same GUID.
#
#  The GUID shall use the syntax described for CPACK_WIX_UPGRADE_GUID.
#
# .. variable:: CPACK_WIX_LICENSE_RTF
#
#  RTF License File
#
#  If CPACK_RESOURCE_FILE_LICENSE has an .rtf extension it is used as-is.
#
#  If CPACK_RESOURCE_FILE_LICENSE has an .txt extension it is implicitly
#  converted to RTF by the WiX Generator.
#  The expected encoding of the .txt file is UTF-8.
#
#  With CPACK_WIX_LICENSE_RTF you can override the license file used by the
#  WiX Generator in case CPACK_RESOURCE_FILE_LICENSE is in an unsupported
#  format or the .txt -> .rtf conversion does not work as expected.
#
# .. variable:: CPACK_WIX_PRODUCT_ICON
#
#  The Icon shown next to the program name in Add/Remove programs.
#
#  If set, this icon is used in place of the default icon.
#
# .. variable:: CPACK_WIX_UI_REF
#
#  This variable allows you to override the Id of the ``<UIRef>`` element
#  in the WiX template.
#
#  The default is ``WixUI_InstallDir`` in case no CPack components have
#  been defined and ``WixUI_FeatureTree`` otherwise.
#
# .. variable:: CPACK_WIX_UI_BANNER
#
#  The bitmap will appear at the top of all installer pages other than the
#  welcome and completion dialogs.
#
#  If set, this image will replace the default banner image.
#
#  This image must be 493 by 58 pixels.
#
# .. variable:: CPACK_WIX_UI_DIALOG
#
#  Background bitmap used on the welcome and completion dialogs.
#
#  If this variable is set, the installer will replace the default dialog
#  image.
#
#  This image must be 493 by 312 pixels.
#
# .. variable:: CPACK_WIX_PROGRAM_MENU_FOLDER
#
#  Start menu folder name for launcher.
#
#  If this variable is not set, it will be initialized with CPACK_PACKAGE_NAME
#
# .. variable:: CPACK_WIX_CULTURES
#
#  Language(s) of the installer
#
#  Languages are compiled into the WixUI extension library.  To use them,
#  simply provide the name of the culture.  If you specify more than one
#  culture identifier in a comma or semicolon delimited list, the first one
#  that is found will be used.  You can find a list of supported languages at:
#  http://wix.sourceforge.net/manual-wix3/WixUI_localization.htm
#
# .. variable:: CPACK_WIX_TEMPLATE
#
#  Template file for WiX generation
#
#  If this variable is set, the specified template will be used to generate
#  the WiX wxs file.  This should be used if further customization of the
#  output is required.
#
#  If this variable is not set, the default MSI template included with CMake
#  will be used.
#
# .. variable:: CPACK_WIX_PATCH_FILE
#
#  Optional XML file with fragments to be inserted into generated WiX sources
#
#  This optional variable can be used to specify an XML file that the
#  WiX generator will use to inject fragments into its generated
#  source files.
#
#  Patch files understood by the CPack WiX generator
#  roughly follow this RELAX NG compact schema:
#
#  .. code-block:: none
#
#     start = CPackWiXPatch
#
#     CPackWiXPatch = element CPackWiXPatch { CPackWiXFragment* }
#
#     CPackWiXFragment = element CPackWiXFragment
#     {
#         attribute Id { string },
#         fragmentContent*
#     }
#
#     fragmentContent = element * - CPackWiXFragment
#     {
#         (attribute * { text } | text | fragmentContent)*
#     }
#
#  Currently fragments can be injected into most
#  Component, File and Directory elements.
#
#  The following additional special Ids can be used:
#
#  * ``#PRODUCT`` for the ``<Product>`` element.
#  * ``#PRODUCTFEATURE`` for the root ``<Feature>`` element.
#
#  The following example illustrates how this works.
#
#  Given that the WiX generator creates the following XML element:
#
#  .. code-block:: xml
#
#     <Component Id="CM_CP_applications.bin.my_libapp.exe" Guid="*"/>
#
#  The following XML patch file may be used to inject an Environment element
#  into it:
#
#  .. code-block:: xml
#
#     <CPackWiXPatch>
#       <CPackWiXFragment Id="CM_CP_applications.bin.my_libapp.exe">
#         <Environment Id="MyEnvironment" Action="set"
#           Name="MyVariableName" Value="MyVariableValue"/>
#       </CPackWiXFragment>
#     </CPackWiXPatch>
#
# .. variable:: CPACK_WIX_EXTRA_SOURCES
#
#  Extra WiX source files
#
#  This variable provides an optional list of extra WiX source files (.wxs)
#  that should be compiled and linked.  The full path to source files is
#  required.
#
# .. variable:: CPACK_WIX_EXTRA_OBJECTS
#
#  Extra WiX object files or libraries
#
#  This variable provides an optional list of extra WiX object (.wixobj)
#  and/or WiX library (.wixlib) files.  The full path to objects and libraries
#  is required.
#
# .. variable:: CPACK_WIX_EXTENSIONS
#
#  This variable provides a list of additional extensions for the WiX
#  tools light and candle.
#
# .. variable:: CPACK_WIX_<TOOL>_EXTENSIONS
#
#  This is the tool specific version of CPACK_WIX_EXTENSIONS.
#  ``<TOOL>`` can be either LIGHT or CANDLE.
#
# .. variable:: CPACK_WIX_<TOOL>_EXTRA_FLAGS
#
#  This list variable allows you to pass additional
#  flags to the WiX tool ``<TOOL>``.
#
#  Use it at your own risk.
#  Future versions of CPack may generate flags which may be in conflict
#  with your own flags.
#
#  ``<TOOL>`` can be either LIGHT or CANDLE.
#
# .. variable:: CPACK_WIX_CMAKE_PACKAGE_REGISTRY
#
#  If this variable is set the generated installer will create
#  an entry in the windows registry key
#  ``HKEY_LOCAL_MACHINE\Software\Kitware\CMake\Packages\<package>``
#  The value for ``<package>`` is provided by this variable.
#
#  Assuming you also install a CMake configuration file this will
#  allow other CMake projects to find your package with
#  the :command:`find_package` command.
#
# .. variable:: CPACK_WIX_PROPERTY_<PROPERTY>
#
#  This variable can be used to provide a value for
#  the Windows Installer property ``<PROPERTY>``
#
#  The follwing list contains some example properties that can be used to
#  customize information under
#  "Programs and Features" (also known as "Add or Remove Programs")
#
#  * ARPCOMMENTS - Comments
#  * ARPHELPLINK - Help and support information URL
#  * ARPURLINFOABOUT - General information URL
#  * URLUPDATEINFO - Update information URL
#  * ARPHELPTELEPHONE - Help and support telephone number
#  * ARPSIZE - Size (in kilobytes) of the application

#=============================================================================
# Copyright 2014-2015 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if(NOT CPACK_WIX_ROOT)
  file(TO_CMAKE_PATH "$ENV{WIX}" CPACK_WIX_ROOT)
endif()

find_program(CPACK_WIX_CANDLE_EXECUTABLE candle
  PATHS "${CPACK_WIX_ROOT}/bin")

if(NOT CPACK_WIX_CANDLE_EXECUTABLE)
  message(FATAL_ERROR "Could not find the WiX candle executable.")
endif()

find_program(CPACK_WIX_LIGHT_EXECUTABLE light
  PATHS "${CPACK_WIX_ROOT}/bin")

if(NOT CPACK_WIX_LIGHT_EXECUTABLE)
  message(FATAL_ERROR "Could not find the WiX light executable.")
endif()
