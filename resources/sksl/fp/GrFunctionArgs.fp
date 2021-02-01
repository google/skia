/*#pragma settings NoInline*/

// Verify that all the basic ES2 types are supported as FP helper function arguments.
bool takes_float       (float f1, float2 f2, float3 f3, float4 f4) { return true; }
bool takes_float_matrix(float2x2 m2, float3x3 m3, float4x4 m4)     { return true; }
bool takes_half        (half h1, half2 h2, half3 h3, half4 h4)     { return true; }
bool takes_half_matrix (half2x2 m2, half3x3 m3, half4x4 m4)        { return true; }
bool takes_bool        (bool b, bool2 b2, bool3 b3, bool4 b4)      { return true; }
bool takes_int         (int i, int2 i2, int3 i3, int4 i4)          { return true; }

half4 main() {
    return takes_float(float(1), float2(2), float3(3), float4(4)) &&
           takes_float_matrix(float2x2(2), float3x3(3), float4x4(4)) &&
           takes_half(half(1), half2(2), half3(3), half4(4)) &&
           takes_half_matrix(half2x2(2), half3x3(3), half4x4(4)) &&
           takes_bool(true, bool2(true), bool3(true), bool4(true)) &&
           takes_int(1, int2(2), int3(3), int4(4))
                ? half4(1) : half4(0);
}
