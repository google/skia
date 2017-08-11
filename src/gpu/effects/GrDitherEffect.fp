// This controls the range of values added to color channels
layout(key) in int rangeType;

@make {
    static std::unique_ptr<GrFragmentProcessor> Make(GrPixelConfig dstConfig) {
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
        return std::unique_ptr<GrFragmentProcessor>(new GrDitherEffect(rangeType));
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
            range = 1.0 / 15.0;
            break;
    }
    @if (sk_Caps.integerSupport) {
        // This ordered-dither code is lifted from the cpu backend.
        uint x = uint(sk_FragCoord.x);
        uint y = uint(sk_FragCoord.y);
        uint m = (y & 1) << 5 | (x & 1) << 4 |
                 (y & 2) << 2 | (x & 2) << 1 |
                 (y & 4) >> 1 | (x & 4) >> 2;
        value = float(m) * 1.0 / 64.0 - 63.0 / 128.0;
    } else {
        // Simulate the integer effect used above using step/mod. For speed, simulates a 4x4
        // dither pattern rather than an 8x8 one.
        float4 modValues = mod(sk_FragCoord.xyxy, float4(2.0, 2.0, 4.0, 4.0));
        float4 stepValues = step(modValues, float4(1.0, 1.0, 2.0, 2.0));
        value = dot(stepValues, float4(8.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0)) - 15.0 / 32.0;
    }
    // For each color channel, add the random offset to the channel value and then clamp
    // between 0 and alpha to keep the color premultiplied.
    sk_OutColor = float4(clamp(sk_InColor.rgb + value * range, 0, sk_InColor.a), sk_InColor.a);
}

@test(testData) {
    float range = testData->fRandom->nextRangeF(0.001f, 0.05f);
    return std::unique_ptr<GrFragmentProcessor>(new GrDitherEffect(range));
}
