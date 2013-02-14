# This directory contains tarballs of a subset of real performance
# data from our production bots.
#
# How I created these tarballs:

PLATFORM=Skia_Shuttle_Ubuntu12_ATI5770_Float_Bench_32
TEMPDIR=$(mktemp -d)


# DOWNLOAD SELECTED RAW DATA FROM GOOGLE STORAGE

REVS="7671 7679 7686"
for REV in $REVS; do

  FILES="bench_r${REV}_data_skp_device_bitmap_multi_2_mode_tile_256_256_timeIndividualTiles bench_r${REV}_data_skp_device_bitmap_multi_3_mode_tile_256_256_timeIndividualTiles bench_r${REV}_data_skp_device_bitmap_multi_4_mode_tile_256_256_timeIndividualTiles"

  for FILE in $FILES; do
    URL=http://chromium-skia-gm.commondatastorage.googleapis.com/playback/perfdata/${PLATFORM}/data/${FILE}
    curl $URL --output $TEMPDIR/$FILE
  done

done


# TAR UP THE RAW DATA

TARBALL_DIR="$PWD/tools/tests/benchgraphs/$PLATFORM/tarballs"
mkdir -p $TARBALL_DIR
pushd $TEMPDIR
for REV in $REVS; do
  tar --create --gzip --file ${TARBALL_DIR}/r${REV}.tgz bench_r${REV}_*
done 
popd


# CLEAN UP

rm -rf $TEMPDIR
