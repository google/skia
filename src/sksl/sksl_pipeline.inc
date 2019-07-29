STRINGIFY(
    // special-cased within the compiler - append takes various arguments depending on what kind of
    // stage is being appended
    sk_has_side_effects void append();

    float abs(float x);
    float sin(float x);
    float cos(float y);
    float tan(float x);
    float sqrt(float x);
    float clamp(float x, float min, float max);
    float2 clamp(float2 x, float min, float max);
    float3 clamp(float3 x, float min, float max);
    float4 clamp(float4 x, float min, float max);
    sk_has_side_effects void print(float x);
    layout(builtin=10009) int sk_x;
    layout(builtin=10010) int sk_y;
    layout(builtin=10004) out half4 sk_OutColor;

    half4 sample(fragmentProcessor fp);
)
