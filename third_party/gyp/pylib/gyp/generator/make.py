#!/usr/bin/python

# Copyright (c) 2009 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Notes:
#
# This is all roughly based on the Makefile system used by the Linux
# kernel, but is a non-recursive make -- we put the entire dependency
# graph in front of make and let it figure it out.
#
# The code below generates a separate .mk file for each target, but
# all are sourced by the top-level Makefile.  This means that all
# variables in .mk-files clobber one another.  Be careful to use :=
# where appropriate for immediate evaluation, and similarly to watch
# that you're not relying on a variable value to last beween different
# .mk files.
#
# TODOs:
#
# Global settings and utility functions are currently stuffed in the
# toplevel Makefile.  It may make sense to generate some .mk files on
# the side to keep the the files readable.

import gyp
import gyp.common
import gyp.system_test
import os.path
import os

# Debugging-related imports -- remove me once we're solid.
import code
import pprint

generator_default_variables = {
  'EXECUTABLE_PREFIX': '',
  'EXECUTABLE_SUFFIX': '',
  'OS': 'linux',
  'STATIC_LIB_PREFIX': 'lib',
  'SHARED_LIB_PREFIX': 'lib',
  'STATIC_LIB_SUFFIX': '.a',
  'SHARED_LIB_SUFFIX': '.so',
  'INTERMEDIATE_DIR': '$(obj).$(TOOLSET)/geni',
  'SHARED_INTERMEDIATE_DIR': '$(obj)/gen',
  'PRODUCT_DIR': '$(builddir)',
  'SHARED_LIB_DIR': '$(builddir)/lib.$(TOOLSET)',
  'LIB_DIR': '$(obj).$(TOOLSET)',
  'RULE_INPUT_ROOT': '%(INPUT_ROOT)s',  # This gets expanded by Python.
  'RULE_INPUT_PATH': '$(abspath $<)',
  'RULE_INPUT_EXT': '$(suffix $<)',
  'RULE_INPUT_NAME': '$(notdir $<)',

  # This appears unused --- ?
  'CONFIGURATION_NAME': '$(BUILDTYPE)',
}

# Make supports multiple toolsets
generator_supports_multiple_toolsets = True

def ensure_directory_exists(path):
  dir = os.path.dirname(path)
  if dir and not os.path.exists(dir):
    os.makedirs(dir)

