/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <SkCanvas.h>
#include "SkSurface.h"
#include "point_cloud_renderer.h"
#include "util.h"

namespace hello_ar {
    namespace {
        SkColor kPointColor = SkColorSetARGB(220, 20, 232, 255);
        SkScalar kPointWidth = 6.0f;
        SkPaint GetPointPaint() {
            SkPaint paint;
            paint.setStrokeCap(SkPaint::kSquare_Cap);
            paint.setStrokeWidth(kPointWidth);
            paint.setColor(kPointColor);
            return paint;
        }
    }

    void PointCloudRenderer::Draw(ArSession* arSession, ArFrame* arFrame, SkSurface* surface, SkMatrix44& vpv)  const {
        ArPointCloud* arPointCloud = nullptr;
        ArStatus pointCloudStatus =
                ArFrame_acquirePointCloud(arSession, arFrame, &arPointCloud);

        if (pointCloudStatus != AR_SUCCESS) {
            return;
        }

        //Number of points to be rendered
        int32_t numberOfPoints = 0;
        ArPointCloud_getNumberOfPoints(arSession, arPointCloud, &numberOfPoints);
        if (numberOfPoints <= 0) {
            return;
        }

        //Get point data
        const float* pointCloudData;
        ArPointCloud_getData(arSession, arPointCloud, &pointCloudData);

        //Transform each point
        std::vector<SkPoint> points;
        glm::mat4 vpvMat = util::SkMatToGlmMat(vpv);
        for (int i = 0; i < numberOfPoints; i++) {
            glm::vec4 p(pointCloudData[i * 4], pointCloudData[i * 4 + 1], pointCloudData[i * 4 + 2], 1);
            p = vpvMat * p;
            points.push_back(SkPoint::Make(p.x / p.w, p.y / p.w));
        }

        //Draw all points
        SkCanvas* planeCanvas = surface->getCanvas();
        planeCanvas->save();
        SkMatrix i = SkMatrix::I();
        planeCanvas->setMatrix(i);
        planeCanvas->drawPoints(SkCanvas::PointMode::kPoints_PointMode, (size_t) numberOfPoints, &points[0], GetPointPaint());
        planeCanvas->flush();

        planeCanvas->restore();
        ArPointCloud_release(arPointCloud);
    }

}  // namespace hello_ar
