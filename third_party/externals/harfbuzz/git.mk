# git.mk
#
# Copyright 2009, Red Hat, Inc.
# Copyright 2010,2011,2012,2013 Behdad Esfahbod
# Written by Behdad Esfahbod
#
# Copying and distribution of this file, with or without modification,
# is permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#
# The latest version of this file can be downloaded from:
#   https://raw.github.com/behdad/git.mk/master/git.mk
# Bugs, etc, should be reported upstream at:
#   https://github.com/behdad/git.mk
#
# To use in your project, import this file in your git repo's toplevel,
# then do "make -f git.mk".  This modifies all Makefile.am files in
# your project to -include git.mk.  Remember to add that line to new
# Makefile.am files you create in your project, or just rerun the
# "make -f git.mk".
#
# This enables automatic .gitignore generation.  If you need to ignore
# more files, add them to the GITIGNOREFILES variable in your Makefile.am.
# But think twice before doing that.  If a file has to be in .gitignore,
# chances are very high that it's a generated file and should be in one
# of MOSTLYCLEANFILES, CLEANFILES, DISTCLEANFILES, or MAINTAINERCLEANFILES.
#
# The only case that you need to manually add a file to GITIGNOREFILES is
# when remove files in one of mostlyclean-local, clean-local, distclean-local,
# or maintainer-clean-local make targets.
#
# Note that for files like editor backup, etc, there are better places to
# ignore them.  See "man gitignore".
#
# If "make maintainer-clean" removes the files but they are not recognized
# by this script (that is, if "git status" shows untracked files still), send
# me the output of "git status" as well as your Makefile.am and Makefile for
# the directories involved and I'll diagnose.
#
# For a list of toplevel files that should be in MAINTAINERCLEANFILES, see
# Makefile.am.sample in the git.mk git repo.
#
# Don't EXTRA_DIST this file.  It is supposed to only live in git clones,
# not tarballs.  It serves no useful purpose in tarballs and clutters the
# build dir.
#
# This file knows how to handle autoconf, automake, libtool, gtk-doc,
# gnome-doc-utils, yelp.m4, mallard, intltool, gsettings, dejagnu.
#
# This makefile provides the following targets:
#
# - all: "make all" will build all gitignore files.
# - gitignore: makes all gitignore files in the current dir and subdirs.
# - .gitignore: make gitignore file for the current dir.
# - gitignore-recurse: makes all gitignore files in the subdirs.
#
# KNOWN ISSUES:
#
# - Recursive configure doesn't work as $(top_srcdir)/git.mk inside the
#   submodule doesn't find us.  If you have configure.{in,ac} files in
#   subdirs, add a proxy git.mk file in those dirs that simply does:
#   "include $(top_srcdir)/../git.mk".  Add more ..'s to your taste.
#   And add those files to git.  See vte/gnome-pty-helper/git.mk for
#   example.
#



###############################################################################
# Variables user modules may want to add to toplevel MAINTAINERCLEANFILES:
###############################################################################

#
# Most autotools-using modules should be fine including this variable in their
# toplevel MAINTAINERCLEANFILES:
GITIGNORE_MAINTAINERCLEANFILES_TOPLEVEL = \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/autoscan.log \
	$(srcdir)/configure.scan \
	`AUX_DIR=$(srcdir)/$$(cd $(top_srcdir); $(AUTOCONF) --trace 'AC_CONFIG_AUX_DIR:$$1' ./configure.ac); \
	 test "x$$AUX_DIR" = "x$(srcdir)/" && AUX_DIR=$(srcdir); \
	 for x in \
		ar-lib \
		compile \
		config.guess \
		config.sub \
		depcomp \
		install-sh \
		ltmain.sh \
		missing \
		mkinstalldirs \
		test-driver \
	 ; do echo "$$AUX_DIR/$$x"; done` \
	`cd $(top_srcdir); $(AUTOCONF) --trace 'AC_CONFIG_HEADERS:$$1' ./configure.ac | \
	head -n 1 | while read f; do echo "$(srcdir)/$$f.in"; done`