# Header of toplevel Makefile.
# This should go into the build tree, but it's easier to keep it here for now.
SHARED_HEADER = ("""\
# We borrow heavily from the kernel build setup, though we are simpler since
# we don't have Kconfig tweaking settings on us.

# The implicit make rules have it looking for RCS files, among other things.
# We instead explicitly write all the rules we care about.
# It's even quicker (saves ~200ms) to pass -r on the command line.
MAKEFLAGS=-r

# The source directory tree.
srcdir := %(srcdir)s

# The name of the builddir.
builddir_name ?= %(builddir)s

# The V=1 flag on command line makes us verbosely print command lines.
ifdef V
  quiet=
else
  quiet=quiet_
endif

# Specify BUILDTYPE=Release on the command line for a release build.
BUILDTYPE ?= %(default_configuration)s

# Directory all our build output goes into.
# Note that this must be two directories beneath src/ for unit tests to pass,
# as they reach into the src/ directory for data with relative paths.
builddir ?= $(builddir_name)/$(BUILDTYPE)
abs_builddir := $(abspath $(builddir))
depsdir := $(builddir)/.deps

# Object output directory.
obj := $(builddir)/obj
abs_obj := $(abspath $(obj))

# We build up a list of every single one of the targets so we can slurp in the
# generated dependency rule Makefiles in one pass.
all_deps :=

# C++ apps need to be linked with g++.  Not sure what's appropriate.
#
# Note, the flock is used to seralize linking. Linking is a memory-intensive
# process so running parallel links can often lead to thrashing.  To disable
# the serialization, override LINK via an envrionment variable as follows:
#
#   export LINK="$(CXX)"
#
# This will allow make to invoke N linker processes as specified in -jN.
LINK ?= flock $(builddir)/linker.lock $(CXX) %(LINK_flags)s

CC.target ?= $(CC)
CFLAGS.target ?= $(CFLAGS)
CXX.target ?= $(CXX)
CXXFLAGS.target ?= $(CXXFLAGS)
LINK.target ?= $(LINK)
LDFLAGS.target ?= $(LDFLAGS)
AR.target ?= $(AR)
ARFLAGS.target ?= %(ARFLAGS.target)s

# N.B.: the logic of which commands to run should match the computation done
# in gyp's make.py where ARFLAGS.host etc. is computed.
# TODO(evan): move all cross-compilation logic to gyp-time so we don't need
# to replicate this environment fallback in make as well.
CC.host ?= gcc
CFLAGS.host ?=
CXX.host ?= g++
CXXFLAGS.host ?=
LINK.host ?= g++
LDFLAGS.host ?=
AR.host ?= ar
ARFLAGS.host := %(ARFLAGS.host)s

# Flags to make gcc output dependency info.  Note that you need to be
# careful here to use the flags that ccache and distcc can understand.
# We write to a dep file on the side first and then rename at the end
# so we can't end up with a broken dep file.
depfile = $(depsdir)/$@.d
DEPFLAGS = -MMD -MF $(depfile).raw

# We have to fixup the deps output in a few ways.
# (1) the file output should mention the proper .o file.
# ccache or distcc lose the path to the target, so we convert a rule of
# the form:
#   foobar.o: DEP1 DEP2
# into
#   path/to/foobar.o: DEP1 DEP2
# (2) we want missing files not to cause us to fail to build.
# We want to rewrite
#   foobar.o: DEP1 DEP2 \\
#               DEP3
# to
#   DEP1:
#   DEP2:
#   DEP3:
# so if the files are missing, they're just considered phony rules.
# We have to do some pretty insane escaping to get those backslashes
# and dollar signs past make, the shell, and sed at the same time."""
r"""
define fixup_dep
# The depfile may not exist if the input file didn't have any #includes.
touch $(depfile).raw
# Fixup path as in (1).
sed -e "s|^$(notdir $@)|$@|" $(depfile).raw >> $(depfile)
# Add extra rules as in (2).
# We remove slashes and replace spaces with new lines;
# remove blank lines;
# delete the first line and append a colon to the remaining lines.
sed -e 's|\\||' -e 'y| |\n|' $(depfile).raw |\
  grep -v '^$$'                             |\
  sed -e 1d -e 's|$$|:|'                     \
    >> $(depfile)
rm $(depfile).raw
endef
"""
"""
# Command definitions:
# - cmd_foo is the actual command to run;
# - quiet_cmd_foo is the brief-output summary of the command.

quiet_cmd_cc = CC($(TOOLSET)) $@
cmd_cc = $(CC.$(TOOLSET)) $(GYP_CFLAGS) $(DEPFLAGS) $(CFLAGS.$(TOOLSET)) -c -o $@ $<

quiet_cmd_cxx = CXX($(TOOLSET)) $@
cmd_cxx = $(CXX.$(TOOLSET)) $(GYP_CXXFLAGS) $(DEPFLAGS) $(CXXFLAGS.$(TOOLSET)) -c -o $@ $<

quiet_cmd_alink = AR($(TOOLSET)) $@
cmd_alink = rm -f $@ && $(AR.$(TOOLSET)) $(ARFLAGS.$(TOOLSET)) $@ $(filter %%.o,$^)

quiet_cmd_touch = TOUCH $@
cmd_touch = touch $@

quiet_cmd_copy = COPY $@
# send stderr to /dev/null to ignore messages when linking directories.
cmd_copy = ln -f $< $@ 2>/dev/null || cp -af $< $@

# Due to circular dependencies between libraries :(, we wrap the
# special "figure out circular dependencies" flags around the entire
# input list during linking.
quiet_cmd_link = LINK($(TOOLSET)) $@
cmd_link = $(LINK.$(TOOLSET)) $(GYP_LDFLAGS) $(LDFLAGS.$(TOOLSET)) -o $@ -Wl,--start-group $(filter-out FORCE_DO_CMD, $^) -Wl,--end-group $(LIBS)

# Shared-object link (for generating .so).
# Set SONAME to the library filename so our binaries don't reference the local,
# absolute paths used on the link command-line.
# TODO: perhaps this can share with the LINK command above?
quiet_cmd_solink = SOLINK($(TOOLSET)) $@
cmd_solink = $(LINK.$(TOOLSET)) -shared $(GYP_LDFLAGS) $(LDFLAGS.$(TOOLSET)) -Wl,-soname=$(@F) -o $@ -Wl,--start-group $(filter-out FORCE_DO_CMD, $^) -Wl,--end-group $(LIBS)
"""
r"""
# Define an escape_quotes function to escape single quotes.
# This allows us to handle quotes properly as long as we always use
# use single quotes and escape_quotes.
escape_quotes = $(subst ','\'',$(1))
# This comment is here just to include a ' to unconfuse syntax highlighting.
# Define an escape_vars function to escape '$' variable syntax.
# This allows us to read/write command lines with shell variables (e.g.
# $LD_LIBRARY_PATH), without triggering make substitution.
escape_vars = $(subst $$,$$$$,$(1))
# Helper that expands to a shell command to echo a string exactly as it is in
# make. This uses printf instead of echo because printf's behaviour with respect
# to escape sequences is more portable than echo's across different shells
# (e.g., dash, bash).
exact_echo = printf '%%s\n' '$(call escape_quotes,$(1))'
"""
"""
# Helper to compare the command we're about to run against the command
# we logged the last time we ran the command.  Produces an empty
# string (false) when the commands match.
# Tricky point: Make has no string-equality test function.
# The kernel uses the following, but it seems like it would have false
# positives, where one string reordered its arguments.
#   arg_check = $(strip $(filter-out $(cmd_$(1)), $(cmd_$@)) \\
#                       $(filter-out $(cmd_$@), $(cmd_$(1))))
# We instead substitute each for the empty string into the other, and
# say they're equal if both substitutions produce the empty string.
command_changed = $(or $(subst $(cmd_$(1)),,$(cmd_$@)),\\
                       $(subst $(cmd_$@),,$(cmd_$(1))))

# Helper that is non-empty when a prerequisite changes.
# Normally make does this implicitly, but we force rules to always run
# so we can check their command lines.
#   $? -- new prerequisites
#   $| -- order-only dependencies
prereq_changed = $(filter-out $|,$?)

# do_cmd: run a command via the above cmd_foo names, if necessary.
# Should always run for a given target to handle command-line changes.
# Second argument, if non-zero, makes it do asm/C/C++ dependency munging.
define do_cmd
$(if $(or $(command_changed),$(prereq_changed)),
  @$(call exact_echo,  $($(quiet)cmd_$(1)))
  @mkdir -p $(dir $@) $(dir $(depfile))
  @$(cmd_$(1))
  @$(call exact_echo,$(call escape_vars,cmd_$@ := $(cmd_$(1)))) > $(depfile)
  @$(if $(2),$(fixup_dep))
)
endef

# Declare "all" target first so it is the default, even though we don't have the
# deps yet.
.PHONY: all
all:

# Use FORCE_DO_CMD to force a target to run.  Should be coupled with
# do_cmd.
.PHONY: FORCE_DO_CMD
FORCE_DO_CMD:

""")

ROOT_HEADER_SUFFIX_RULES = ("""\
# Suffix rules, putting all outputs into $(obj).
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(srcdir)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)

# Try building from generated source, too.
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj).$(TOOLSET)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)

$(obj).$(TOOLSET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
$(obj).$(TOOLSET)/%.o: $(obj)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
""")

SHARED_HEADER_SUFFIX_RULES_COMMENT1 = ("""\
# Suffix rules, putting all outputs into $(obj).
""")

SHARED_HEADER_SUFFIX_RULES_SRCDIR = {
    '.c': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
"""),
    '.s': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.s FORCE_DO_CMD
	@$(call do_cmd,cc,1)
"""),
    '.S': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.S FORCE_DO_CMD
	@$(call do_cmd,cc,1)
"""),
    '.cpp': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
    '.cc': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
    '.cxx': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(srcdir)/%.cxx FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
}

SHARED_HEADER_SUFFIX_RULES_COMMENT2 = ("""\
# Try building from generated source, too.
""")

SHARED_HEADER_SUFFIX_RULES_OBJDIR1 = {
    '.c': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
"""),
    '.cc': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
    '.cpp': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj).$(TOOLSET)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
}

