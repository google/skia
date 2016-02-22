# Copyright 2015 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

headerLoc=../third_party/vulkan/vulkan.h
outFile=tempVkInterface

if [ ! -e "$outFile" ] ; then
    echo 'I AM HERE'
    touch "$outFile"
fi

chmod 700 $outFile

echo '// *******************************************' > $outFile
echo '// Place these lines into GrVkInterface.cpp::validate' >> $outFile
echo '// *******************************************' >> $outFile
sed -n 's/^VKAPI_ATTR \(VkResult\|void\) VKAPI_CALL vk\([a-zA-Z]*\).*/NULL == fFunctions.f\2 ||/p' $headerLoc >> $outFile
sed -i '1,/NULL/ s/^NULL/if (NULL/' $outFile
sed -i '5,$ s/^/    /' $outFile
sed -i '$ s/ ||/) {/' $outFile

echo '' >> $outFile
echo '// *******************************************' >> $outFile
echo '// Place these lines into GrVkInterface.h' >> $outFile
echo '// *******************************************' >> $outFile
sed -n 's/^VKAPI_ATTR \(VkResult\|void\) VKAPI_CALL vk\([a-zA-Z]*\).*/VkPtr<PFN_vk\2> f\2;/p' $headerLoc >> $outFile

echo '' >> $outFile
echo '// *******************************************' >> $outFile
echo '// Place these lines into GrVkInterface.cpp::GrVKCreateInterface' >> $outFile
echo '// *******************************************' >> $outFile
sed -n 's/^VKAPI_ATTR \(VkResult\|void\) VKAPI_CALL vk\([a-zA-Z]*\).*/GET_PROC(\2);/p' $headerLoc >> $outFile

