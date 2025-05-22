#!/bin/bash

FT_GIT_REPO="https://chromium.googlesource.com/chromium/src/third_party/freetype2.git"
FT_GIT_REF="origin/master"
FT_GIT_DIR="third_party/externals/freetype"
FT_BUILD_DIR="$(dirname -- "$0")"

notshallow() {
  STEP="check ${FT_GIT_DIR} not shallow"
  if $(git -C "${FT_GIT_DIR}" rev-parse --is-shallow-repository); then
    return 1
  fi
}

previousrev() {
  STEP="original revision" &&
  FT_PREVIOUS_REV=$(git grep "${FT_GIT_REPO}" HEAD~1 -- DEPS | sed 's!.*'${FT_GIT_REPO}'@\([[:xdigit:]]\{40\}\).*!\1!')
}

nextrev() {
  STEP="next revision" &&
  git -C "${FT_GIT_DIR}" fetch &&
  FT_NEXT_REV=$1 &&
  : ${FT_NEXT_REV:=$(git -C "${FT_GIT_DIR}" rev-parse "${FT_GIT_REF}")}
}

rolldeps() {
  STEP="roll-deps" &&
  sed -i'' -e "s!${FT_GIT_REPO}@${FT_PREVIOUS_REV}!${FT_GIT_REPO}@${FT_NEXT_REV}!" DEPS &&
  tools/git-sync-deps &&
  git add DEPS
}

rollbazel() {
  STEP="roll-bazel" &&
  sed -i'' -e "s!\"${FT_PREVIOUS_REV}\",!\"${FT_NEXT_REV}\",!" bazel/deps.json &&
  git add bazel/deps.json
}

rolldepsgen() {
  STEP="roll-depsgen" &&
  sed -i'' -e "s!Version: \"${FT_PREVIOUS_REV}\",!Version: \"${FT_NEXT_REV}\",!" infra/bots/deps/deps_gen.go &&
  git add infra/bots/deps/deps_gen.go
}

mergeinclude() {
  SKIA_INCLUDE="include/$1/$2" &&
  STEP="merge ${SKIA_INCLUDE}: check for merge conflicts" &&
  FT_INCLUDE="include/$2" &&
  TMPFILE="$(mktemp)" &&
  git -C "${FT_GIT_DIR}" cat-file blob "${FT_PREVIOUS_REV}:${FT_INCLUDE}" >> "${TMPFILE}" &&
  git merge-file "${FT_BUILD_DIR}/${SKIA_INCLUDE}" "${TMPFILE}" "${FT_GIT_DIR}/${FT_INCLUDE}" &&
  rm "${TMPFILE}" &&
  git add "${FT_BUILD_DIR}/${SKIA_INCLUDE}"
}

update_bazel_patch() {
  STEP="Update Bazel patch" &&
  python3 tools/generate_patches.py \
    ${FT_BUILD_DIR}/include/freetype-android/freetype/config/ftmodule.h builds/android-ftmodule.h \
    ${FT_BUILD_DIR}/include/freetype-android/freetype/config/ftoption.h builds/android-ftoption.h \
    ${FT_BUILD_DIR}/include/freetype-no-type1/freetype/config/ftmodule.h builds/no-type1-ftmodule.h \
    ${FT_BUILD_DIR}/include/freetype-no-type1/freetype/config/ftoption.h builds/no-type1-ftoption.h \
    > bazel/external/freetype/config_files.patch &&
  git add bazel/external/freetype/config_files.patch
}

commit() {
  STEP="commit" &&
  FT_PREVIOUS_REV_SHORT=$(echo "${FT_PREVIOUS_REV}" | cut -c 1-8) &&
  FT_NEXT_REV_SHORT=$(echo "${FT_NEXT_REV}" | cut -c 1-8) &&
  FT_COMMIT_COUNT=$(git -C "${FT_GIT_DIR}" rev-list --count "${FT_PREVIOUS_REV}..${FT_NEXT_REV}") &&
  git commit -m"Roll FreeType from ${FT_PREVIOUS_REV_SHORT} to ${FT_NEXT_REV_SHORT} (${FT_COMMIT_COUNT} commits)

${FT_GIT_REPO}/+log/${FT_PREVIOUS_REV}..${FT_NEXT_REV}

Disable: treat-URL-as-trailer"
}

notshallow &&
previousrev &&
nextrev "$1" &&
rolldeps &&
rollbazel &&
rolldepsgen &&
mergeinclude freetype-android freetype/config/ftoption.h &&
mergeinclude freetype-android freetype/config/ftmodule.h &&
mergeinclude freetype-no-type1 freetype/config/ftoption.h &&
mergeinclude freetype-no-type1 freetype/config/ftmodule.h &&
update_bazel_patch &&
commit &&
true || { echo "Failed step ${STEP}"; exit 1; }
