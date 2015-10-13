vars = {
  'chromium_git': 'https://chromium.googlesource.com',
}

deps = {
  "third_party/gyp":
      Var('chromium_git') + "/external/gyp@81c2e5ff92af29bab61c982808076ddce3d200a2",

  # TODO(kbr): figure out how to better stay in sync with Chromium's
  # versions of googletest and googlemock.
  "src/tests/third_party/googletest":
      Var('chromium_git') + "/external/googletest.git@9855a87157778d39b95eccfb201a9dc90f6d61c6",

  "src/tests/third_party/googlemock":
      Var('chromium_git') + "/external/googlemock.git@b2cb211e49d872101d991201362d7b97d7d69910",

  "third_party/deqp/src":
      "https://android.googlesource.com/platform/external/deqp@92f7752da82925ca5e7288c5b4814efa7a381d89",

  "third_party/libpng":
      "https://android.googlesource.com/platform/external/libpng@094e181e79a3d6c23fd005679025058b7df1ad6c",

  "third_party/zlib":
      Var('chromium_git') + "/chromium/src/third_party/zlib@afd8c4593c010c045902f6c0501718f1823064a3",

  "buildtools":
      Var('chromium_git') + '/chromium/buildtools.git@125d157607de4d7c95bf8b02dd580aae17962f19',
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },
  {
    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
    "pattern": ".",
    "action": ["python", "build/gyp_angle"],
  },
]
