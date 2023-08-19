"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 DOES NOT HAVE ONE AT ALL.

This module defines rules for running JS tests in a browser.

"""

# https://github.com/bazelbuild/rules_webtesting/blob/master/web/web.bzl
load("@io_bazel_rules_webtesting//web:web.bzl", "web_test")

# https://github.com/google/skia-buildbot/blob/main/bazel/test_on_env/test_on_env.bzl
load("@org_skia_go_infra//bazel/test_on_env:test_on_env.bzl", "test_on_env")

def karma_test(name, config_file, srcs, static_files = None, env = None, **kwargs):
    """Tests the given JS files using Karma and a browser provided by Bazel (Chromium)

    This rule injects some JS code into the karma config file and produces both that modified
    configuration file and a bash script which invokes Karma. That script is then invoked
    in an environment that has the Bazel-downloaded browser available and the tests run using it.

    When invoked via `bazel test`, the test runs in headless mode. When invoked via `bazel run`,
    a visible web browser appears for the user to inspect and debug.

    This draws inspiration from the karma_web_test implementation in concatjs
    https://github.com/bazelbuild/rules_nodejs/blob/700b7a3c5f97f2877320e6e699892ee706f85269/packages/concatjs/web_test/karma_web_test.bzl
    https://github.com/bazelbuild/rules_nodejs/blob/c0b5865d7926298206e9a6aa32138967403ad1f2/packages/concatjs/web_test/karma_web_test.bzl
    but we were unable to use it because they prevented us from defining some proxies ourselves,
    which we need in order to communicate our test gms (PNG files) to a server that runs alongside
    the test. This implementation is simpler than concatjs's and does not try to work for all
    situations nor bundle everything together.

    Args:
      name: The name of the rule which actually runs the tests. generated dependent rules will use
        this name plus an applicable suffix.
      config_file: A karma config file. The user is to expect a function called BAZEL_APPLY_SETTINGS
        is defined and should call it with the configuration object before passing it to config.set.
      srcs: A list of JavaScript test files or helpers.
      static_files: Arbitrary files which are available to be loaded.
        Files are served at:
          - `/static/<WORKSPACE_NAME>/<path-to-file>` or
          - `/static/<WORKSPACE_NAME>/<path-to-rule>/<file>`
        Examples:
          - `/static/skia/modules/canvaskit/tests/assets/color_wheel.gif`
          - `/static/skia/modules/canvaskit/canvaskit_wasm/canvaskit.wasm`
      env: An optional label to a binary. If set, the test will be wrapped in a test_on_env rule,
        and this binary will be used as the "env" part of test_on_env. It will be started before
        the tests run and be running in parallel to them. See the test_on_env.bzl in the
        Skia Infra repo for more.
      **kwargs: Additional arguments are passed to @io_bazel_rules_webtesting/web_test.
    """
    if len(srcs) == 0:
        fail("Must pass at least one file into srcs or there will be no tests to run")
    if not static_files:
        static_files = []

    karma_test_name = name + "_karma_test"
    _karma_test(
        name = karma_test_name,
        srcs = srcs,
        config_file = config_file,
        static_files = static_files,
        visibility = ["//visibility:private"],
        tags = ["manual", "no-remote"],
    )

    # See the following link for the options.
    # https://github.com/bazelbuild/rules_webtesting/blob/e9cf17123068b1123c68219edf9b274bf057b9cc/web/internal/web_test.bzl#L164
    # TODO(kjlubick) consider using web_test_suite to test on Firefox as well.
    if not env:
        web_test(
            name = name,
            launcher = ":" + karma_test_name,
            browser = "@io_bazel_rules_webtesting//browsers:chromium-local",
            test = karma_test_name,
            tags = [
                # https://bazel.build/reference/be/common-definitions#common.tags
                "no-remote",
                # native is required to be set by web_test for reasons that are not
                # abundantly clear.
                "native",
            ],
            **kwargs
        )
    else:
        web_test_name = name + "_web_test"
        web_test(
            name = web_test_name,
            launcher = ":" + karma_test_name,
            browser = "@io_bazel_rules_webtesting//browsers:chromium-local",
            test = karma_test_name,
            visibility = ["//visibility:private"],
            tags = [
                # https://bazel.build/reference/be/common-definitions#common.tags
                "no-remote",
                "manual",
                # native is required to be set by web_test for reasons that are not
                # abundantly clear.
                "native",
            ],
            **kwargs
        )
        test_on_env(
            name = name,
            env = env,
            test = ":" + web_test_name,
            test_on_env_binary = "@org_skia_go_infra//bazel/test_on_env:test_on_env",
            tags = ["no-remote"],
        )

# This JS code is injected into the the provided karma configuration file. It contains
# Bazel-specific logic that could be re-used across different configuration files.
# Concretely, it sets up the browser configuration and whether we want to just run the tests
# and exit (e.g. the user ran `bazel test foo`) or if we want to have an interactive session
# (e.g. the user ran `bazel run foo`).
_apply_bazel_settings_js_code = """
(function(cfg) {
// This is is a JS function provided via environment variables to let us resolve files
// https://bazelbuild.github.io/rules_nodejs/Built-ins.html#nodejs_binary-templated_args
const runfiles = require(process.env['BAZEL_NODE_RUNFILES_HELPER']);

// Apply the paths to any files that are coming from other Bazel rules (e.g. compiled JS).
function addFilePaths(cfg) {
  if (!cfg.files) {
    cfg.files = [];
  }
  cfg.files = cfg.files.concat([_BAZEL_SRCS]);
  cfg.basePath = "_BAZEL_BASE_PATH";

  if (!cfg.proxies) {
    cfg.proxies = {};
  }
  // The following is based off of the concatjs version
  // https://github.com/bazelbuild/rules_nodejs/blob/700b7a3c5f97f2877320e6e699892ee706f85269/packages/concatjs/web_test/karma.conf.js#L276
  const staticFiles = [_BAZEL_STATIC_FILES];
  for (const file of staticFiles) {
    // We need to find the actual path (symlinks can apparently cause issues on Windows).
    const resolvedFile = runfiles.resolve(file);
    cfg.files.push({pattern: resolvedFile, included: false});
    // We want the file to be available on a path according to its location in the workspace
    // (and not the path on disk), so we use a proxy to redirect.
    // Prefixing the proxy path with '/absolute' allows karma to load files that are not
    // underneath the basePath. This doesn't see to be an official API.
    // https://github.com/karma-runner/karma/issues/2703
    cfg.proxies['/static/' + file] = '/absolute' + resolvedFile;
  }
}

// Returns true if invoked with bazel run, i.e. the user wants to see the results on a real
// browser.
function isBazelRun() {
  // This env var seems to be a good indicator on Linux, at least.
  return !!process.env['DISPLAY'];
}

// Configures the settings to run chrome.
function applyChromiumSettings(cfg, chromiumPath) {
  if (isBazelRun()) {
    cfg.browsers = ['Chrome'];
    cfg.singleRun = false;
  } else {
    // Invoked via bazel test, so run the tests once in a headless browser and be done
    // When running on the CI, we saw errors like "No usable sandbox! Update your kernel or ..
    // --no-sandbox". concatjs's version https://github.com/bazelbuild/rules_nodejs/blob/700b7a3c5f97f2877320e6e699892ee706f85269/packages/concatjs/web_test/karma.conf.js#L69
    // detects if sandboxing is supported, but to avoid that complexity, we just always disable
    // the sandbox. https://docs.travis-ci.com/user/chrome#karma-chrome-launcher
    cfg.browsers = ['ChromeHeadlessNoSandbox'];
    cfg.customLaunchers = {
      'ChromeHeadlessNoSandbox': {
        'base': 'ChromeHeadless',
        'flags': [
          '--no-sandbox',
          // may help tests be less flaky
          // https://peter.sh/experiments/chromium-command-line-switches/#browser-test
          '--browser-test',
        ],
      },
    }
    cfg.singleRun = true;
  }

  try {
    // Setting the CHROME_BIN environment variable tells Karma which chrome to use.
    // We want it to use the Chrome brought via Bazel.
    process.env.CHROME_BIN = runfiles.resolve(chromiumPath);
  } catch {
    throw new Error(`Failed to resolve Chromium binary '${chromiumPath}' in runfiles`);
  }
}

function applyBazelSettings(cfg) {
  addFilePaths(cfg)

  // This is a JSON file that contains this metadata, mixed in with some other data, e.g.
  // the link to the correct executable for the given platform.
  // https://github.com/bazelbuild/rules_webtesting/blob/e9cf17123068b1123c68219edf9b274bf057b9cc/browsers/chromium-local.json
  const webTestMetadata = require(runfiles.resolve(process.env['WEB_TEST_METADATA']));

  const webTestFiles = webTestMetadata['webTestFiles'][0];
  const path = webTestFiles['namedFiles']['CHROMIUM'];
  if (path) {
    applyChromiumSettings(cfg, path);
  } else {
    throw new Error("not supported yet");
  }
}

applyBazelSettings(cfg)

function addPlugins(cfg) {
    // Without listing these plugins, they will not be loaded (kjlubick suspects
    // this has to do with karma/npm not being able to find them "globally"
    // via some automagic process).
    cfg.plugins = [
      'karma-jasmine',
      'karma-chrome-launcher',
      'karma-firefox-launcher',
    ];
}

addPlugins(cfg)

// The user is expected to treat the BAZEL_APPLY_SETTINGS as a function name and pass in
// the configuration as a parameter. Thus, we need to end such that our IIFE will be followed
// by the parameter in parentheses and get passed in as cfg.
})"""

def _expand_templates_in_karma_config(ctx):
    # Wrap the absolute paths of our files in quotes and make them comma separated so they
    # can go in the Karma files list.
    srcs = ['"{}"'.format(_absolute_path(ctx, f)) for f in ctx.files.srcs]
    src_list = ", ".join(srcs)

    # Set our base path to that which contains the karma configuration file.
    # This requires going up a few directory segments. This allows our absolute paths to
    # all be compatible with each other.
    config_segments = len(ctx.outputs.configuration.short_path.split("/"))
    base_path = "/".join([".."] * config_segments)

    static_files = ['"{}"'.format(_absolute_path(ctx, f)) for f in ctx.files.static_files]
    static_list = ", ".join(static_files)

    # Replace the placeholders in the embedded JS with those files. We cannot use .format() because
    # the curly braces from the JS code throw it off.
    apply_bazel_settings = _apply_bazel_settings_js_code.replace("_BAZEL_SRCS", src_list)
    apply_bazel_settings = apply_bazel_settings.replace("_BAZEL_BASE_PATH", base_path)
    apply_bazel_settings = apply_bazel_settings.replace("_BAZEL_STATIC_FILES", static_list)

    # Add in the JS fragment that applies the Bazel-specific settings to the provided config.
    # https://docs.bazel.build/versions/main/skylark/lib/actions.html#expand_template
    ctx.actions.expand_template(
        output = ctx.outputs.configuration,
        template = ctx.file.config_file,
        substitutions = {
            "BAZEL_APPLY_SETTINGS": apply_bazel_settings,
        },
    )

def _absolute_path(ctx, file):
    # Referencing things in @npm yields a short_path that starts with ../
    # For those cases, we can just remove the ../
    if file.short_path.startswith("../"):
        return file.short_path[3:]

    # Otherwise, we have a local file, so we need to include the workspace path to make it
    # an absolute path
    return ctx.workspace_name + "/" + file.short_path

_invoke_karma_bash_script = """#!/usr/bin/env bash
# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
# https://github.com/bazelbuild/bazel/blob/master/tools/bash/runfiles/runfiles.bash
set -uo pipefail; f=build_bazel_rules_nodejs/third_party/github.com/bazelbuild/bazel/tools/bash/runfiles/runfiles.bash
source "${{RUNFILES_DIR:-/dev/null}}/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "${{RUNFILES_MANIFEST_FILE:-/dev/null}}" | cut -f2- -d' ')" 2>/dev/null || \
  source "$0.runfiles/$f" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
  {{ echo>&2 "ERROR: cannot find $f"; exit 1; }}; f=; set -e
