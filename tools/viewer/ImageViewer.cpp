/*
 * Copyright 2018 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ImageViewer.h"

#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkImage.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkTSort.h"

#include "imgui.h"

using namespace sk_app;

Application* Application::Create(int argc, char** argv, void* platformData) {
    return new ImageViewer(argc, argv, platformData);
}

static void get_file_list(const char* dir, SkTArray<SkString> *files) {
    SkOSFile::Iter fileIterator(dir);
    SkString fileName;
    while (fileIterator.next(&fileName, false)) {
        if (fileName.startsWith(".")) {
            continue;
        }
        files->push_back(SkOSPath::Join(dir, fileName.c_str()));
    }

    SkOSFile::Iter dirIterator(dir);
    SkString dirName;
    while (dirIterator.next(&dirName, true)) {
        if (dirName.startsWith(".")) {
            continue;
        }
        get_file_list(SkOSPath::Join(dir, dirName.c_str()).c_str(), files);
    }
}

ImageViewer::ImageViewer(int argc, char** argv, void* platformData) {
    SkGraphics::Init();

    for (int i = 1; i < argc; ++i) {
        fSrcDirs.push_back(SkString(argv[i]));
    }
    if (fSrcDirs.empty()) {
        fSrcDirs.push_back(SkString("."));
    }

    fWindow = Window::CreateNativeWindow(platformData);
    fWindow->setRequestedDisplayParams(DisplayParams());

    fCommands.attach(fWindow);
    fWindow->pushLayer(this);
    fWindow->pushLayer(&fImGuiLayer);

    this->initImages();

    // add key-bindings
    fCommands.addCommand('1', "View", "Limit Scale to 100%",
                         [this]() { fLimitScale = !fLimitScale; });

    fWindow->attach(Window::BackendType::kNativeGL_BackendType);
}

#if defined(SK_BUILD_FOR_WIN)
    #define strcasecmp _stricmp
#endif

void ImageViewer::initImages() {
    SkTArray<SkString> allFiles;
    for (auto srcDir : fSrcDirs) {
        get_file_list(srcDir.c_str(), &allFiles);
    }

    fImageNames.reset();
    fImagesByName.reset();
    for (auto filename : allFiles) {
        SkString basename = SkOSPath::Basename(filename.c_str());
        if (basename.isEmpty()) {
            continue;
        }
        SkString dirname = SkOSPath::Dirname(filename.c_str());
        if (dirname.isEmpty()) {
            dirname = ".";
        }
        SkTArray<SkString>* dirList = nullptr;
        if (SkTArray<SkString>** entry = fImagesByName.find(basename)) {
            dirList = *entry;
        } else {
            dirList = new SkTArray<SkString>();
            fImagesByName.set(basename, dirList);
            fImageNames.push_back(basename);
        }
        dirList->push_back(dirname);
    }

    SkTQSort(fImageNames.begin(), fImageNames.end() - 1, [](const SkString& a, const SkString& b) {
        return strcasecmp(a.c_str(), b.c_str()) < 0;
    });
}

ImageViewer::~ImageViewer() {
    fWindow->detach();
    delete fWindow;
}

bool ImGui_FilteredListBox::draw(const char* label, int* index, int numItems,
                                 std::function<const char*(int)> items) {
    fFilteredItems.resize(0);
    fFilteredIndices.resize(0);

    int curIndex = -1;

    fFilter.Draw();
    for (int i = 0; i < numItems; ++i) {
        const char* item = items(i);
        if (fFilter.PassFilter(item) || i == *index) {
            if (i == *index) {
                curIndex = fFilteredItems.size();
            }
            fFilteredItems.push_back(item);
            fFilteredIndices.push_back(i);
        }
    }

    if (ImGui::ListBox(label, &curIndex, fFilteredItems.begin(), fFilteredItems.size()) &&
        curIndex >= 0 && curIndex < fFilteredItems.size() &&
        fFilteredIndices[curIndex] != *index) {
        *index = fFilteredIndices[curIndex];
        return true;
    }

    return false;
}

void ImageViewer::drawImagePicker() {
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Images")) {
        if (ImGui::Button("Rescan")) {
            this->initImages();
        }

        bool newImages = fImageListBox.draw("Image", &fCurrentImage, fImageNames.count(),
                                            [this](int i) { return fImageNames[i].c_str(); });
        if (newImages) {
            fImages.reset();
            fVisibleDirs.resize(0);
            SkString basename = fImageNames[fCurrentImage];
            SkTArray<SkString>** entry = fImagesByName.find(basename);
            SkTArray<SkString>* dirList = *entry;
            for (auto dirname : *dirList) {
                fVisibleDirs.push_back(true);
                SkString path = SkOSPath::Join(dirname.c_str(), basename.c_str());
                sk_sp<SkData> data(SkData::MakeFromFileName(path.c_str()));
                sk_sp<SkImage> image(SkImage::MakeFromEncoded(std::move(data)));
                if (image) {
                    fImages.push_back({ dirname, std::move(image) });
                }
            }
        }

        if (fImages.count()) {
            SkString basename = fImageNames[fCurrentImage];
            SkTArray<SkString>** entry = fImagesByName.find(basename);
            SkTArray<SkString>* dirList = *entry;
            for (int i = 0; i < dirList->count(); ++i) {
                ImGui::Checkbox((*dirList)[i].c_str(), &fVisibleDirs[i]);
            }
        }
    }
    ImGui::End();
}

void ImageViewer::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        ImGui::Text("Scale: %3.0f%% %c", fScale * 100.0f, fLimitScale ? '*' : ' ');

        ImGui::EndMainMenuBar();
    }
}

/*
 * Given [count] rectangles, each one [w] x [h], and a single sized [totalW] x [totalH], what is
 * the maximum scale at which they can be drawn in a uniform grid (and how many rows are required).
 *
 * Leaves [padW] between each interior column, and [padH] between each interior row.
 */