#
# All modules should also be fine including the following variable, which
# removes automake-generated Makefile.in files:
GITIGNORE_MAINTAINERCLEANFILES_MAKEFILE_IN = \
	`cd $(top_srcdir); $(AUTOCONF) --trace 'AC_CONFIG_FILES:$$1' ./configure.ac | \
	while read f; do \
	  case $$f in Makefile|*/Makefile) \
	    test -f "$(srcdir)/$$f.am" && echo "$(srcdir)/$$f.in";; esac; \
	done`
#
# Modules that use libtool and use  AC_CONFIG_MACRO_DIR() may also include this,
# though it's harmless to include regardless.
GITIGNORE_MAINTAINERCLEANFILES_M4_LIBTOOL = \
	`MACRO_DIR=$(srcdir)/$$(cd $(top_srcdir); $(AUTOCONF) --trace 'AC_CONFIG_MACRO_DIR:$$1' ./configure.ac); \
	 if test "x$$MACRO_DIR" != "x$(srcdir)/"; then \
		for x in \
			libtool.m4 \
			ltoptions.m4 \
			ltsugar.m4 \
			ltversion.m4 \
			lt~obsolete.m4 \
		; do echo "$$MACRO_DIR/$$x"; done; \
	 fi`



###############################################################################
# Default rule is to install ourselves in all Makefile.am files:
###############################################################################

git-all: git-mk-install

git-mk-install:
	@echo "Installing git makefile"
	@any_failed=; \
		find "`test -z "$(top_srcdir)" && echo . || echo "$(top_srcdir)"`" -name Makefile.am | while read x; do \
		if grep 'include .*/git.mk' $$x >/dev/null; then \
			echo "$$x already includes git.mk"; \
		else \
			failed=; \
			echo "Updating $$x"; \
			{ cat $$x; \
			  echo ''; \
			  echo '-include $$(top_srcdir)/git.mk'; \
			} > $$x.tmp || failed=1; \
			if test x$$failed = x; then \
				mv $$x.tmp $$x || failed=1; \
			fi; \
			if test x$$failed = x; then : else \
				echo "Failed updating $$x"; >&2 \
				any_failed=1; \
			fi; \
	fi; done; test -z "$$any_failed"

.PHONY: git-all git-mk-install



###############################################################################
# Actual .gitignore generation:
###############################################################################

