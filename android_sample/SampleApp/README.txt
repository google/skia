Building the sample app for Android using an Android tree:

Copy this folder into an Android tree in packages/apps. In addition to jni,
res, and src, there needs to be a fourth folder named "skia_extra".  This
will include the skia files which are not part of an Android checkout. It
should have three folders: include, samplecode, and src.

skia/trunk/include/views -> skia_extra/include/views
skia/trunk/include/xml -> skia_extra/include/xml

skia/trunk/samplecode -> skia_extra/samplecode

skia/trunk/src/views -> skia_extra/src/views
skia/trunk/src/ports/SkXMLParser_empty.cpp -> skia_extra/src/ports/
skia/trunk/src/xml -> skia_extra/src/xml

skia/trunk/include/utils/android/AndroidKeyToSkKey.h -> jni/

From packages/apps/SampleApp, type "mm" to build, and install the
resulting apk.

(It may be necessary to remove samples that do not build from
skia_extra/samplecode/samplecode_files.mk)

TODO: Instructions for building from SDK/NDK
