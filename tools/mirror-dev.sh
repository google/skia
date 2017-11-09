# Copyright 2013 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

 # For each installed *-dev package DEV
 for DEV in $(dpkg --list | grep '^ii  [^ ]' | cut -d ' ' -f 3 | grep -P '\-dev($|\:amd64$)')
 do
     # For each multi-arch *.so SO installed by DEV
     for DEV_64_SO in $(dpkg -L $DEV | grep '/lib/x86_64-linux-gnu/.*\.so$')
     do
        # Skip if DEV_64_SO is not a symlink
        if ! test -L $DEV_64_SO
        then
            echo "$DEV installed $DEV_64_SO which is real."
            continue
        fi

        DEV_64_TARGET=$(readlink $DEV_64_SO)
        DEV_64_TARGET_FULL=$(readlink -f $DEV_64_SO)

        DEV_32_SO=$(echo $DEV_64_SO | sed -e 's@/lib/x86_64-linux-gnu/@/lib/i386-linux-gnu/@')
        DEV_32_TARGET=$(echo $DEV_64_TARGET | sed -e 's@/lib/x86_64-linux-gnu/@/lib/i386-linux-gnu/@')
        DEV_32_TARGET_FULL=$(echo $DEV_64_TARGET_FULL | sed -e 's@/lib/x86_64-linux-gnu/@/lib/i386-linux-gnu/@')

        # Error if DEV_32_TARGET does not exist.
        if ! test -e $DEV_32_TARGET_FULL
        then
            DEV_64_TARGET_PKG=$(dpkg -S $DEV_64_TARGET_FULL | cut -d ':' -f 1)
            echo "Could not find $DEV_32_TARGET_FULL, probably provided by $DEV_64_TARGET_PKG:i386."
            #echo "   $DEV_64_SO -> $DEV_64_TARGET ($DEV_64_TARGET_FULL)"
            #echo "   $DEV_32_SO -> $DEV_32_TARGET ($DEV_32_TARGET_FULL)"
            continue
        fi

        # Create DEV_32_SO
        sudo ln -s $DEV_32_TARGET $DEV_32_SO
     done
 done
