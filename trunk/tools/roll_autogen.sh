#/bin/bash
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# roll_autogen.sh: Helper script for removing old revisions from an svn
# repository.  Unfortunately, the only way to discard old revisions is to clone
# the repository locally, use svnadmin to dump a range of commits from the local
# copy, re-import them into a brand-new repository, "reset" the original repo,
# and then import the commits from the new repository into the original.  This
# script automates all of that except for resetting the original repository.

REPO=${REPO:-"https://skia-autogen.googlecode.com"}
REVS_TO_KEEP=${REVS_TO_KEEP:-50}
REPO_SVN="${REPO}/svn"
CLONE_DIR="local_clone_dir"
LOCAL_CLONE="$(pwd)/${CLONE_DIR}"

echo "Creating local repository in ${LOCAL_CLONE}"
svnadmin create ${LOCAL_CLONE}
pushd ${LOCAL_CLONE}/hooks > /dev/null
echo "#!/bin/sh" > pre-revprop-change
chmod 755 pre-revprop-change 
popd > /dev/null

# Determine the latest revision.  Note that any revisions committed while we
# were syncing will be lost forever!
END=`svn info ${REPO_SVN} | grep Revision | cut -c11-`
START=$((END-REVS_TO_KEEP))
DUMPFILE="skia-autogen_r${START}-${END}.dump"

echo "Cloning ${REPO_SVN} into ${LOCAL_CLONE}..."
svnsync init file://${LOCAL_CLONE} ${REPO_SVN}
svnsync --non-interactive sync file://${LOCAL_CLONE}

echo "Dumping revisions ${START} to ${END} to ${DUMPFILE}."
svnadmin dump --revision ${START}:${END} ${LOCAL_CLONE} > ${DUMPFILE}

echo "Removing temporary local clone."
rm -rf ${LOCAL_CLONE}

echo "Re-creating local clone from ${DUMPFILE}."
svnadmin create ${LOCAL_CLONE}
svnadmin load ${LOCAL_CLONE} < ${DUMPFILE}

echo "Deleting ${DUMPFILE}"
rm ${DUMPFILE}

echo "Now you need to reset the remote repository. Typically, a link to do this"
echo "can be found at (${REPO}/adminSource).
echo "Please do so and press any key to continue."
read -n 1 -s

echo "Syncing ${LOCAL_CLONE} to ${REPO_SVN}."
svnsync init ${REPO_SVN} file://${LOCAL_CLONE}
svnsync sync ${REPO_SVN}

echo "Removing temporary local clone."
rm -rf ${LOCAL_CLONE}

echo "Removing local checkout."
rm -rf ${CHECKOUT_DIR}

echo "Finished!"
