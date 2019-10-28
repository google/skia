STRINGIFY(

// defines built-in interfaces supported by SkiaSL fragment shaders

// See "enum SpvBuiltIn_" in ./spirv.h
layout(builtin=15) in float4 sk_FragCoord;
layout(builtin=17) in bool sk_Clockwise;  // Similar to gl_FrontFacing, but defined in device space.
layout(builtin=3) float sk_ClipDistance[1];

// 9999 is a temporary value that causes us to ignore these declarations beyond
// adding them to the symbol table. This works fine in GLSL (where they do not
// require any further handling) but will fail in SPIR-V. We'll have a better
// solution for this soon.
layout(builtin=9999) int gl_SampleMaskIn[1];
layout(builtin=9999) out int gl_SampleMask[1];
layout(builtin=9999) out half4 gl_SecondaryFragColorEXT;

layout(location=0,index=0,builtin=10001) out half4 sk_FragColor;
layout(builtin=10008) half4 sk_LastFragColor;
layout(builtin=10011) half sk_Width;
layout(builtin=10012) half sk_Height;

)