static void fit_rects(int count, int w, int h, int padW, int padH, int totalW, int totalH,
                      int* bestRows, float* bestScale) {
    *bestScale = 0.0f;
    *bestRows = 0;

    for (int rows = 1; rows <= count; ++rows) {
        int cols = (count + (rows - 1)) / rows;
        int availW = totalW - ((cols - 1) * padW);
        int availH = totalH - ((rows - 1) * padH);
        int sumRectW = w * cols;
        int sumRectH = h * rows;
        float scaleW = static_cast<float>(availW) / sumRectW;
        float scaleH = static_cast<float>(availH) / sumRectH;
        float scale  = SkTMin(scaleW, scaleH);
        if (scale > *bestScale) {
            *bestScale = scale;
            *bestRows = rows;
        }
    }
}

void ImageViewer::drawImages(SkCanvas* canvas) {
    int maxImgW = 0;
    int maxImgH = 0;
    int numVisibleImages = 0;
    for (int i = 0; i < fImages.count(); ++i) {
        if (fVisibleDirs[i]) {
            maxImgW = SkTMax(maxImgW, fImages[i].fImage->width());
            maxImgH = SkTMax(maxImgH, fImages[i].fImage->height());
            ++numVisibleImages;
        }
    }

    SkPaint labelPaint;
    labelPaint.setAntiAlias(true);
    labelPaint.setColor(SK_ColorWHITE);

    const int kPadW = 5;
    const int kPadH = kPadW + labelPaint.getFontSpacing();
    const int kMenuHeight = 20;

    int availW = fWindow->width()  - (2 * kPadW);
    int availH = fWindow->height() - (kMenuHeight + kPadH + kPadW);

    int rows = 0;
    fit_rects(numVisibleImages, maxImgW, maxImgH, kPadW, kPadH, availW, availH, &rows, &fScale);

    if (fLimitScale) {
        fScale = SkTMin(fScale, 1.0f);
    }

    SkPaint imgPaint;
    imgPaint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(kPadW, kMenuHeight + labelPaint.getFontSpacing());
    canvas->drawLine(0, 0, 20.0f, 0, imgPaint);
    canvas->save();
    int cols = rows ? (numVisibleImages + (rows - 1)) / rows : 0;
    int imagesOnThisRow = 0;
    for (int i = 0; i < fImages.count(); ++i) {
        if (fVisibleDirs[i]) {
            canvas->drawString(fImages[i].fLabel, 0, 0, labelPaint);
            canvas->save();
            canvas->translate(0, kPadW);
            canvas->scale(fScale, fScale);
            canvas->drawImage(fImages[i].fImage, 0, 0, &imgPaint);
            canvas->restore();
            if (++imagesOnThisRow >= cols) {
                canvas->restore();
                canvas->translate(0, fImages[i].fImage->height() * fScale + kPadH);
                canvas->save();
                imagesOnThisRow = 0;
            } else {
                canvas->translate(fImages[i].fImage->width() * fScale + kPadW, 0);
            }
        }
    }
}

void ImageViewer::onBackendCreated() {
    fWindow->setTitle("ImageViewer");
    fWindow->show();
    fWindow->inval();
}

void ImageViewer::onPaint(SkCanvas* canvas) {
    canvas->clear(SK_ColorGRAY);
    this->drawImagePicker();    // GUI
    this->drawImages(canvas);   // Actual images
    this->drawMenuBar();        // Menu / status
    fCommands.drawHelp(canvas);
}

bool ImageViewer::onMouse(int x, int y, Window::InputState state, uint32_t modifiers) {
    switch (state) {
        case Window::kUp_InputState:
            fGesture.touchEnd(nullptr);
            break;
        case Window::kDown_InputState:
            fGesture.touchBegin(nullptr, x, y);
            break;
        case Window::kMove_InputState:
            fGesture.touchMoved(nullptr, x, y);
            break;
    }
    fWindow->inval();
    return true;
}

void ImageViewer::onIdle() {
    fWindow->inval();
}

bool ImageViewer::onKey(Window::Key key, Window::InputState state, uint32_t modifiers) {
    return fCommands.onKey(key, state, modifiers);
}

bool ImageViewer::onChar(SkUnichar c, uint32_t modifiers) {
    return fCommands.onChar(c, modifiers);
}