$(srcdir)/.gitignore: Makefile.am $(top_srcdir)/git.mk
	@echo "git.mk: Generating $@"
	@{ \
		if test "x$(DOC_MODULE)" = x -o "x$(DOC_MAIN_SGML_FILE)" = x; then :; else \
			for x in \
				$(DOC_MODULE)-decl-list.txt \
				$(DOC_MODULE)-decl.txt \
				tmpl/$(DOC_MODULE)-unused.sgml \
				"tmpl/*.bak" \
				xml html \
			; do echo "/$$x"; done; \
			FLAVOR=$$(cd $(top_srcdir); $(AUTOCONF) --trace 'GTK_DOC_CHECK:$$2' ./configure.ac); \
			case $$FLAVOR in *no-tmpl*) echo /tmpl;; esac; \
		fi; \
		if test "x$(DOC_MODULE)$(DOC_ID)" = x -o "x$(DOC_LINGUAS)" = x; then :; else \
			for lc in $(DOC_LINGUAS); do \
				for x in \
					$(if $(DOC_MODULE),$(DOC_MODULE).xml) \
					$(DOC_PAGES) \
					$(DOC_INCLUDES) \
				; do echo "/$$lc/$$x"; done; \
			done; \
			for x in \
				$(_DOC_OMF_ALL) \
				$(_DOC_DSK_ALL) \
				$(_DOC_HTML_ALL) \
				$(_DOC_MOFILES) \
				$(DOC_H_FILE) \
				"*/.xml2po.mo" \
				"*/*.omf.out" \
			; do echo /$$x; done; \
		fi; \
		if test "x$(HELP_ID)" = x -o "x$(HELP_LINGUAS)" = x; then :; else \
			for lc in $(HELP_LINGUAS); do \
				for x in \
					$(HELP_FILES) \
					"$$lc.stamp" \
					"$$lc.mo" \
				; do echo "/$$lc/$$x"; done; \
			done; \
		fi; \
		if test "x$(gsettings_SCHEMAS)" = x; then :; else \
			for x in \
				$(gsettings_SCHEMAS:.xml=.valid) \
				$(gsettings__enum_file) \
			; do echo "/$$x"; done; \
		fi; \
		if test -f $(srcdir)/po/Makefile.in.in; then \
			for x in \
				po/Makefile.in.in \
				po/Makefile.in.in~ \
				po/Makefile.in \
				po/Makefile \
				po/Makevars.template \
				po/POTFILES \
				po/Rules-quot \
				po/stamp-it \
				po/.intltool-merge-cache \
				"po/*.gmo" \
				"po/*.header" \
				"po/*.mo" \
				"po/*.sed" \
				"po/*.sin" \
				po/$(GETTEXT_PACKAGE).pot \
				intltool-extract.in \
				intltool-merge.in \
				intltool-update.in \
			; do echo "/$$x"; done; \
		fi; \
		if test -f $(srcdir)/configure; then \
			for x in \
				autom4te.cache \
				configure \
				config.h \
				stamp-h1 \
				libtool \
				config.lt \
			; do echo "/$$x"; done; \
		fi; \
		if test "x$(DEJATOOL)" = x; then :; else \
			for x in \
				$(DEJATOOL) \
			; do echo "/$$x.sum"; echo "/$$x.log"; done; \
			echo /site.exp; \
		fi; \
		if test "x$(am__dirstamp)" = x; then :; else \
			echo "$(am__dirstamp)"; \
		fi; \
		if test "x$(LTCOMPILE)" = x -a "x$(GTKDOC_RUN)" = x; then :; else \
			for x in \
				"*.lo" \
				".libs" "_libs" \
			; do echo "$$x"; done; \
		fi; \
		for x in \
			.gitignore \
			$(GITIGNOREFILES) \
			$(CLEANFILES) \
			$(PROGRAMS) $(check_PROGRAMS) $(EXTRA_PROGRAMS) \
			$(LIBRARIES) $(check_LIBRARIES) $(EXTRA_LIBRARIES) \
			$(LTLIBRARIES) $(check_LTLIBRARIES) $(EXTRA_LTLIBRARIES) \
			so_locations \
			$(MOSTLYCLEANFILES) \
			$(TEST_LOGS) \
			$(TEST_LOGS:.log=.trs) \
			$(TEST_SUITE_LOG) \
			"*.$(OBJEXT)" \
			$(DISTCLEANFILES) \
			$(am__CONFIG_DISTCLEAN_FILES) \
			$(CONFIG_CLEAN_FILES) \
			TAGS ID GTAGS GRTAGS GSYMS GPATH tags \
			"*.tab.c" \
			$(MAINTAINERCLEANFILES) \
			$(BUILT_SOURCES) \
			$(DEPDIR) \
			$(patsubst %.vala,%.c,$(filter %.vala,$(SOURCES))) \
			$(filter %_vala.stamp,$(DIST_COMMON)) \
			$(filter %.vapi,$(DIST_COMMON)) \
			$(filter %$(patsubst %.vapi,%.h,$(filter %.vapi,$(DIST_COMMON))),$(DIST_COMMON)) \
			Makefile \
			Makefile.in \
			"*.orig" \
			"*.rej" \
			"*.bak" \
			"*~" \
			".*.sw[nop]" \
			".dirstamp" \
		; do echo "/$$x"; done; \
	} | \
	sed "s@^/`echo "$(srcdir)" | sed 's/\(.\)/[\1]/g'`/@/@" | \
	sed 's@/[.]/@/@g' | \
	LC_ALL=C sort | uniq > $@.tmp && \
	mv $@.tmp $@;

all: $(srcdir)/.gitignore gitignore-recurse-maybe
gitignore: $(srcdir)/.gitignore gitignore-recurse

gitignore-recurse-maybe:
	@for subdir in $(DIST_SUBDIRS); do \
	  case " $(SUBDIRS) " in \
	    *" $$subdir "*) :;; \
	    *) test "$$subdir" = . -o -e "$$subdir/.git" || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) gitignore || echo "Skipping $$subdir");; \
	  esac; \
	done
gitignore-recurse:
	@for subdir in $(DIST_SUBDIRS); do \
	    test "$$subdir" = . -o -e "$$subdir/.git" || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) gitignore || echo "Skipping $$subdir"); \
	done

maintainer-clean: gitignore-clean
gitignore-clean:
	-rm -f $(srcdir)/.gitignore

.PHONY: gitignore-clean gitignore gitignore-recurse gitignore-recurse-maybe
