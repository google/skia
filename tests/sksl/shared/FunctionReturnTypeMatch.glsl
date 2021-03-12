
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
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
    float x1 = 1.0;
    vec2 x2 = vec2(2.0);
    vec3 x3 = vec3(3.0);
    vec4 x4 = vec4(4.0);
    mat2 x5 = mat2(2.0);
    mat3 x6 = mat3(3.0);
    mat4 x7 = mat4(4.0);
    float x8 = 1.0;
    vec2 x9 = vec2(2.0);
    vec3 x10 = vec3(3.0);
    vec4 x11 = vec4(4.0);
    mat2 x12 = mat2(2.0);
    mat3 x13 = mat3(3.0);
    mat4 x14 = mat4(4.0);
    bool x15 = true;
    bvec2 x16 = bvec2(true);
    bvec3 x17 = bvec3(true);
    bvec4 x18 = bvec4(true);
    int x19 = 1;
    ivec2 x20 = ivec2(2);
    ivec3 x21 = ivec3(3);
    ivec4 x22 = ivec4(4);
    return ((((((((((((((((((((x1 == 1.0 && x2 == returns_float2()) && x3 == returns_float3()) && x4 == returns_float4()) && x5 == returns_float2x2()) && x6 == returns_float3x3()) && x7 == returns_float4x4()) && x8 == returns_half()) && x9 == returns_half2()) && x10 == returns_half3()) && x11 == returns_half4()) && x12 == returns_half2x2()) && x13 == returns_half3x3()) && x14 == returns_half4x4()) && x15 == returns_bool()) && x16 == returns_bool2()) && x17 == returns_bool3()) && x18 == returns_bool4()) && x19 == returns_int()) && x20 == returns_int2()) && x21 == returns_int3()) && x22 == returns_int4() ? colorGreen : colorRed;
}