SHARED_HEADER_SUFFIX_RULES_OBJDIR2 = {
    '.c': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.c FORCE_DO_CMD
	@$(call do_cmd,cc,1)
"""),
    '.cc': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.cc FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
    '.cpp': ("""\
$(obj).$(TOOLSET)/$(TARGET)/%.o: $(obj)/%.cpp FORCE_DO_CMD
	@$(call do_cmd,cxx,1)
"""),
}

SHARED_HEADER_SUFFIX_RULES = (
    SHARED_HEADER_SUFFIX_RULES_COMMENT1 +
    ''.join(SHARED_HEADER_SUFFIX_RULES_SRCDIR.values()) +
    SHARED_HEADER_SUFFIX_RULES_COMMENT2 +
    ''.join(SHARED_HEADER_SUFFIX_RULES_OBJDIR1.values()) +
    ''.join(SHARED_HEADER_SUFFIX_RULES_OBJDIR2.values())
)

SHARED_FOOTER = """\
# "all" is a concatenation of the "all" targets from all the included
# sub-makefiles. This is just here to clarify.
all:

# Add in dependency-tracking rules.  $(all_deps) is the list of every single
# target in our tree.  First, only consider targets that already have been
# built, as unbuilt targets will be built regardless of dependency info:
all_deps := $(wildcard $(sort $(all_deps)))
# Of those, only consider the ones with .d (dependency) info:
d_files := $(wildcard $(foreach f,$(all_deps),$(depsdir)/$(f).d))
ifneq ($(d_files),)
  # Rather than include each individual .d file, concatenate them into a
  # single file which make is able to load faster.  We split this into
  # commands that take 1000 files at a time to avoid overflowing the
  # command line.
  $(shell cat $(wordlist 1,1000,$(d_files)) > $(depsdir)/all.deps)
%(generate_all_deps)s
  # make looks for ways to re-generate included makefiles, but in our case, we
  # don't have a direct way. Explicitly telling make that it has nothing to do
  # for them makes it go faster.
  $(depsdir)/all.deps: ;

  include $(depsdir)/all.deps
endif
"""

header = """\
# This file is generated by gyp; do not edit.

"""


def Compilable(filename):
  """Return true if the file is compilable (should be in OBJS)."""
  for res in (filename.endswith(e) for e
             in ['.c', '.cc', '.cpp', '.cxx', '.s', '.S']):
    if res:
      return True
  return False


def Linkable(filename):
  """Return true if the file is linkable (should be on the link line)."""
  return filename.endswith('.o')


def Target(filename):
  """Translate a compilable filename to its .o target."""
  return os.path.splitext(filename)[0] + '.o'


def EscapeShellArgument(s):
  """Quotes an argument so that it will be interpreted literally by a POSIX
     shell. Taken from
     http://stackoverflow.com/questions/35817/whats-the-best-way-to-escape-ossystem-calls-in-python
     """
  return "'" + s.replace("'", "'\\''") + "'"


def EscapeMakeVariableExpansion(s):
  """Make has its own variable expansion syntax using $. We must escape it for
     string to be interpreted literally."""
  return s.replace('$', '$$')


def EscapeCppDefine(s):
  """Escapes a CPP define so that it will reach the compiler unaltered."""
  s = EscapeShellArgument(s)
  s = EscapeMakeVariableExpansion(s)
  return s


def QuoteIfNecessary(string):
  """TODO: Should this ideally be replaced with one or more of the above
     functions?"""
  if '"' in string:
    string = '"' + string.replace('"', '\\"') + '"'
  return string


def StringToMakefileVariable(string):
  """Convert a string to a value that is acceptable as a make variable name."""
  # TODO: replace other metacharacters that we encounter.
  return string.replace(' ', '_')


srcdir_prefix = ''
def Sourceify(path):
  """Convert a path to its source directory form."""
  if '$(' in path:
    return path
  if os.path.isabs(path):
    return path
  return srcdir_prefix + path


# Map from qualified target to path to output.
target_outputs = {}
# Map from qualified target to a list of all linker dependencies,
# transitively expanded.
# Used in building shared-library-based executables.
target_link_deps = {}


class MakefileWriter:
  """MakefileWriter packages up the writing of one target-specific foobar.mk.

  Its only real entry point is Write(), and is mostly used for namespacing.
  """

  def __init__(self):
    # Keep track of the total number of outputs for this makefile.
    self._num_outputs = 0


  def NumOutputs(self):
    return self._num_outputs


  def Write(self, qualified_target, base_path, output_filename, spec, configs,
            part_of_all):
    """The main entry point: writes a .mk file for a single target.

    Arguments:
      qualified_target: target we're generating
      base_path: path relative to source root we're building in, used to resolve
                 target-relative paths
      output_filename: output .mk file name to write
      spec, configs: gyp info
      part_of_all: flag indicating this target is part of 'all'
    """
    ensure_directory_exists(output_filename)

    self.fp = open(output_filename, 'w')

    self.fp.write(header)

    self.path = base_path
    self.target = spec['target_name']
    self.type = spec['type']
    self.toolset = spec['toolset']

    deps, link_deps = self.ComputeDeps(spec)

    # Some of the generation below can add extra output, sources, or
    # link dependencies.  All of the out params of the functions that
    # follow use names like extra_foo.
    extra_outputs = []
    extra_sources = []
    extra_link_deps = []

    self.output = self.ComputeOutput(spec)
    self._INSTALLABLE_TARGETS = ('executable', 'loadable_module',
                                 'shared_library')
    if self.type in self._INSTALLABLE_TARGETS:
      self.alias = os.path.basename(self.output)
      install_path = self._InstallableTargetInstallPath()
    else:
      self.alias = self.output
      install_path = self.output

    self.WriteLn("TOOLSET := " + self.toolset)
    self.WriteLn("TARGET := " + self.target)

    # Actions must come first, since they can generate more OBJs for use below.
    if 'actions' in spec:
      self.WriteActions(spec['actions'], extra_sources, extra_outputs,
                        part_of_all)

    # Rules must be early like actions.
    if 'rules' in spec:
      self.WriteRules(spec['rules'], extra_sources, extra_outputs, part_of_all)

    if 'copies' in spec:
      self.WriteCopies(spec['copies'], extra_outputs, part_of_all)

    all_sources = spec.get('sources', []) + extra_sources
    if all_sources:
      self.WriteSources(configs, deps, all_sources,
                        extra_outputs, extra_link_deps, part_of_all)
      sources = filter(Compilable, all_sources)
      if sources:
        self.WriteLn(SHARED_HEADER_SUFFIX_RULES_COMMENT1)
        extensions = set([os.path.splitext(s)[1] for s in sources])
        for ext in extensions:
          if ext in SHARED_HEADER_SUFFIX_RULES_SRCDIR:
            self.WriteLn(SHARED_HEADER_SUFFIX_RULES_SRCDIR[ext])
        self.WriteLn(SHARED_HEADER_SUFFIX_RULES_COMMENT2)
        for ext in extensions:
          if ext in SHARED_HEADER_SUFFIX_RULES_OBJDIR1:
            self.WriteLn(SHARED_HEADER_SUFFIX_RULES_OBJDIR1[ext])
        for ext in extensions:
          if ext in SHARED_HEADER_SUFFIX_RULES_OBJDIR2:
            self.WriteLn(SHARED_HEADER_SUFFIX_RULES_OBJDIR2[ext])
        self.WriteLn('# End of this set of suffix rules')


    self.WriteTarget(spec, configs, deps,
                     extra_link_deps + link_deps, extra_outputs, part_of_all)

    # Update global list of target outputs, used in dependency tracking.
    target_outputs[qualified_target] = install_path

    # Update global list of link dependencies.
    if self.type == 'static_library':
      target_link_deps[qualified_target] = [self.output]
    elif self.type == 'shared_library':
      # Anyone that uses us transitively depend on all of our link
      # dependencies.
      target_link_deps[qualified_target] = [self.output] + link_deps

    self.fp.close()


  def WriteSubMake(self, output_filename, makefile_path, targets, build_dir):
    """Write a "sub-project" Makefile.

    This is a small, wrapper Makefile that calls the top-level Makefile to build
    the targets from a single gyp file (i.e. a sub-project).

    Arguments:
      output_filename: sub-project Makefile name to write
      makefile_path: path to the top-level Makefile
      targets: list of "all" targets for this sub-project
      build_dir: build output directory, relative to the sub-project
    """
    ensure_directory_exists(output_filename)
    self.fp = open(output_filename, 'w')
    self.fp.write(header)
    # For consistency with other builders, put sub-project build output in the
    # sub-project dir (see test/subdirectory/gyptest-subdir-all.py).
    self.WriteLn('export builddir_name ?= %s' %
                 os.path.join(os.path.dirname(output_filename), build_dir))
    self.WriteLn('.PHONY: all')
    self.WriteLn('all:')
    if makefile_path:
      makefile_path = ' -C ' + makefile_path
    self.WriteLn('\t$(MAKE)%s %s' % (makefile_path, ' '.join(targets)))
    self.fp.close()


  def WriteActions(self, actions, extra_sources, extra_outputs, part_of_all):
    """Write Makefile code for any 'actions' from the gyp input.

    extra_sources: a list that will be filled in with newly generated source
                   files, if any
    extra_outputs: a list that will be filled in with any outputs of these
                   actions (used to make other pieces dependent on these
                   actions)
    part_of_all: flag indicating this target is part of 'all'
    """
    for action in actions:
      name = self.target + '_' + StringToMakefileVariable(action['action_name'])
      self.WriteLn('### Rules for action "%s":' % action['action_name'])
      inputs = action['inputs']
      outputs = action['outputs']

      # Build up a list of outputs.
      # Collect the output dirs we'll need.
      dirs = set()
      for out in outputs:
        dir = os.path.split(out)[0]
        if dir:
          dirs.add(dir)
      if int(action.get('process_outputs_as_sources', False)):
        extra_sources += outputs

      # Write the actual command.
      command = gyp.common.EncodePOSIXShellList(action['action'])
      if 'message' in action:
        self.WriteLn('quiet_cmd_%s = ACTION %s $@' % (name, action['message']))
      else:
        self.WriteLn('quiet_cmd_%s = ACTION %s $@' % (name, name))
      if len(dirs) > 0:
        command = 'mkdir -p %s' % ' '.join(dirs) + '; ' + command
      # Set LD_LIBRARY_PATH in case the action runs an executable from this
      # build which links to shared libs from this build.
      if self.path:
        cd_action = 'cd %s; ' % Sourceify(self.path)
      else:
        cd_action = ''
      # actions run on the host, so they should in theory only use host
      # libraries, but until everything is made cross-compile safe, also use
      # target libraries.
      # TODO(piman): when everything is cross-compile safe, remove lib.target
      self.WriteLn('cmd_%s = export LD_LIBRARY_PATH=$(builddir)/lib.host:'
                   '$(builddir)/lib.target:$$LD_LIBRARY_PATH; %s%s'
                   % (name, cd_action, command))
      self.WriteLn()
      outputs = map(self.Absolutify, outputs)
      # The makefile rules are all relative to the top dir, but the gyp actions
      # are defined relative to their containing dir.  This replaces the obj
      # variable for the action rule with an absolute version so that the output
      # goes in the right place.
      # Only write the 'obj' and 'builddir' rules for the "primary" output (:1);
      # it's superfluous for the "extra outputs", and this avoids accidentally
      # writing duplicate dummy rules for those outputs.
      self.WriteMakeRule(outputs[:1], ['obj := $(abs_obj)'])
      self.WriteMakeRule(outputs[:1], ['builddir := $(abs_builddir)'])
      self.WriteDoCmd(outputs, map(Sourceify, map(self.Absolutify, inputs)),
                      part_of_all=part_of_all, command=name)

      # Stuff the outputs in a variable so we can refer to them later.
      outputs_variable = 'action_%s_outputs' % name
      self.WriteLn('%s := %s' % (outputs_variable, ' '.join(outputs)))
      extra_outputs.append('$(%s)' % outputs_variable)
      self.WriteLn()

    self.WriteLn()


  def WriteRules(self, rules, extra_sources, extra_outputs, part_of_all):
    """Write Makefile code for any 'rules' from the gyp input.

    extra_sources: a list that will be filled in with newly generated source
                   files, if any
    extra_outputs: a list that will be filled in with any outputs of these
                   rules (used to make other pieces dependent on these rules)
    part_of_all: flag indicating this target is part of 'all'
    """
    for rule in rules:
      name = self.target + '_' + StringToMakefileVariable(rule['rule_name'])
      count = 0
      self.WriteLn('### Generated for rule %s:' % name)

      all_outputs = []

      for rule_source in rule.get('rule_sources', []):
        dirs = set()
        rule_source_basename = os.path.basename(rule_source)
        (rule_source_root, rule_source_ext) = \
            os.path.splitext(rule_source_basename)

        outputs = [self.ExpandInputRoot(out, rule_source_root)
                   for out in rule['outputs']]
        for out in outputs:
          dir = os.path.dirname(out)
          if dir:
            dirs.add(dir)
          if int(rule.get('process_outputs_as_sources', False)):
            extra_sources.append(out)
        all_outputs += outputs
        inputs = map(Sourceify, map(self.Absolutify, [rule_source] +
                                    rule.get('inputs', [])))
        actions = ['$(call do_cmd,%s_%d)' % (name, count)]

        if name == 'resources_grit':
          # HACK: This is ugly.  Grit intentionally doesn't touch the
          # timestamp of its output file when the file doesn't change,
          # which is fine in hash-based dependency systems like scons
          # and forge, but not kosher in the make world.  After some
          # discussion, hacking around it here seems like the least
          # amount of pain.
          actions += ['@touch --no-create $@']

        # Only write the 'obj' and 'builddir' rules for the "primary" output
        # (:1); it's superfluous for the "extra outputs", and this avoids
        # accidentally writing duplicate dummy rules for those outputs.
        self.WriteMakeRule(outputs[:1], ['obj := $(abs_obj)'])
        self.WriteMakeRule(outputs[:1], ['builddir := $(abs_builddir)'])
        self.WriteMakeRule(outputs, inputs + ['FORCE_DO_CMD'], actions)
        self.WriteLn('all_deps += %s' % ' '.join(outputs))
        self._num_outputs += len(outputs)

        action = [self.ExpandInputRoot(ac, rule_source_root)
                  for ac in rule['action']]
        mkdirs = ''
        if len(dirs) > 0:
          mkdirs = 'mkdir -p %s; ' % ' '.join(dirs)
        if self.path:
          cd_action = 'cd %s; ' % Sourceify(self.path)
        else:
          cd_action = ''
        # Set LD_LIBRARY_PATH in case the rule runs an executable from this
        # build which links to shared libs from this build.
        # rules run on the host, so they should in theory only use host
        # libraries, but until everything is made cross-compile safe, also use
        # target libraries.
        # TODO(piman): when everything is cross-compile safe, remove lib.target
        self.WriteLn(
            "cmd_%(name)s_%(count)d = export LD_LIBRARY_PATH="
              "$(builddir)/lib.host:$(builddir)/lib.target:$$LD_LIBRARY_PATH; "
              "%(cd_action)s%(mkdirs)s%(action)s" % {
          'action': gyp.common.EncodePOSIXShellList(action),
          'cd_action': cd_action,
          'count': count,
          'mkdirs': mkdirs,
          'name': name,
        })
        self.WriteLn(
            'quiet_cmd_%(name)s_%(count)d = RULE %(name)s_%(count)d $@' % {
          'count': count,
          'name': name,
        })
        self.WriteLn()
        count += 1

      outputs_variable = 'rule_%s_outputs' % name
      self.WriteList(all_outputs, outputs_variable)
      extra_outputs.append('$(%s)' % outputs_variable)

      self.WriteLn('### Finished generating for rule: %s' % name)
      self.WriteLn()
    self.WriteLn('### Finished generating for all rules')
    self.WriteLn('')


  def WriteCopies(self, copies, extra_outputs, part_of_all):
    """Write Makefile code for any 'copies' from the gyp input.

    extra_outputs: a list that will be filled in with any outputs of this action
                   (used to make other pieces dependent on this action)
    part_of_all: flag indicating this target is part of 'all'
    """
    self.WriteLn('### Generated for copy rule.')

    variable = self.target + '_copies'
    outputs = []
    for copy in copies:
      for path in copy['files']:
        path = Sourceify(self.Absolutify(path))
        filename = os.path.split(path)[1]
        output = Sourceify(self.Absolutify(os.path.join(copy['destination'],
                                                        filename)))
        self.WriteDoCmd([output], [path], 'copy', part_of_all)
        outputs.append(output)
    self.WriteLn('%s = %s' % (variable, ' '.join(outputs)))
    extra_outputs.append('$(%s)' % variable)
    self.WriteLn()


  def WriteSources(self, configs, deps, sources,
                   extra_outputs, extra_link_deps,
                   part_of_all):
    """Write Makefile code for any 'sources' from the gyp input.
    These are source files necessary to build the current target.

    configs, deps, sources: input from gyp.
    extra_outputs: a list of extra outputs this action should be dependent on;
                   used to serialize action/rules before compilation
    extra_link_deps: a list that will be filled in with any outputs of
                     compilation (to be used in link lines)
    part_of_all: flag indicating this target is part of 'all'
    """

    # Write configuration-specific variables for CFLAGS, etc.
    for configname in sorted(configs.keys()):
      config = configs[configname]
      self.WriteList(config.get('defines'), 'DEFS_%s' % configname, prefix='-D',
          quoter=EscapeCppDefine)
      self.WriteLn("# Flags passed to both C and C++ files.");
      self.WriteList(config.get('cflags'), 'CFLAGS_%s' % configname)
      self.WriteLn("# Flags passed to only C (and not C++) files.");
      self.WriteList(config.get('cflags_c'), 'CFLAGS_C_%s' % configname)
      self.WriteLn("# Flags passed to only C++ (and not C) files.");
      self.WriteList(config.get('cflags_cc'), 'CFLAGS_CC_%s' % configname)
      includes = config.get('include_dirs')
      if includes:
        includes = map(Sourceify, map(self.Absolutify, includes))
      self.WriteList(includes, 'INCS_%s' % configname, prefix='-I')

    compilable = filter(Compilable, sources)
    objs = map(self.Objectify, map(self.Absolutify, map(Target, compilable)))
    self.WriteList(objs, 'OBJS')

    self.WriteLn('# Add to the list of files we specially track '
                 'dependencies for.')
    self.WriteLn('all_deps += $(OBJS)')
    self._num_outputs += len(objs)
    self.WriteLn()

    # Make sure our dependencies are built first.
    if deps:
      self.WriteMakeRule(['$(OBJS)'], deps,
                         comment = 'Make sure our dependencies are built '
                                   'before any of us.',
                         order_only = True)

    # Make sure the actions and rules run first.
    # If they generate any extra headers etc., the per-.o file dep tracking
    # will catch the proper rebuilds, so order only is still ok here.
    if extra_outputs:
      self.WriteMakeRule(['$(OBJS)'], extra_outputs,
                         comment = 'Make sure our actions/rules run '
                                   'before any of us.',
                         order_only = True)

    if objs:
      extra_link_deps.append('$(OBJS)')
      self.WriteLn("""\
# CFLAGS et al overrides must be target-local.
# See "Target-specific Variable Values" in the GNU Make manual.""")
      self.WriteLn("$(OBJS): TOOLSET := $(TOOLSET)")
      self.WriteLn("$(OBJS): GYP_CFLAGS := "
                   "$(DEFS_$(BUILDTYPE)) "
                   "$(INCS_$(BUILDTYPE)) "
                   "$(CFLAGS_$(BUILDTYPE)) "
                   "$(CFLAGS_C_$(BUILDTYPE))")
      self.WriteLn("$(OBJS): GYP_CXXFLAGS := "
                   "$(DEFS_$(BUILDTYPE)) "
                   "$(INCS_$(BUILDTYPE)) "
                   "$(CFLAGS_$(BUILDTYPE)) "
                   "$(CFLAGS_CC_$(BUILDTYPE))")

    # If there are any object files in our input file list, link them into our
    # output.
    extra_link_deps += filter(Linkable, sources)

    self.WriteLn()


  def ComputeOutput(self, spec):
    """Return the 'output' (full output path) of a gyp spec.

    E.g., the loadable module 'foobar' in directory 'baz' will produce
      '$(obj)/baz/libfoobar.so'
    """
    output = None
    target = spec['target_name']
    target_prefix = ''
    target_ext = ''
    path = os.path.join('$(obj).' + self.toolset, self.path)
    if self.type == 'static_library':
      if target[:3] == 'lib':
        target = target[3:]
      target_prefix = 'lib'
      target_ext = '.a'
    elif self.type in ('loadable_module', 'shared_library'):
      if target[:3] == 'lib':
        target = target[3:]
      target_prefix = 'lib'
      target_ext = '.so'
    elif self.type == 'none':
      target = '%s.stamp' % target
    elif self.type == 'settings':
      return None
    elif self.type == 'executable':
      path = os.path.join('$(builddir)')
    else:
      print ("ERROR: What output file should be generated?",
             "typ", self.type, "target", target)

    path = spec.get('product_dir', path)
    target_prefix = spec.get('product_prefix', target_prefix)
    target = spec.get('product_name', target)
    product_ext = spec.get('product_extension')
    if product_ext:
      target_ext = '.' + product_ext

    return os.path.join(path, target_prefix + target + target_ext)


  def ComputeDeps(self, spec):
    """Compute the dependencies of a gyp spec.

    Returns a tuple (deps, link_deps), where each is a list of
    filenames that will need to be put in front of make for either
    building (deps) or linking (link_deps).
    """
    deps = []
    link_deps = []
    if 'dependencies' in spec:
      deps.extend([target_outputs[dep] for dep in spec['dependencies']
                   if target_outputs[dep]])
      for dep in spec['dependencies']:
        if dep in target_link_deps:
          link_deps.extend(target_link_deps[dep])
      deps.extend(link_deps)
      # TODO: It seems we need to transitively link in libraries (e.g. -lfoo)?
      # This hack makes it work:
      # link_deps.extend(spec.get('libraries', []))
    return (gyp.common.uniquer(deps), gyp.common.uniquer(link_deps))


  def WriteTarget(self, spec, configs, deps, link_deps, extra_outputs,
                  part_of_all):
    """Write Makefile code to produce the final target of the gyp spec.

    spec, configs: input from gyp.
    deps, link_deps: dependency lists; see ComputeDeps()
    extra_outputs: any extra outputs that our target should depend on
    part_of_all: flag indicating this target is part of 'all'
    """

    self.WriteLn('### Rules for final target.')

    if extra_outputs:
      self.WriteMakeRule([self.output], extra_outputs,
                         comment = 'Build our special outputs first.',
                         order_only = True)
      self.WriteMakeRule(extra_outputs, deps,
                         comment=('Preserve order dependency of '
                                  'special output on deps.'),
                         order_only = True,
                         multiple_output_trick = False)

    if self.type not in ('settings', 'none'):
      for configname in sorted(configs.keys()):
        config = configs[configname]
        self.WriteList(config.get('ldflags'), 'LDFLAGS_%s' % configname)
      libraries = spec.get('libraries')
      if libraries:
        # Remove duplicate entries
        libraries = gyp.common.uniquer(libraries)
      self.WriteList(libraries, 'LIBS')
      self.WriteLn('%s: GYP_LDFLAGS := $(LDFLAGS_$(BUILDTYPE))' % self.output)
      self.WriteLn('%s: LIBS := $(LIBS)' % self.output)

    if self.type == 'executable':
      self.WriteDoCmd([self.output], link_deps, 'link', part_of_all)
    elif self.type == 'static_library':
      self.WriteDoCmd([self.output], link_deps, 'alink', part_of_all)
    elif self.type in ('loadable_module', 'shared_library'):
      self.WriteDoCmd([self.output], link_deps, 'solink', part_of_all)
    elif self.type == 'none':
      # Write a stamp line.
      self.WriteDoCmd([self.output], deps, 'touch', part_of_all)
    elif self.type == 'settings':
      # Only used for passing flags around.
      pass
    else:
      print "WARNING: no output for", self.type, target

    # Add an alias for each target (if there are any outputs).
    # Installable target aliases are created below.
    if ((self.output and self.output != self.target) and
        (self.type not in self._INSTALLABLE_TARGETS)):
      self.WriteMakeRule([self.target], [self.output],
                         comment='Add target alias', phony = True)
      if part_of_all:
        self.WriteMakeRule(['all'], [self.target],
                           comment = 'Add target alias to "all" target.',
                           phony = True)

    # Add special-case rules for our installable targets.
    # 1) They need to install to the build dir or "product" dir.
    # 2) They get shortcuts for building (e.g. "make chrome").
    # 3) They are part of "make all".
    if self.type in self._INSTALLABLE_TARGETS:
      if self.type == 'shared_library':
        file_desc = 'shared library'
      else:
        file_desc = 'executable'
      install_path = self._InstallableTargetInstallPath()
      installable_deps = [self.output]
      # Point the target alias to the final binary output.
      self.WriteMakeRule([self.target], [install_path],
                         comment='Add target alias', phony = True)
      if install_path != self.output:
        self.WriteDoCmd([install_path], [self.output], 'copy',
                        comment = 'Copy this to the %s output path.' %
                        file_desc, part_of_all=part_of_all)
        installable_deps.append(install_path)
      if self.output != self.alias and self.alias != self.target:
        self.WriteMakeRule([self.alias], installable_deps,
                           comment = 'Short alias for building this %s.' %
                           file_desc, phony = True)
      if part_of_all:
        self.WriteMakeRule(['all'], [install_path],
                           comment = 'Add %s to "all" target.' % file_desc,
                           phony = True)


  def WriteList(self, list, variable=None, prefix='', quoter=QuoteIfNecessary):
    """Write a variable definition that is a list of values.

    E.g. WriteList(['a','b'], 'foo', prefix='blah') writes out
         foo = blaha blahb
    but in a pretty-printed style.
    """
    self.fp.write(variable + " := ")
    if list:
      list = [quoter(prefix + l) for l in list]
      self.fp.write(" \\\n\t".join(list))
    self.fp.write("\n\n")


  def WriteDoCmd(self, outputs, inputs, command, part_of_all, comment=None):
    """Write a Makefile rule that uses do_cmd.

    This makes the outputs dependent on the command line that was run,
    as well as support the V= make command line flag.
    """
    self.WriteMakeRule(outputs, inputs,
                       actions = ['$(call do_cmd,%s)' % command],
                       comment = comment,
                       force = True)
    # Add our outputs to the list of targets we read depfiles from.
    self.WriteLn('all_deps += %s' % ' '.join(outputs))
    self._num_outputs += len(outputs)


  def WriteMakeRule(self, outputs, inputs, actions=None, comment=None,
                    order_only=False, force=False, phony=False,
                    multiple_output_trick=True):
    """Write a Makefile rule, with some extra tricks.

    outputs: a list of outputs for the rule (note: this is not directly
             supported by make; see comments below)
    inputs: a list of inputs for the rule
    actions: a list of shell commands to run for the rule
    comment: a comment to put in the Makefile above the rule (also useful
             for making this Python script's code self-documenting)
    order_only: if true, makes the dependency order-only
    force: if true, include FORCE_DO_CMD as an order-only dep
    phony: if true, the rule does not actually generate the named output, the
           output is just a name to run the rule
    multiple_output_trick: if true (the default), perform tricks such as dummy
           rules to avoid problems with multiple outputs.
    """
    if comment:
      self.WriteLn('# ' + comment)
    if phony:
      self.WriteLn('.PHONY: ' + ' '.join(outputs))
    # TODO(evanm): just make order_only a list of deps instead of these hacks.
    if order_only:
      order_insert = '| '
    else:
      order_insert = ''
    if force:
      force_append = ' FORCE_DO_CMD'
    else:
      force_append = ''
    if actions:
      self.WriteLn("%s: TOOLSET := $(TOOLSET)" % outputs[0])
    self.WriteLn('%s: %s%s%s' % (outputs[0], order_insert, ' '.join(inputs),
                                 force_append))
    if actions:
      for action in actions:
        self.WriteLn('\t%s' % action)
    if multiple_output_trick and len(outputs) > 1:
      # If we have more than one output, a rule like
      #   foo bar: baz
      # that for *each* output we must run the action, potentially
      # in parallel.  That is not what we're trying to write -- what
      # we want is that we run the action once and it generates all
      # the files.
      # http://www.gnu.org/software/hello/manual/automake/Multiple-Outputs.html
      # discusses this problem and has this solution:
      # 1) Write the naive rule that would produce parallel runs of
      # the action.
      # 2) Make the outputs seralized on each other, so we won't start
      # a parallel run until the first run finishes, at which point
      # we'll have generated all the outputs and we're done.
      self.WriteLn('%s: %s' % (' '.join(outputs[1:]), outputs[0]))
      # Add a dummy command to the "extra outputs" rule, otherwise make seems to
      # think these outputs haven't (couldn't have?) changed, and thus doesn't
      # flag them as changed (i.e. include in '$?') when evaluating dependent
      # rules, which in turn causes do_cmd() to skip running dependent commands.
      self.WriteLn('%s: ;' % (' '.join(outputs[1:])))
    self.WriteLn()


  def WriteLn(self, text=''):
    self.fp.write(text + '\n')


  def Objectify(self, path):
    """Convert a path to its output directory form."""
    if '$(' in path:
      path = path.replace('$(obj)/', '$(obj).%s/$(TARGET)/' % self.toolset)
      return path
    return '$(obj).%s/$(TARGET)/%s' % (self.toolset, path)

  def Absolutify(self, path):
    """Convert a subdirectory-relative path into a base-relative path.
    Skips over paths that contain variables."""
    if '$(' in path:
      return path
    return os.path.normpath(os.path.join(self.path, path))


  def FixupArgPath(self, arg):
    if '/' in arg or '.h.' in arg:
      return self.Absolutify(arg)
    return arg


  def ExpandInputRoot(self, template, expansion):
    if '%(INPUT_ROOT)s' not in template:
      return template
    path = template % { 'INPUT_ROOT': expansion }
    if not os.path.dirname(path):
      # If it's just the file name, turn it into a path so FixupArgPath()
      # will know to Absolutify() it.
      path = os.path.join('.', path)
    return path


  def _InstallableTargetInstallPath(self):
    """Returns the location of the final output for an installable target."""
    if self.type == 'shared_library':
      # Install all shared libs into a common directory (per toolset) for
      # convenient access with LD_LIBRARY_PATH.
      return '$(builddir)/lib.%s/%s' % (self.toolset, self.alias)
    return '$(builddir)/' + self.alias


def WriteAutoRegenerationRule(params, root_makefile, makefile_name,
                              build_files):
  """Write the target to regenerate the Makefile."""
  options = params['options']
  build_files_args = [gyp.common.RelativePath(filename, options.toplevel_dir)
                      for filename in params['build_files_arg']]
  gyp_binary = gyp.common.FixIfRelativePath(params['gyp_binary'],
                                            options.toplevel_dir)
  if not gyp_binary.startswith(os.sep):
    gyp_binary = os.path.join('.', gyp_binary)
  root_makefile.write(
      "quiet_cmd_regen_makefile = ACTION Regenerating $@\n"
      "cmd_regen_makefile = %(cmd)s\n"
      "%(makefile_name)s: %(deps)s\n"
      "\t$(call do_cmd,regen_makefile)\n\n" % {
          'makefile_name': makefile_name,
          'deps': ' '.join(map(Sourceify, build_files)),
          'cmd': gyp.common.EncodePOSIXShellList(
                     [gyp_binary, '-fmake'] +
                     gyp.RegenerateFlags(options) +
                     build_files_args)})


def RunSystemTests():
  """Run tests against the system to compute default settings for commands.

  Returns:
    dictionary of settings matching the block of command-lines used in
    SHARED_HEADER.  E.g. the dictionary will contain a ARFLAGS.target
    key for the default ARFLAGS for the target ar command.
  """
  # Compute flags used for building static archives.
  # N.B.: this fallback logic should match the logic in SHARED_HEADER.
  # See comment there for more details.
  ar_target = os.environ.get('AR.target', os.environ.get('AR', 'ar'))
  cc_target = os.environ.get('CC.target', os.environ.get('CC', 'cc'))
  arflags_target = 'crs'
  if gyp.system_test.TestArSupportsT(ar_command=ar_target,
                                     cc_command=cc_target):
    arflags_target = 'crsT'

  ar_host = os.environ.get('AR.host', 'ar')
  cc_host = os.environ.get('CC.host', 'gcc')
  arflags_host = 'crs'
  # It feels redundant to compute this again given that most builds aren't
  # cross-compiles, but due to quirks of history CC.host defaults to 'gcc'
  # while CC.target defaults to 'cc', so the commands really are different
  # even though they're nearly guaranteed to run the same code underneath.
  if gyp.system_test.TestArSupportsT(ar_command=ar_host, cc_command=cc_host):
    arflags_host = 'crsT'

  link_flags = ''
  if gyp.system_test.TestLinkerSupportsThreads(cc_command=cc_target):
    # N.B. we don't test for cross-compilation; as currently written, we
    # don't even use flock when linking in the cross-compile setup!
    # TODO(evan): refactor cross-compilation such that this code can
    # be reused.
    link_flags = '-Wl,--threads --Wl,--thread-count=4'

  # TODO(evan): cache this output.  (But then we'll need to add extra
  # flags to gyp to flush the cache, yuk!  It's fast enough for now to
  # just run it every time.)

  return { 'ARFLAGS.target': arflags_target,
           'ARFLAGS.host': arflags_host,
           'LINK_flags': link_flags }


def GenerateOutput(target_list, target_dicts, data, params):
  options = params['options']
  generator_flags = params.get('generator_flags', {})
  builddir_name = generator_flags.get('output_dir', 'out')

  def CalculateMakefilePath(build_file, base_name):
    """Determine where to write a Makefile for a given gyp file."""
    # Paths in gyp files are relative to the .gyp file, but we want
    # paths relative to the source root for the master makefile.  Grab
    # the path of the .gyp file as the base to relativize against.
    # E.g. "foo/bar" when we're constructing targets for "foo/bar/baz.gyp".
    base_path = gyp.common.RelativePath(os.path.dirname(build_file),
                                        options.depth)
    # We write the file in the base_path directory.
    output_file = os.path.join(options.depth, base_path, base_name)
    if options.generator_output:
      output_file = os.path.join(options.generator_output, output_file)
    base_path = gyp.common.RelativePath(os.path.dirname(build_file),
                                        options.toplevel_dir)
    return base_path, output_file

  # TODO:  search for the first non-'Default' target.  This can go
  # away when we add verification that all targets have the
  # necessary configurations.
  default_configuration = None
  toolsets = set([target_dicts[target]['toolset'] for target in target_list])
  for target in target_list:
    spec = target_dicts[target]
    if spec['default_configuration'] != 'Default':
      default_configuration = spec['default_configuration']
      break
  if not default_configuration:
    default_configuration = 'Default'

  srcdir = '.'
  makefile_name = 'Makefile' + options.suffix
  makefile_path = os.path.join(options.toplevel_dir, makefile_name)
  if options.generator_output:
    global srcdir_prefix
    makefile_path = os.path.join(options.generator_output, makefile_path)
    srcdir = gyp.common.RelativePath(srcdir, options.generator_output)
    srcdir_prefix = '$(srcdir)/'

  header_params = {
      'srcdir': srcdir,
      'builddir': builddir_name,
      'default_configuration': default_configuration,
    }
  header_params.update(RunSystemTests())

  ensure_directory_exists(makefile_path)
  root_makefile = open(makefile_path, 'w')
  root_makefile.write(SHARED_HEADER % header_params)
  for toolset in toolsets:
    root_makefile.write('TOOLSET := %s\n' % toolset)
    root_makefile.write(ROOT_HEADER_SUFFIX_RULES)

  # Find the list of targets that derive from the gyp file(s) being built.
  needed_targets = set()
  for build_file in params['build_files']:
    for target in gyp.common.AllTargets(target_list, target_dicts, build_file):
      needed_targets.add(target)

  num_outputs = 0
  build_files = set()
  include_list = set()
  for qualified_target in target_list:
    build_file, target, toolset = gyp.common.ParseQualifiedTarget(
        qualified_target)
    build_files.add(gyp.common.RelativePath(build_file, options.toplevel_dir))
    included_files = data[build_file]['included_files']
    for included_file in included_files:
      # The included_files entries are relative to the dir of the build file
      # that included them, so we have to undo that and then make them relative
      # to the root dir.
      relative_include_file = gyp.common.RelativePath(
          gyp.common.UnrelativePath(included_file, build_file),
          options.toplevel_dir)
      abs_include_file = os.path.abspath(relative_include_file)
      # If the include file is from the ~/.gyp dir, we should use absolute path
      # so that relocating the src dir doesn't break the path.
      if (params['home_dot_gyp'] and
          abs_include_file.startswith(params['home_dot_gyp'])):
        build_files.add(abs_include_file)
      else:
        build_files.add(relative_include_file)

    base_path, output_file = CalculateMakefilePath(build_file,
        target + '.' + toolset + options.suffix + '.mk')

    spec = target_dicts[qualified_target]
    configs = spec['configurations']

    writer = MakefileWriter()
    writer.Write(qualified_target, base_path, output_file, spec, configs,
                 part_of_all=qualified_target in needed_targets)
    num_outputs += writer.NumOutputs()

    # Our root_makefile lives at the source root.  Compute the relative path
    # from there to the output_file for including.
    mkfile_rel_path = gyp.common.RelativePath(output_file,
                                              os.path.dirname(makefile_path))
    include_list.add(mkfile_rel_path)

  # Write out per-gyp (sub-project) Makefiles.
  depth_rel_path = gyp.common.RelativePath(options.depth, os.getcwd())
  for build_file in build_files:
    # The paths in build_files were relativized above, so undo that before
    # testing against the non-relativized items in target_list and before
    # calculating the Makefile path.
    build_file = os.path.join(depth_rel_path, build_file)
    gyp_targets = [target_dicts[target]['target_name'] for target in target_list
                   if target.startswith(build_file) and
                   target in needed_targets]
    # Only generate Makefiles for gyp files with targets.
    if not gyp_targets:
      continue
    base_path, output_file = CalculateMakefilePath(build_file,
        os.path.splitext(os.path.basename(build_file))[0] + '.Makefile')
    makefile_rel_path = gyp.common.RelativePath(os.path.dirname(makefile_path),
                                                os.path.dirname(output_file))
    writer.WriteSubMake(output_file, makefile_rel_path, gyp_targets,
                        builddir_name)


  # Write out the sorted list of includes.
  root_makefile.write('\n')
  for include_file in sorted(include_list):
    # We wrap each .mk include in an if statement so users can tell make to
    # not load a file by setting NO_LOAD.  The below make code says, only
    # load the .mk file if the .mk filename doesn't start with a token in
    # NO_LOAD.
    root_makefile.write(
        "ifeq ($(strip $(foreach prefix,$(NO_LOAD),\\\n"
        "    $(findstring $(join ^,$(prefix)),\\\n"
        "                 $(join ^," + include_file + ")))),)\n")
    root_makefile.write("  include " + include_file + "\n")
    root_makefile.write("endif\n")
  root_makefile.write('\n')

  if generator_flags.get('auto_regeneration', True):
    WriteAutoRegenerationRule(params, root_makefile, makefile_name, build_files)

  # Write the rule to load dependencies.  We batch 1000 files at a time to
  # avoid overflowing the command line.
  all_deps = ""
  for i in range(1001, num_outputs, 1000):
    all_deps += ("""
  ifneq ($(word %(start)d,$(d_files)),)
    $(shell cat $(wordlist %(start)d,%(end)d,$(d_files)) >> $(depsdir)/all.deps)
  endif""" % { 'start': i, 'end': i + 999 })

  # Add a check to make sure we tried to process all the .d files.
  all_deps += """
  ifneq ($(word %(last)d,$(d_files)),)
    $(error Found unprocessed dependency files (gyp didn't generate enough rules!))
  endif
""" % { 'last': ((num_outputs / 1000) + 1) * 1000 + 1 }

  root_makefile.write(SHARED_FOOTER % { 'generate_all_deps': all_deps })

  root_makefile.close()
