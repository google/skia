/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSFile_ios_DEFINED
#define SkOSFile_ios_DEFINED

#include "include/core/SkString.h"

#ifdef SK_BUILD_FOR_IOS
#import <CoreFoundation/CoreFoundation.h>

#include "include/ports/SkCFObject.h"

static bool ios_get_path_in_bundle(const char path[], SkString* result) {
    // Get a reference to the main bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();

    // Get a reference to the file's URL
    // Use this to normalize the path
    sk_cfp<CFURLRef> pathURL(CFURLCreateFromFileSystemRepresentation(/*allocator=*/nullptr,
                                                                     (const UInt8*)path,
                                                                     strlen(path),
                                                                     /*isDirectory=*/false));
    sk_cfp<CFStringRef> pathRef(CFURLCopyFileSystemPath(pathURL.get(), kCFURLPOSIXPathStyle));
    // We use "data" as our subdirectory to match {{bundle_resources_dir}}/data in GN
    // Unfortunately "resources" is not a valid top-level name in iOS, so we push it one level down
    sk_cfp<CFURLRef> fileURL(CFBundleCopyResourceURL(mainBundle, pathRef.get(),
                                                     /*resourceType=*/nullptr, CFSTR("data")));
    if (!fileURL) {
        return false;
    }
    if (!result) {
        return true;
    }

    // Convert the URL reference into a string reference
    sk_cfp<CFStringRef> filePath(CFURLCopyFileSystemPath(fileURL.get(), kCFURLPOSIXPathStyle));

    // Get the system encoding method
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    // Convert the string reference into an SkString
    result->set(CFStringGetCStringPtr(filePath.get(), encodingMethod));
    return true;
}
#endif

#endif  // SkOSFile_ios_DEFINED
