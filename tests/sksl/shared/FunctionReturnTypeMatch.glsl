
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float returns_float() {
    return 1.0;
}
vec2 returns_float2() {
    return vec2(2.0);
}
vec3 returns_float3() {
    return vec3(3.0);
}
vec4 returns_float4() {
    return vec4(4.0);
}
mat2 returns_float2x2() {
    return mat2(2.0);
}
mat3 returns_float3x3() {
    return mat3(3.0);
}
mat4 returns_float4x4() {
    return mat4(4.0);
}
float returns_half() {
    return 1.0;
}
vec2 returns_half2() {
    return vec2(2.0);
}
vec3 returns_half3() {
    return vec3(3.0);
}
vec4 returns_half4() {
    return vec4(4.0);
}
mat2 returns_half2x2() {
    return mat2(2.0);
}
mat3 returns_half3x3() {
    return mat3(3.0);
}
mat4 returns_half4x4() {
    return mat4(4.0);
}
bool returns_bool() {
    return true;
}
bvec2 returns_bool2() {
    return bvec2(true);
}
bvec3 returns_bool3() {
    return bvec3(true);
}
bvec4 returns_bool4() {
    return bvec4(true);
}
int returns_int() {
    return 1;
}
ivec2 returns_int2() {
    return ivec2(2);
}
ivec3 returns_int3() {
    return ivec3(3);
}
ivec4 returns_int4() {
    return ivec4(4);
}
vec4 main() {
    float x1 = returns_float();
    vec2 x2 = returns_float2();
    vec3 x3 = returns_float3();
    vec4 x4 = returns_float4();
    mat2 x5 = returns_float2x2();
    mat3 x6 = returns_float3x3();
    mat4 x7 = returns_float4x4();
    float x8 = returns_half();
    vec2 x9 = returns_half2();
    vec3 x10 = returns_half3();
    vec4 x11 = returns_half4();
    mat2 x12 = returns_half2x2();
    mat3 x13 = returns_half3x3();
    mat4 x14 = returns_half4x4();
    bool x15 = returns_bool();
    bvec2 x16 = returns_bool2();
    bvec3 x17 = returns_bool3();
    bvec4 x18 = returns_bool4();
    int x19 = returns_int();
    ivec2 x20 = returns_int2();
    ivec3 x21 = returns_int3();
    ivec4 x22 = returns_int4();
    return ((((((((((((((((((((x1 == returns_float() && x2 == returns_float2()) && x3 == returns_float3()) && x4 == returns_float4()) && x5 == returns_float2x2()) && x6 == returns_float3x3()) && x7 == returns_float4x4()) && x8 == returns_half()) && x9 == returns_half2()) && x10 == returns_half3()) && x11 == returns_half4()) && x12 == returns_half2x2()) && x13 == returns_half3x3()) && x14 == returns_half4x4()) && x15 == returns_bool()) && x16 == returns_bool2()) && x17 == returns_bool3()) && x18 == returns_bool4()) && x19 == returns_int()) && x20 == returns_int2()) && x21 == returns_int3()) && x22 == returns_int4() ? colorGreen : colorRed;
}
