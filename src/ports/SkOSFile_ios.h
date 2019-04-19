/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSFile_ios_DEFINED
#define SkOSFile_ios_DEFINED

#include "SkString.h"

#ifdef SK_BUILD_FOR_IOS
#import <CoreFoundation/CoreFoundation.h>

static bool ios_get_path_in_bundle(const char path[], SkString* result) {
    // Get a reference to the main bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();

    // Get a reference to the file's URL
    CFStringRef pathRef = CFStringCreateWithCString(nullptr, path, kCFStringEncodingUTF8);
    // We use "data" as our subdirectory to match {{bundle_resources_dir}}/data in GN
    // Unfortunately "resources" is not a valid top-level name in iOS, so we push it one level down
    CFURLRef imageURL = CFBundleCopyResourceURL(mainBundle, pathRef, nullptr, CFSTR("data"));
    CFRelease(pathRef);
    if (!imageURL) {
        return false;
    }
    if (!result) {
        return true;
    }

    // Convert the URL reference into a string reference
    CFStringRef imagePath = CFURLCopyFileSystemPath(imageURL, kCFURLPOSIXPathStyle);
    CFRelease(imageURL);

    // Get the system encoding method
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    // Convert the string reference into an SkString
    result->set(CFStringGetCStringPtr(imagePath, encodingMethod));
    CFRelease(imagePath);
    return true;
}
#endif

#endif  // SkOSFile_ios_DEFINED
