/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontScanner.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/ports/SkFontMgr_directory.h"
#include "src/core/SkOSFile.h"
#include "src/ports/SkFontMgr_custom.h"
#include "src/ports/SkTypeface_FreeType.h"
#include "src/utils/SkOSPath.h"

class DirectorySystemFontLoader : public SkFontMgr_Custom::SystemFontLoader {
public:
    DirectorySystemFontLoader(const char* dir) : fBaseDirectory(dir) { }

    void loadSystemFonts(const SkFontScanner* scanner,
                         SkFontMgr_Custom::Families* families) const override
    {
        load_directory_fonts(scanner, fBaseDirectory, ".ttf", families);
        load_directory_fonts(scanner, fBaseDirectory, ".ttc", families);
        load_directory_fonts(scanner, fBaseDirectory, ".otf", families);
        load_directory_fonts(scanner, fBaseDirectory, ".pfb", families);

        if (families->empty()) {
            SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
            families->push_back().reset(family);
            family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
        }
    }

private:
    static SkFontStyleSet_Custom* find_family(SkFontMgr_Custom::Families& families,
                                              const char familyName[])
    {
       for (int i = 0; i < families.size(); ++i) {
            if (families[i]->getFamilyName().equals(familyName)) {
                return families[i].get();
            }
        }
        return nullptr;
    }

    static void load_directory_fonts(const SkFontScanner* scanner,
                                     const SkString& directory, const char* suffix,
                                     SkFontMgr_Custom::Families* families)
    {
        SkOSFile::Iter iter(directory.c_str(), suffix);
        SkString name;

        while (iter.next(&name, false)) {
            SkString filename(SkOSPath::Join(directory.c_str(), name.c_str()));
            std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(filename.c_str());
            if (!stream) {
                // SkDebugf("---- failed to open <%s>\n", filename.c_str());
                continue;
            }

            int numFaces;
            if (!scanner->scanFile(stream.get(), &numFaces)) {
                // SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
                continue;
            }

            for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
                int numInstances;
                if (!scanner->scanFace(stream.get(), faceIndex, &numInstances)) {
                    // SkDebugf("---- failed to open <%s> as a font\n", filename.c_str());
                    continue;
                }
                for (int instanceIndex = 0; instanceIndex <= numInstances; ++instanceIndex) {
                    bool isFixedPitch;
                    SkString realname;
                    SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
                    if (!scanner->scanInstance(stream.get(),
                                               faceIndex,
                                               instanceIndex,
                                               &realname,
                                               &style,
                                               &isFixedPitch,
                                               nullptr, nullptr)) {
                        // SkDebugf("---- failed to open <%s> <%d> as a font\n",
                        //          filename.c_str(), faceIndex);
                        continue;
                    }

                    SkFontStyleSet_Custom* addTo = find_family(*families, realname.c_str());
                    if (nullptr == addTo) {
                        addTo = new SkFontStyleSet_Custom(realname);
                        families->push_back().reset(addTo);
                    }
                    addTo->appendTypeface(sk_make_sp<SkTypeface_File>(
                            style, isFixedPitch, true, realname, filename.c_str(),
                            (instanceIndex << 16) + faceIndex));
                }
            }
        }

        SkOSFile::Iter dirIter(directory.c_str());
        while (dirIter.next(&name, true)) {
            if (name.startsWith(".")) {
                continue;
            }
            SkString dirname(SkOSPath::Join(directory.c_str(), name.c_str()));
            load_directory_fonts(scanner, dirname, suffix, families);
        }
    }

    SkString fBaseDirectory;
};

sk_sp<SkFontMgr> SkFontMgr_New_Custom_Directory(const char* dir) {
    return sk_make_sp<SkFontMgr_Custom>(DirectorySystemFontLoader(dir));
}
