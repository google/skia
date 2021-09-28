#!/bin/bash

FT_GIT_REPO=https://chromium.googlesource.com/chromium/src/third_party/freetype2.git
FT_GIT_REF=origin/master
FT_GIT_DIR=third_party/externals/freetype
FT_BUILD_DIR=$(dirname -- "$0")

previousrev() {
  STEP="original revision" &&
  FT_PREVIOUS_REV=$(git grep "${FT_GIT_REPO}" HEAD~1 -- DEPS | sed 's!.*'${FT_GIT_REPO}'@\([[:xdigit:]]\{40\}\).*!\1!')
}

nextrev() {
  STEP="next revision" &&
  git -C ${FT_GIT_DIR} fetch &&
  FT_NEXT_REV=$(git -C ${FT_GIT_DIR} rev-parse ${FT_GIT_REF})
}

rolldeps() {
  STEP="roll-deps" &&
  sed -i'' -e "s!${FT_GIT_REPO}@${FT_PREVIOUS_REV}!${FT_GIT_REPO}@${FT_NEXT_REV}!" DEPS &&
  tools/git-sync-deps &&
  git add DEPS
}

mergeinclude() {
  STEP="merge ${SKIA_INCLUDE}: check for merge conflicts" &&
  SKIA_INCLUDE=include/$1/$2 &&
  FT_INCLUDE=include/freetype/config/$2 &&
  TMPFILE=$(mktemp) &&
  git -C ${FT_GIT_DIR} cat-file blob ${FT_PREVIOUS_REV}:${FT_INCLUDE} >> ${TMPFILE} &&
  git merge-file ${FT_BUILD_DIR}/${SKIA_INCLUDE} ${TMPFILE} ${FT_GIT_DIR}/${FT_INCLUDE} &&
  rm ${TMPFILE} &&
  git add ${FT_BUILD_DIR}/${SKIA_INCLUDE}
}

commit() {
  STEP="commit" &&
  FT_PREVIOUS_REV_SHORT=$(expr substr "${FT_PREVIOUS_REV}" 1 8) &&
  FT_NEXT_REV_SHORT=$(expr substr "${FT_NEXT_REV}" 1 8) &&
  FT_COMMIT_COUNT=$(git -C ${FT_GIT_DIR} rev-list --count ${FT_PREVIOUS_REV}..${FT_NEXT_REV}) &&
  git commit -m"Roll FreeType from ${FT_PREVIOUS_REV_SHORT} to ${FT_NEXT_REV_SHORT} (${FT_COMMIT_COUNT} commits)

${FT_GIT_REPO}/+log/${FT_PREVIOUS_REV}..${FT_NEXT_REV}

Disable: treat-URL-as-trailer"
}

previousrev &&
nextrev &&
rolldeps "$@" &&
mergeinclude freetype-android ftoption.h &&
mergeinclude freetype-android ftmodule.h &&
mergeinclude freetype-no-type1 ftoption.h &&
mergeinclude freetype-no-type1 ftmodule.h &&
commit &&
true || { echo "Failed step ${STEP}"; exit 1; }
