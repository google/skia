// This controls the range of values added to color channels
layout(key) in int rangeType;

@make {
    static sk_sp<GrFragmentProcessor> Make(GrPixelConfig dstConfig) {
        int rangeType;
        switch (dstConfig) {
            case kGray_8_GrPixelConfig:
            case kRGBA_8888_GrPixelConfig:
            case kBGRA_8888_GrPixelConfig:
            case kSRGBA_8888_GrPixelConfig:
            case kSBGRA_8888_GrPixelConfig:
                rangeType = 0;
                break;
            case kRGB_565_GrPixelConfig:
                rangeType = 1;
                break;
            case kRGBA_4444_GrPixelConfig:
                rangeType = 2;
                break;
            case kUnknown_GrPixelConfig:
            case kAlpha_half_GrPixelConfig:
            case kRGBA_8888_sint_GrPixelConfig:
            case kRGBA_float_GrPixelConfig:
            case kRG_float_GrPixelConfig:
            case kRGBA_half_GrPixelConfig:
            case kAlpha_8_GrPixelConfig:
                return nullptr;
        }
        return sk_sp<GrFragmentProcessor>(new GrDitherEffect(rangeType));
    }
}

void main() {
    float value;
    float range;
    @switch (rangeType) {
        case 0:
            range = 1.0 / 255.0;
            break;
        case 1:
            range = 1.0 / 63.0;
            break;
        default:
            // Experimentally this looks better than the expected value of 1/15.
            range = 0.125 / 15.0;
            break;
    }
    @if (sk_Caps.integerSupport) {
        // This ordered-dither code is lifted from the cpu backend.
        int x = int(sk_FragCoord.x);
        int y = int(sk_FragCoord.y);
        uint m = (y & 1) << 5 | (x & 1) << 4 |
                 (y & 2) << 2 | (x & 2) << 1 |
                 (y & 4) >> 1 | (x & 4) >> 2;
        value = float(m) * 1.0 / 64.0 - 63.0 / 128.0;
    } else {
        // Generate a random number based on the fragment position. For this
        // random number generator, we use the "GLSL rand" function
        // that seems to be floating around on the internet. It works under
        // the assumption that sin(<big number>) oscillates with high frequency
        // and sampling it will generate "randomness". Since we're using this
        // for rendering and not cryptography it should be OK.
        value = fract(sin(dot(sk_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) - .5;
    }
    // For each color channel, add the random offset to the channel value and then clamp
    // between 0 and alpha to keep the color premultiplied.
    sk_OutColor = vec4(clamp(sk_InColor.rgb + value * range, 0, sk_InColor.a), sk_InColor.a);
}

@test(testData) {
    float range = testData->fRandom->nextRangeF(0.001f, 0.05f);
    return sk_sp<GrFragmentProcessor>(new GrDitherEffect(range));
}
