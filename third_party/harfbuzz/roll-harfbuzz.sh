#!/bin/bash

HB_GIT_REPO=https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git
HB_GIT_REF=origin/upstream/main
HB_GIT_DIR=third_party/externals/harfbuzz
HB_BUILD_DIR=$(dirname -- "$0")

previousrev() {
  STEP="original revision" &&
  HB_PREVIOUS_REV=$(git grep "${HB_GIT_REPO}" HEAD~1 -- DEPS | sed 's!.*'${HB_GIT_REPO}'@\([[:xdigit:]]\{40\}\).*!\1!')
}

nextrev() {
  STEP="next revision" &&
  git -C ${HB_GIT_DIR} fetch &&
  HB_NEXT_REV=$(git -C ${HB_GIT_DIR} rev-parse ${HB_GIT_REF})
}

rolldeps() {
  STEP="roll-deps" &&
  sed -i'' -e "s!${HB_GIT_REPO}@${HB_PREVIOUS_REV}!${HB_GIT_REPO}@${HB_NEXT_REV}!" DEPS &&
  tools/git-sync-deps &&
  git add DEPS
}

rollbazel() {
  STEP="roll-bazel" &&
  sed -i'' -e "s!commit = \"${HB_PREVIOUS_REV}\",!commit = \"${HB_NEXT_REV}\",!" bazel/deps.bzl &&
  git add bazel/deps.bzl
}

check_all_files_are_categorized() {
  #for each file name in ${HB_GIT_DIR}/src/hb-*.{cc,h,hh}
  #  if the file name is not present in BUILD.gn
  #    should be added to BUILD.gn (in 'unused_sources' if unwanted)

  #for each file name \"src/.*\" in BUILD.gn
  #  if the file name does not exist
  #    should be removed from BUILD.gn

  STEP="Updating BUILD.gn" &&
  HB_BUILD_DIR_REL=$(realpath --relative-to=${HB_GIT_DIR} ${HB_BUILD_DIR})
  ( # Create subshell for IFS, CDPATH, and cd.
    # This implementation doesn't handle '"' or '\n' in file names.
    IFS=$'\n' &&
    CDPATH= &&
    cd -- "${HB_GIT_DIR}" &&

    HB_SOURCE_MISSING=false &&
    find src -type f \( -name "*.cc" -o -name "*.h" -o -name "*.hh" \) | while read HB_SOURCE
    do
      if ! grep -qF "$HB_SOURCE" ${HB_BUILD_DIR_REL}/BUILD.gn; then
        if ! ${HB_SOURCE_MISSING}; then
          echo "Is in src/*.{cc,h,hh} but not in BUILD.gn:"
          HB_SOURCE_MISSING=true
        fi
        echo "      \"\$_${HB_SOURCE}\","
      fi
    done &&

    GN_SOURCE_MISSING=false &&
    grep -oE "\"\\\$_src/[^\"]+\"" ${HB_BUILD_DIR_REL}/BUILD.gn | sed 's/^...\(.*\).$/\1/' | while read GN_SOURCE
    do
      if [ ! -f "${GN_SOURCE}" ]; then
        if ! ${GN_SOURCE_MISSING}; then
          echo "Is referenced in BUILD.gn but does not exist:" &&
          GN_SOURCE_MISSING=true
        fi
        echo "\"${GN_SOURCE}\""
      fi
    done &&

    GN_SOURCE_DUPLICATES=$(sort ${HB_BUILD_DIR_REL}/BUILD.gn | uniq -d | grep -oE "\"\\\$_src/[^\"]+\"")
    if [ -n "${GN_SOURCE_DUPLICATES}" ]; then
      echo "Is listed more than once in BUILD.gn:" &&
      echo ${GN_SOURCE_DUPLICATES}
    fi
  )
}

commit() {
  STEP="commit" &&
  HB_PREVIOUS_REV_SHORT=$(expr substr "${HB_PREVIOUS_REV}" 1 8) &&
  HB_NEXT_REV_SHORT=$(expr substr "${HB_NEXT_REV}" 1 8) &&
  HB_COMMIT_COUNT=$(git -C ${HB_GIT_DIR} rev-list --count ${HB_PREVIOUS_REV}..${HB_NEXT_REV}) &&
  git commit -m"Roll HarfBuzz from ${HB_PREVIOUS_REV_SHORT} to ${HB_NEXT_REV_SHORT} (${HB_COMMIT_COUNT} commits)

${HB_GIT_REPO}/+log/${HB_PREVIOUS_REV}..${HB_NEXT_REV}

Disable: treat-URL-as-trailer"
}

previousrev &&
nextrev &&
rolldeps "$@" &&
rollbazel &&
check_all_files_are_categorized &&
commit &&
true || { echo "Failed step ${STEP}"; exit 1; }
