
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool takes_float2(vec2 x) {
    return true;
}
bool takes_float3(vec3 x) {
    return true;
}
bool takes_float4(vec4 x) {
    return true;
}
bool takes_float2x2(mat2 x) {
    return true;
}
bool takes_float3x3(mat3 x) {
    return true;
}
bool takes_float4x4(mat4 x) {
    return true;
}
bool takes_half(float x) {
    return true;
}
bool takes_half2(vec2 x) {
    return true;
}
bool takes_half3(vec3 x) {
    return true;
}
bool takes_half4(vec4 x) {
    return true;
}
bool takes_half2x2(mat2 x) {
    return true;
}
bool takes_half3x3(mat3 x) {
    return true;
}
bool takes_half4x4(mat4 x) {
    return true;
}
bool takes_bool(bool x) {
    return true;
}
bool takes_bool2(bvec2 x) {
    return true;
}
bool takes_bool3(bvec3 x) {
    return true;
}
bool takes_bool4(bvec4 x) {
    return true;
}
bool takes_int(int x) {
    return true;
}
bool takes_int2(ivec2 x) {
    return true;
}
bool takes_int3(ivec3 x) {
    return true;
}
bool takes_int4(ivec4 x) {
    return true;
}
vec4 main() {
    return ((((((((((((((((((((true && takes_float2(vec2(2.0))) && takes_float3(vec3(3.0))) && takes_float4(vec4(4.0))) && takes_float2x2(mat2(2.0))) && takes_float3x3(mat3(3.0))) && takes_float4x4(mat4(4.0))) && takes_half(1.0)) && takes_half2(vec2(2.0))) && takes_half3(vec3(3.0))) && takes_half4(vec4(4.0))) && takes_half2x2(mat2(2.0))) && takes_half3x3(mat3(3.0))) && takes_half4x4(mat4(4.0))) && takes_bool(true)) && takes_bool2(bvec2(true))) && takes_bool3(bvec3(true))) && takes_bool4(bvec4(true))) && takes_int(1)) && takes_int2(ivec2(2))) && takes_int3(ivec3(3))) && takes_int4(ivec4(4)) ? colorGreen : colorRed;
}
