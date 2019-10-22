STRINGIFY(

// defines built-in interfaces supported by SkiaSL fragment shaders

layout(builtin=15) in float4 sk_FragCoord;
layout(builtin=3) float sk_ClipDistance[1];
layout(builtin=20) out int sk_SampleMask[1];

// 9999 is a temporary value that causes us to ignore these declarations beyond
// adding them to the symbol table. This works fine in GLSL (where they do not
// require any further handling) but will fail in SPIR-V. We'll have a better
// solution for this soon.
layout(builtin=9999) float4 gl_LastFragData[1];
layout(builtin=9999) half4 gl_LastFragColor;
layout(builtin=9999) half4 gl_LastFragColorARM;
layout(builtin=9999) half4 gl_SecondaryFragColorEXT;

layout(builtin=10003) half4 sk_InColor;
layout(builtin=10004) out half4 sk_OutColor;
layout(builtin=10005) float2[] sk_TransformedCoords2D;
layout(builtin=10006) sampler2D[] sk_TextureSamplers;
layout(builtin=10011) half sk_Width;
layout(builtin=10012) half sk_Height;

half4 sample(fragmentProcessor fp);
half4 sample(fragmentProcessor fp, float2 coords);
half4 sample(fragmentProcessor fp, half4 input);
half4 sample(fragmentProcessor fp, half4 input, float2 coords);
half4 sample(fragmentProcessor? fp);
half4 sample(fragmentProcessor? fp, float2 coords);
half4 sample(fragmentProcessor? fp, half4 input);
half4 sample(fragmentProcessor? fp, half4 input, float2 coords);
)
