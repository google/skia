in uniform float maxOffset;

@make {
    static sk_sp<GrFragmentProcessor> Make(GrPixelConfig dstConfig) {
        float maxOffset;
        switch (dstConfig) {
            case kAlpha_8_GrPixelConfig:
            case kGray_8_GrPixelConfig:
            case kRGBA_8888_GrPixelConfig:
            case kBGRA_8888_GrPixelConfig:
            case kSRGBA_8888_GrPixelConfig:
            case kSBGRA_8888_GrPixelConfig:
                maxOffset = 0.5f / 255.f;
                break;
            case kRGB_565_GrPixelConfig:
                maxOffset = 0.5f / 31.f;
                break;
            case kRGBA_4444_GrPixelConfig:
                maxOffset = 0.5f / 15.f;
                break;
            case kUnknown_GrPixelConfig:
            case kAlpha_half_GrPixelConfig:
            case kRGBA_8888_sint_GrPixelConfig:
            case kRGBA_float_GrPixelConfig:
            case kRG_float_GrPixelConfig:
            case kRGBA_half_GrPixelConfig:
                return nullptr;
        }
        return sk_sp<GrFragmentProcessor>(new GrDitherEffect(maxOffset));
    }
}

void main() {
    // Generate a random number based on the fragment position. For this
    // random number generator, we use the "GLSL rand" function
    // that seems to be floating around on the internet. It works under
    // the assumption that sin(<big number>) oscillates with high frequency
    // and sampling it will generate "randomness". Since we're using this
    // for rendering and not cryptography it should be OK.

    // For each channel c, add the random offset to the pixel to either bump
    // it up or let it remain constant during quantization.
    float r = fract(sin(dot(sk_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
    sk_OutColor = clamp(maxOffset * vec4(r) + sk_InColor, 0, 1);
}

@test(testData) {
    float maxOffset = testData->fRandom->nextRangeF(0.001f, 0.05f);
    return sk_sp<GrFragmentProcessor>(new GrDitherEffect(maxOffset));
}
