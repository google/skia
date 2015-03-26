/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "SkPatchGrid.h"
#include "SkString.h"

 /**
 * This bench measures the rendering time of a gridof patches. 
 * This bench also tests the different combination of optional parameters for the function 
 * (passing texture coordinates and colors, only textures coordinates, only colors or none).
 * Finally, it also has 3 possible sizes small, medium and big to test if the size of the patches
 * in the grid affects time. 
 */

class PatchGridBench : public Benchmark {
    
public:
    
    enum Size {
        kSmall_Size,
        kMedium_Size,
        kBig_Size
    };
    
    enum VertexMode {
        kNone_VertexMode,
        kColors_VertexMode,
        kTexCoords_VertexMode,
        kBoth_VertexMode
    };
    
    PatchGridBench(Size size, VertexMode vertexMode)
    : fVertexMode(vertexMode)
    , fSize(size) { }
    
    void setScale(SkCanvas* canvas){
        switch (fSize) {
            case kSmall_Size:
                canvas->scale(0.1f, 0.1f);
                break;
            case kMedium_Size:
                canvas->scale(1.0f, 1.0f);
                break;
            case kBig_Size:
                canvas->scale(3.0f, 3.0f);
                break;
        }
    }
    
    void setGrid() {
        SkPoint vertices[4][5] = {
            {{50,50}, {150,50}, {250,50},{350,50},{450,50}},
            {{50,150}, {120,120}, {250,150},{350,150},{450,150}},
            {{50,250}, {150,250}, {250,250},{350,250},{450,250}},
            {{100,300}, {150,350}, {250,350},{350,350},{450,350}}
        };
        
        SkColor cornerColors[4][5] = {
            {SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE},
            {SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED},
            {SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE},
            {SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED},
        };
        
        SkPoint texCoords[4][5] = {
            {{0.0f,0.0f}, {1.0f,0.0f}, {2.0f,0.0f}, {3.0f,0.0f}, {4.0f,0.0f}},
            {{0.0f,1.0f}, {1.0f,1.0f}, {2.0f,1.0f}, {3.0f,1.0f}, {4.0f,1.0f}},
            {{0.0f,2.0f}, {1.0f,2.0f}, {2.0f,2.0f}, {3.0f,2.0f}, {4.0f,2.0f}},
            {{0.0f,3.0f}, {1.0f,3.0f}, {2.0f,3.0f}, {3.0f,3.0f}, {4.0f,3.0f}},
        };
        
        SkPoint hrzCtrl[4][8] = {
            {{75,30},{125,45},{175,70},{225,20},{275,50},{325,50},{375,5},{425,90}},
            {{75,150},{125,150},{175,150},{225,150},{275,150},{325,150},{375,150},{425,150}},
            {{75,250},{125,250},{175,250},{225,250},{275,200},{325,150},{375,250},{425,250}},
            {{75,350},{125,350},{175,350},{225,350},{275,350},{325,350},{375,350},{425,350}}
        };
        
        SkPoint vrtCtrl[6][5] = {
            {{50,75},{150,75},{250,75},{350,75},{450,75}},
            {{50,125},{150,125},{250,125},{350,125},{450,125}},
            {{50,175},{150,175},{220,225},{350,175},{470,225}},
            {{50,225},{150,225},{220,175},{350,225},{470,155}},
            {{50,275},{150,275},{250,275},{350,275},{400,305}},
            {{50,325},{150,325},{250,325},{350,325},{450,325}}
        };
        
        static const int kRows = 3;
        static const int kCols = 4;
        
        fGrid.reset(kRows, kCols, SkPatchGrid::kColors_VertexType, NULL);
        for (int i = 0; i < kRows; i++) {
            for (int j = 0; j < kCols; j++) {
                SkPoint points[12];
                
                //set corners
                points[SkPatchUtils::kTopP0_CubicCtrlPts] = vertices[i][j];
                points[SkPatchUtils::kTopP3_CubicCtrlPts] = vertices[i][j + 1];
                points[SkPatchUtils::kBottomP0_CubicCtrlPts] = vertices[i + 1][j];
                points[SkPatchUtils::kBottomP3_CubicCtrlPts] = vertices[i + 1][j + 1];
                
                points[SkPatchUtils::kTopP1_CubicCtrlPts] = hrzCtrl[i][j * 2];
                points[SkPatchUtils::kTopP2_CubicCtrlPts] = hrzCtrl[i][j * 2 + 1];
                points[SkPatchUtils::kBottomP1_CubicCtrlPts] = hrzCtrl[i + 1][j * 2];
                points[SkPatchUtils::kBottomP2_CubicCtrlPts] = hrzCtrl[i + 1][j * 2 + 1];
                
                points[SkPatchUtils::kLeftP1_CubicCtrlPts] = vrtCtrl[i * 2][j];
                points[SkPatchUtils::kLeftP2_CubicCtrlPts] = vrtCtrl[i * 2 + 1][j];
                points[SkPatchUtils::kRightP1_CubicCtrlPts] = vrtCtrl[i * 2][j + 1];
                points[SkPatchUtils::kRightP2_CubicCtrlPts] = vrtCtrl[i * 2 + 1][j + 1];
                
                SkColor colors[4];
                colors[0] = cornerColors[i][j];
                colors[1] = cornerColors[i][j + 1];
                colors[3] = cornerColors[i + 1][j];
                colors[2] = cornerColors[i + 1][j + 1];
                
                SkPoint texs[4];
                texs[0] = texCoords[i][j];
                texs[1] = texCoords[i][j + 1];
                texs[3] = texCoords[i + 1][j];
                texs[2] = texCoords[i + 1][j + 1];
                
                switch (fVertexMode) {
                    case kNone_VertexMode:
                        fGrid.setPatch(j, i, points, NULL, NULL);
                        break;
                    case kColors_VertexMode:
                        fGrid.setPatch(j, i, points, colors, NULL);
                        break;
                    case kTexCoords_VertexMode:
                        fGrid.setPatch(j, i, points, NULL, texs);
                        break;
                    case kBoth_VertexMode:
                        fGrid.setPatch(j, i, points, colors, texs);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    // override this method to change the shader
    SkShader* createShader() {
        const SkColor colors[] = {
            SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE,
            SK_ColorMAGENTA, SK_ColorBLUE, SK_ColorYELLOW,
        };
        const SkPoint pts[] = { { 200.f / 4.f, 0.f }, { 3.f * 200.f / 4, 200.f } };
        
        return SkGradientShader::CreateLinear(pts, colors, NULL,
                                              SK_ARRAY_COUNT(colors),
                                              SkShader::kMirror_TileMode);
    }

protected:
    const char* onGetName() override {
        SkString vertexMode;
        switch (fVertexMode) {
            case kNone_VertexMode:
                vertexMode.set("meshlines");
                break;
            case kColors_VertexMode:
                vertexMode.set("colors");
                break;
            case kTexCoords_VertexMode:
                vertexMode.set("texs");
                break;
            case kBoth_VertexMode:
                vertexMode.set("colors_texs");
                break;
            default:
                break;
        }
        
        SkString size;
        switch (fSize) {
            case kSmall_Size:
                size.set("small");
                break;
            case kMedium_Size:
                size.set("medium");
                break;
            case kBig_Size:
                size.set("big");
                break;
            default:
                break;
        }
        fName.printf("patch_grid_%s_%s", vertexMode.c_str(), size.c_str());
        return fName.c_str();
    }
    
    void onPreDraw() override {
        this->setGrid();
        switch (fVertexMode) {
            case kTexCoords_VertexMode:
            case kBoth_VertexMode:
                fPaint.setShader(createShader())->unref();
                break;
            default:
                fPaint.setShader(NULL);
                break;
        }
        this->setupPaint(&fPaint);
    }

    void onDraw(const int loops, SkCanvas* canvas) override {
        this->setScale(canvas);
        for (int i = 0; i < loops; i++) {
            fGrid.draw(canvas, fPaint);
        }
    }

    SkPaint     fPaint;
    SkString    fName;
    SkPatchGrid fGrid;
    VertexMode  fVertexMode;
    Size        fSize;
    
    typedef Benchmark INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new PatchGridBench(PatchGridBench::kSmall_Size,
                                     PatchGridBench::kNone_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kSmall_Size,
                                     PatchGridBench::kColors_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kSmall_Size,
                                     PatchGridBench::kTexCoords_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kSmall_Size,
                                     PatchGridBench::kBoth_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kMedium_Size,
                                     PatchGridBench::kNone_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kMedium_Size,
                                     PatchGridBench::kColors_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kMedium_Size,
                                     PatchGridBench::kTexCoords_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kMedium_Size,
                                     PatchGridBench::kBoth_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kBig_Size,
                                     PatchGridBench::kNone_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kBig_Size,
                                     PatchGridBench::kColors_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kBig_Size,
                                     PatchGridBench::kTexCoords_VertexMode); )
DEF_BENCH( return new PatchGridBench(PatchGridBench::kBig_Size,
                                     PatchGridBench::kBoth_VertexMode); )
