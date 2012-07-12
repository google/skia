#!/bin/bash
#
# Runs doxygen and stores its results in the skia-autogen repo, so that they
# can be browsed at http://skia-autogen.googlecode.com/svn/docs/html/index.html
#
# The DOXYGEN_TEMPDIR env variable is the working directory within which we will
# check out the code, generate documentation, and store the doxygen log
# (by default, /tmp/skia-doxygen). The DOXYGEN_COMMIT env variable determines
# whether docs should be commited (true by default).
#
# Sample Usage:
#  export DOXYGEN_TEMPDIR=/tmp/doxygen
#  export DOXYGEN_COMMIT=false
#  bash update-doxygen.sh

# Prepare a temporary dir and check out Skia trunk and docs.
cd
DOXYGEN_TEMPDIR=${DOXYGEN_TEMPDIR:-/tmp/skia-doxygen}
DOXYGEN_COMMIT=${DOXYGEN_COMMIT:-true}

mkdir -p $DOXYGEN_TEMPDIR
cd $DOXYGEN_TEMPDIR
svn checkout http://skia.googlecode.com/svn/trunk  # read-only
svn checkout https://skia-autogen.googlecode.com/svn/docs  # writeable

# Run Doxygen.
cd trunk
doxygen Doxyfile
cd ../docs

# Add any newly created files to Subversion.
NEWFILES=$(svn status | grep ^\? | awk '{print $2}')
if [ -n "$NEWFILES" ]; then
  svn add $NEWFILES
fi

# We haven't updated the timestamp footer yet... if there are no changes
# yet, just exit. (We'll wait until there are any actual doc changes before
# updating the timestamp and committing changes to the repository.)
MODFILES=$(svn status | grep ^[AM])
if [ -z "$MODFILES" ]; then
  echo "No documentation updates, exiting early."
  exit 0
fi

# Update the timestamp footer.
cat >iframe_footer.html <<EOF
<html><body>
<address style="text-align: right;"><small>
Generated on $(date) for skia by
<a href="http://www.doxygen.org/index.html">doxygen</a>
$(doxygen --version) </small></address>
</body></html>
EOF

# Make sure that all files have the correct mimetype.
find . -name '*.html' -exec svn propset svn:mime-type text/html '{}' \;
find . -name '*.css'  -exec svn propset svn:mime-type text/css '{}' \;
find . -name '*.js'   -exec svn propset svn:mime-type text/javascript '{}' \;
find . -name '*.gif'  -exec svn propset svn:mime-type image/gif '{}' \;
find . -name '*.png'  -exec svn propset svn:mime-type image/png '{}' \;

# Output files with documentation updates.
echo -e "\n\nThe following are the documentation updates:"
echo $MODFILES

if $DOXYGEN_COMMIT ; then
  # Commit the updated docs to the subversion repo.
  svn commit --message 'commit doxygen-generated documentation'
fi
