# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


class SkiaXCodeApi(recipe_api.RecipeApi):
  XCODE_BUILD_VERSION = '16a242d' # Xcode 16.0

  @property
  def version(self):
    return self.XCODE_BUILD_VERSION

  @property
  def path(self):
    return self.m.vars.cache_dir.joinpath('Xcode.app')

  def install(self):
    # XCode build is listed in parentheses after the version at
    # https://developer.apple.com/news/releases/, or on Wikipedia here:
    # https://en.wikipedia.org/wiki/Xcode#Version_comparison_table
    # Use lowercase letters.
    # https://chrome-infra-packages.appspot.com/p/infra_internal/ios/xcode

    # Copied from
    # https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/self.mpy#322
    with self.m.step.nest('ensure xcode') as step_result:
      mac_toolchain_cmd = self.m.vars.workdir.joinpath(
          'mac_toolchain', 'mac_toolchain')
      # Download mac_toolchain if necessary.
      if not self.m.path.exists(mac_toolchain_cmd):
        package_path = 'infra/tools/mac_toolchain'
        self.m.cipd.ensure_tool(
            package_path + '/${platform}',
            'git_revision:0cb1e51344de158f72524c384f324465aebbcef2',
            'mac_toolchain')
        # Find where CIPD downloaded the package.
        search_dir = self.m.vars.workdir.joinpath('cipd_tool', package_path)
        contents = self.m.file.listdir('locate mac_toolchain', search_dir, test_data=['abc123'])
        if len(contents) != 1:
          raise Exception('Expected exactly one subdirectory in %s but found: %v' % (
              search_dir, contents))  # pragma: nocover
        self.m.step('ls -R cipd_tool', ['ls', '-R', self.m.vars.workdir.joinpath('cipd_tool')])
        self.m.step('ls -R cipd_tool', ['ls', '-R', search_dir])
        mac_toolchain_cmd = contents[0].joinpath('mac_toolchain')
        self.m.step('ls -alh mac_toolchain', ['ls', '-alh', mac_toolchain_cmd])

      # Ensure XCode is installed.
      xcode_app_path = self.path
      step_result.step_summary_text = (
          'Ensuring Xcode version %s in %s' % (
              self.version, xcode_app_path))
      install_xcode_cmd = [
          mac_toolchain_cmd, 'install',
          # "ios" is needed for simulator builds
          # (Build-Mac-Clang-x64-Release-iOS).
          '-kind', 'ios',
          '-xcode-version', self.version,
          '-output-dir', xcode_app_path,
      ]
      self.m.step('install xcode', install_xcode_cmd)
      self.m.step('select xcode', [
          'sudo', 'xcode-select', '-switch', xcode_app_path])
