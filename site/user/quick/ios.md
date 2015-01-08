iOS
===

Prerequisites
-------------

_These steps should closely follow building on Mac OS X. Those steps seem slightly out of date._

Build and run SampleApp in the XCode IDE
----------------------------------------

### XCode 4.5


To build SampleApp on XCode 4.5 using the IDE these steps should work:

    GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7=1 arm_neon=0" ./gyp_skia
    xed out/gyp/SampleApp.xcodeproj # opens the SampleApp project in the IDE

Note that if you run make at the command line the gyp\_skia script will rerun
and you'll lose the effect of the GYP\_DEFINES. To avoid this do:

    export GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7=1 arm_neon=0"

### XCode 3

Use GYP\_DEFINES to tell gyp\_skia how to build for iOS. Here's a bash shell
snippet that sets the world up to build SampleApp with XCode 3:

    function buildSampleApp()
    {
      sdkVersion="4.3"
      if [[ "$1" == "sim" ]] ; then
        export GYP_DEFINES="skia_os='ios' skia_arch_type='x86' \
          ios_sdk_dir='/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator' \
          ios_sdk_version='$sdkVersion'"
      elif [[ "$1" == "iphone" ]] ; then
        export GYP_DEFINES="skia_os='ios' skia_arch_type='arm' armv7='1' arm_neon='0' \
          ios_sdk_dir='/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS' \
          ios_sdk_version='$sdkVersion'"
      elif [[ "$1" == "mac" ]] ; then
        export GYP_DEFINES=""
      else
        echo "buildSampleApp  expects 'sim', 'iphone', or 'mac'"
      fi
      if [[ "$1" == "sim" ]] || [[ "$1" == "iphone" ]] || [[ "$1" == "mac" ]] ; then
        save=`pwd`
        cd /chrome/nih/skia/trunk
        echo "$GYP_DEFINES ./gyp_skia gyp/SampleApp.gyp"
        ./gyp_skia gyp/SampleApp.gyp
        cd $save
      fi
      if [[ "$1" == "sim" ]] ; then
        setiossdk iphonesimulator$sdkVersion
      elif [[ "$1" == "iphone" ]] ; then
        setiossdk iphoneos$sdkVersion
      fi
    }

The script function setiossdk called by buildSampleApp is a
not-completely-working hackery. When gyp builds an iOS-targeted project, it is
hard-coded for the iOS simulator. To point the project at either the iOS
simulator, or an iOS device, the project file must be opened to create a
custom pbxuser file.

This is accomplished by:

    function setiossdk()
    {
      osascript -e 'tell app "Xcode" to quit'
      osascript -e 'repeat until appIsRunning("Xcode") is false' -e \
        'do shell script "sleep 1"' -e 'end repeat'
      save=`pwd`
      skia
      cd out/gyp
      for project in *.xcodeproj; do
        open $project
      done
      osascript -e 'tell app "Xcode" to quit'
      osascript -e 'repeat until appIsRunning("Xcode") is false' -e \
        'do shell script "sleep 1"' -e 'end repeat'
      for project in *.xcodeproj; do
        lsave=`pwd`
        cd $project
        filename=`eval whoami`.pbxuser
        while [[ ! -s $filename ]] ; do
          sleep 1
          echo -n "."
        done
        sed -e '/activeSDKPreference/ d' <$filename | sed -e '/activeTarget/ i\
    \                activeSDKPreference = '$1';' >x$filename
        if [[ -s x$filename ]] ; then
          mv x$filename $filename
        else
          echo "mv x$filename $project/$filename failed"
        fi
        cd $lsave
      done
      open SampleApp.xcodeproj
      cd $save
    }

In particular, the calls to osascript to wait for Xcode to quit use faulty syntax.

