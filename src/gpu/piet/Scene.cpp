/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/piet/Scene.h"

#include "include/core/SkTypes.h"

namespace skgpu::piet {
namespace {

bool get_piet_verb(SkPath::Verb verb, PgpuPathVerb* outVerb) {
    switch (verb) {
        case SkPath::kMove_Verb:
            *outVerb = PgpuPathVerb::MoveTo;
            break;
        case SkPath::kLine_Verb:
            *outVerb = PgpuPathVerb::LineTo;
            break;
        case SkPath::kQuad_Verb:
            *outVerb = PgpuPathVerb::QuadTo;
            break;
        case SkPath::kConic_Verb:
        case SkPath::kCubic_Verb:
            *outVerb = PgpuPathVerb::CurveTo;
            break;
        case SkPath::kClose_Verb:
            *outVerb = PgpuPathVerb::Close;
            break;
        case SkPath::kDone_Verb:
        default:
            return false;
    }
    return true;
}

PgpuFill get_fill_type(SkPathFillType fillType) {
    switch (fillType) {
        case SkPathFillType::kWinding:
            return PgpuFill::NonZero;
        case SkPathFillType::kEvenOdd:
            return PgpuFill::EvenOdd;
        default:
            // TODO(b/238756757): pgpu-render doesn't define fill types for kInverseWinding and
            // kInverseEvenOdd. This should be updated to support those cases.
            SkDebugf("piet: unsupported fill type\n");
            break;
    }
    return PgpuFill::NonZero;
}

}  // namespace

Scene::Scene() : Object(pgpu_scene_new()) { SkASSERT(this->get() != nullptr); }

void Scene::addPath(const SkPath& path, const Transform& transform) {
    this->initBuilderIfNecessary();

    SkPath::Iter pathIter(path, /*forceClose=*/false);
    PgpuPathIter iter;
    iter.context = &pathIter;
    iter.next_element = [](void* context, PgpuPathElement* outElem) {
        SkASSERT(outElem);
        SkASSERT(context);

        SkPoint points[4];
        SkPath::Verb verb = static_cast<SkPath::Iter*>(context)->next(points);
        if (!get_piet_verb(verb, &outElem->verb)) {
            return false;
        }

        for (int i = 0; i < 3; ++i) {
            const SkPoint& p = points[i];
            outElem->points[i] = {p.x(), p.y()};
        }

        return true;
    };

    pgpu_scene_builder_transform(fActiveBuilder->get(), &transform);
    pgpu_scene_builder_fill_path(
            fActiveBuilder->get(), get_fill_type(path.getFillType()), nullptr, nullptr, &iter);
}

bool Scene::finalize() {
    if (!fActiveBuilder) {
        return false;
    }
    fActiveBuilder.reset();
    return true;
}

void Scene::initBuilderIfNecessary() {
    if (!fActiveBuilder) {
        fActiveBuilder = Builder(pgpu_scene_builder_for_scene(this->get()));
        SkASSERT(fActiveBuilder);
    }
}

}  // namespace skgpu::piet
