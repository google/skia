/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

in fragmentProcessor src;
layout(ctype=SkIRect) in int4 bounds;
uniform float4 boundsUniform;
layout(ctype=SkRect) in float4 srcRect;
in uniform float xInvZoom;
in uniform float yInvZoom;
in uniform float xInvInset;
in uniform float yInvInset;

uniform half2 offset;

half4 main(float2 coord) {
    float2 zoom_coord = offset + coord * float2(xInvZoom, yInvZoom);
    float2 delta = (coord - boundsUniform.xy) * boundsUniform.zw;
    delta = min(delta, float2(1.0) - delta);
    delta *= float2(xInvInset, yInvInset);

    float weight = 0.0;
    if (delta.s < 2.0 && delta.t < 2.0) {
        delta = float2(2.0) - delta;
        float dist = length(delta);
        dist = max(2.0 - dist, 0.0);
        weight = min(dist * dist, 1.0);
    } else {
        float2 delta_squared = delta * delta;
        weight = min(min(delta_squared.x, delta_squared.y), 1.0);
    }

    return sample(src, mix(coord, zoom_coord, weight));
}

@setData(pdman) {
    pdman.set2f(offset, srcRect.x(), srcRect.y());
    pdman.set4f(boundsUniform, bounds.x(), bounds.y(), 1.f/ bounds.width(), 1.f / bounds.height());
}

@test(d) {
    const int kMaxWidth = 200;
    const int kMaxHeight = 200;
    const SkScalar kMaxInset = 20.0f;
    uint32_t width = d->fRandom->nextULessThan(kMaxWidth);
    uint32_t height = d->fRandom->nextULessThan(kMaxHeight);
    SkScalar inset = d->fRandom->nextRangeScalar(1.0f, kMaxInset);

    SkIRect bounds = SkIRect::MakeWH(SkIntToScalar(kMaxWidth), SkIntToScalar(kMaxHeight));
    SkRect srcRect = SkRect::MakeWH(SkIntToScalar(width), SkIntToScalar(height));

    auto src = GrProcessorUnitTest::MakeChildFP(d);
    auto effect = GrMagnifierEffect::Make(std::move(src),
                                          bounds,
                                          srcRect,
                                          srcRect.width() / bounds.width(),
                                          srcRect.height() / bounds.height(),
                                          bounds.width() / inset,
                                          bounds.height() / inset);
    SkASSERT(effect);
    return effect;
}
