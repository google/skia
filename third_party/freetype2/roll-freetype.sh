#!/bin/bash

rolldeps() {
  STEP="roll-deps" &&
  roll-dep "$@" third_party/externals/freetype/
}

previousrev() {
  STEP="original revision" &&
  PREVIOUS_FREETYPE_REV=$(git grep "'freetype_revision':" HEAD~1 -- DEPS | grep -Eho "[0-9a-fA-F]{32}")
}

mergeinclude() {
  PATH=$1
  INCLUDE=$2 &&
  previousrev &&
  STEP="merge ${PATH}/${INCLUDE}: check for merge conflicts" &&
  TMPFILE=$(mktemp) &&
  git -C third_party/externals/freetype2/src/ cat-file blob ${PREVIOUS_FREETYPE_REV}:include/freetype/config/${INCLUDE} >> ${TMPFILE} &&
  git merge-file third_party/freetype2/include/${PATH}/${INCLUDE} ${TMPFILE} third_party/externals/freetype2/src/include/freetype/config/${INCLUDE} &&
  rm ${TMPFILE} &&
  git add third_party/freetype2/include/${PATH}/${INCLUDE}
}

commit() {
  STEP="commit" &&
  git commit --quiet --amend --no-edit
}

rolldeps "$@" &&
mergeinclude freetype-android ftoption.h &&
mergeinclude freetype-android ftconfig.h &&
mergeinclude freetype-no-type1 ftoption.h &&
mergeinclude freetype-no-type1 ftconfig.h &&
commit ||
{ echo "Failed step ${STEP}"; exit 1; }