# --- end runfiles.bash initialization v2 ---

readonly KARMA=$(rlocation "{_KARMA_EXECUTABLE_SCRIPT}")
readonly CONF=$(rlocation "{_KARMA_CONFIGURATION_FILE}")

# set a temporary directory as the home directory, because otherwise Chrome fails to
# start up, complaining about a read-only file system. This does not get cleaned up automatically
# by Bazel, so we do so after Karma finishes.
export HOME=$(mktemp -d)

readonly COMMAND="${{KARMA}} "start" ${{CONF}}"
${{COMMAND}}
KARMA_EXIT_CODE=$?
echo "Karma returned ${{KARMA_EXIT_CODE}}"
# Attempt to clean up the temporary home directory. If this fails, that's not a big deal because
# the contents are small and will be cleaned up by the OS on reboot.
rm -rf $HOME || true
exit $KARMA_EXIT_CODE
"""

def _create_bash_script_to_invoke_karma(ctx):
    ctx.actions.write(
        output = ctx.outputs.executable,
        is_executable = True,
        content = _invoke_karma_bash_script.format(
            _KARMA_EXECUTABLE_SCRIPT = _absolute_path(ctx, ctx.executable.karma),
            _KARMA_CONFIGURATION_FILE = _absolute_path(ctx, ctx.outputs.configuration),
        ),
    )

def _karma_test_impl(ctx):
    _expand_templates_in_karma_config(ctx)
    _create_bash_script_to_invoke_karma(ctx)

    # The files that need to be included when we run the bash script that invokes Karma are:
    #   - The templated configuration file
    #   - Any JS test files the user provided
    #   - Any static files the user specified
    runfiles = [
        ctx.outputs.configuration,
    ]
    runfiles += ctx.files.srcs
    runfiles += ctx.files.static_files

    # Now we combine this with the files necessary to run Karma
    # (which includes the plugins as data dependencies).
    karma_files = ctx.attr.karma[DefaultInfo].default_runfiles

    # https://bazel.build/rules/lib/builtins/ctx#runfiles
    combined_runfiles = ctx.runfiles(files = runfiles).merge(karma_files)

    # https://bazel.build/rules/lib/providers/DefaultInfo
    return [DefaultInfo(
        runfiles = combined_runfiles,
        executable = ctx.outputs.executable,
    )]

_karma_test = rule(
    implementation = _karma_test_impl,
    test = True,
    executable = True,
    attrs = {
        "config_file": attr.label(
            doc = "The karma config file",
            mandatory = True,
            allow_single_file = [".js"],
        ),
        "srcs": attr.label_list(
            doc = "A list of JavaScript test files",
            allow_files = [".js"],
            mandatory = True,
        ),
        "karma": attr.label(
            doc = "karma binary label",
            # By default, we use the karma pulled in via Bazel running npm install
            # that has extra data dependencies for the necessary plugins.
            default = "//bazel/karma:karma_with_plugins",
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
        "static_files": attr.label_list(
            doc = "Additional files which are available to be loaded",
            allow_files = True,
        ),
    },
    outputs = {
        "configuration": "%{name}.conf.js",
    },
)
