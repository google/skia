#!/bin/bash

EXPAT_GIT_REPO=https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git
EXPAT_GIT_REF=origin/upstream/master
EXPAT_GIT_DIR=third_party/externals/expat
EXPAT_BUILD_DIR=$(dirname -- "$0")

previousrev() {
  STEP="original revision" &&
  EXPAT_PREVIOUS_REV=$(git grep "${EXPAT_GIT_REPO}" HEAD~1 -- DEPS | sed 's!.*'${EXPAT_GIT_REPO}'@\([[:xdigit:]]\{40\}\).*!\1!')
}

nextrev() {
  STEP="next revision" &&
  git -C "${EXPAT_GIT_DIR}" fetch &&
  EXPAT_NEXT_REV=$(git -C "${EXPAT_GIT_DIR}" rev-parse ${EXPAT_GIT_REF})
}

rolldeps() {
  STEP="roll-deps" &&
  sed -i'' -e "s!${EXPAT_GIT_REPO}@${EXPAT_PREVIOUS_REV}!${EXPAT_GIT_REPO}@${EXPAT_NEXT_REV}!" DEPS &&
  tools/git-sync-deps &&
  git add DEPS
}

rollbazel() {
  STEP="roll-bazel" &&
  sed -i'' -e "s!\"${EXPAT_PREVIOUS_REV}\",!\"${EXPAT_NEXT_REV}\",!" bazel/deps.json &&
  git add bazel/deps.json
}

rolldepsgen() {
  STEP="roll-depsgen" &&
  sed -i'' -e "s!Version: \"${EXPAT_PREVIOUS_REV}\",!Version: \"${EXPAT_NEXT_REV}\",!" infra/bots/deps/deps_gen.go &&
  git add infra/bots/deps/deps_gen.go
}

check_all_files_are_categorized() {
  #for each file name in ${EXPAT_GIT_DIR}/expat/lib/*.{c,h}
  #  if the file name is not present in BUILD.gn
  #    should be added to BUILD.gn (in 'unused_sources' if unwanted)

  #for each file name \"expat/lib/.*\" in BUILD.gn
  #  if the file name does not exist
  #    should be removed from BUILD.gn

  STEP="Updating BUILD.gn" &&
  EXPAT_BUILD_DIR_REL=$(realpath --relative-to="${EXPAT_GIT_DIR}" "${EXPAT_BUILD_DIR}")
  ( # Create subshell for IFS, CDPATH, and cd.
    # This implementation doesn't handle '"' or '\n' in file names.
    IFS=$'\n' &&
    CDPATH= &&
    cd -- "${EXPAT_GIT_DIR}" &&

    EXPAT_SOURCE_MISSING=false &&
    find "expat/lib" -type f \( -name "*.c" -o -name "*.h" \) | while read EXPAT_SOURCE
    do
      if ! grep -qF "$EXPAT_SOURCE" "${EXPAT_BUILD_DIR_REL}/BUILD.gn"; then
        if ! ${EXPAT_SOURCE_MISSING}; then
          echo "Is in expat/lib/*.{c,h} but not in BUILD.gn:"
          EXPAT_SOURCE_MISSING=true
        fi
        echo "      \"\$_src/${EXPAT_SOURCE}\","
      fi
    done &&

    GN_SOURCE_MISSING=false &&
    grep -oE "\"\\\$_src/[^\"]+\"" "${EXPAT_BUILD_DIR_REL}/BUILD.gn" | sed 's@^"._src/\(.*\).$@\1@' | while read GN_SOURCE
    do
      if [ ! -f "${GN_SOURCE}" ]; then
        if ! ${GN_SOURCE_MISSING}; then
          echo "Is referenced in BUILD.gn but does not exist:" &&
          GN_SOURCE_MISSING=true
        fi
        echo "\"${GN_SOURCE}\""
      fi
    done &&

    GN_SOURCE_DUPLICATES=$(sort "${EXPAT_BUILD_DIR_REL}/BUILD.gn" | uniq -d | grep -oE "\"\\\$_src/[^\"]+\"")
    if [ ! -z ${GN_SOURCE_DUPLICATES} ]; then
      echo "Is listed more than once in BUILD.gn:" &&
      echo ${GN_SOURCE_DUPLICATES}
    fi
  )
}

update_expat_config_h() {
  STEP="update expat config.h" &&
  ( cd "${EXPAT_GIT_DIR}/expat" &&
    ./buildconf.sh &&
    ./configure) &&
  cp "${EXPAT_GIT_DIR}/expat/expat_config.h" "${EXPAT_BUILD_DIR}/include/expat_config/" &&
  patch -d "${EXPAT_BUILD_DIR}" -p3 < "${EXPAT_BUILD_DIR}/0001-Do-not-claim-getrandom.patch" &&
  patch -d "${EXPAT_BUILD_DIR}" -p3 < "${EXPAT_BUILD_DIR}/0002-Do-not-claim-arc4random_buf.patch" &&
  git add "${EXPAT_BUILD_DIR}/include/expat_config/expat_config.h"
}

update_bazel_patch() {
  STEP="Update Bazel patch" &&
  python3 tools/generate_patches.py \
    "${EXPAT_BUILD_DIR}/include/expat_config/expat_config.h" expat_config.h \
    > bazel/external/expat/config_files.patch &&
  git add bazel/external/expat/config_files.patch
}

commit() {
  STEP="commit" &&
  EXPAT_PREVIOUS_REV_SHORT=$(expr substr "${EXPAT_PREVIOUS_REV}" 1 8) &&
  EXPAT_NEXT_REV_SHORT=$(expr substr "${EXPAT_NEXT_REV}" 1 8) &&
  EXPAT_COMMIT_COUNT=$(git -C ${EXPAT_GIT_DIR} rev-list --count ${EXPAT_PREVIOUS_REV}..${EXPAT_NEXT_REV}) &&
  git commit -m"Roll Expat from ${EXPAT_PREVIOUS_REV_SHORT} to ${EXPAT_NEXT_REV_SHORT} (${EXPAT_COMMIT_COUNT} commits)

${EXPAT_GIT_REPO}/+log/${EXPAT_PREVIOUS_REV}..${EXPAT_NEXT_REV}

Disable: treat-URL-as-trailer"
}

previousrev &&
nextrev &&
rolldeps "$@" &&
rollbazel &&
rolldepsgen &&
update_expat_config_h &&
check_all_files_are_categorized &&
update_bazel_patch &&
commit &&
true || { echo "Failed step ${STEP}"; exit 1; }
