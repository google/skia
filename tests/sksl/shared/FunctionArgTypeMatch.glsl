
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool takes_void_b() {
    return true;
}
bool takes_float_bf(float x) {
    return true;
}
bool takes_float2_bf2(vec2 x) {
    return true;
}
bool takes_float3_bf3(vec3 x) {
    return true;
}
bool takes_float4_bf4(vec4 x) {
    return true;
}
bool takes_float2x2_bf22(mat2 x) {
    return true;
}
bool takes_float3x3_bf33(mat3 x) {
    return true;
}
bool takes_float4x4_bf44(mat4 x) {
    return true;
}
bool takes_half_bh(float x) {
    return true;
}
bool takes_half2_bh2(vec2 x) {
    return true;
}
bool takes_half3_bh3(vec3 x) {
    return true;
}
bool takes_half4_bh4(vec4 x) {
    return true;
}
bool takes_half2x2_bh22(mat2 x) {
    return true;
}
bool takes_half3x3_bh33(mat3 x) {
    return true;
}
bool takes_half4x4_bh44(mat4 x) {
    return true;
}
bool takes_bool_bb(bool x) {
    return true;
}
bool takes_bool2_bb2(bvec2 x) {
    return true;
}
bool takes_bool3_bb3(bvec3 x) {
    return true;
}
bool takes_bool4_bb4(bvec4 x) {
    return true;
}
bool takes_int_bi(int x) {
    return true;
}
bool takes_int2_bi2(ivec2 x) {
    return true;
}
bool takes_int3_bi3(ivec3 x) {
    return true;
}
bool takes_int4_bi4(ivec4 x) {
    return true;
}
vec4 main() {
    return ((((((((((((((((((((((true && takes_void_b()) && takes_float_bf(1.0)) && takes_float2_bf2(vec2(2.0))) && takes_float3_bf3(vec3(3.0))) && takes_float4_bf4(vec4(4.0))) && takes_float2x2_bf22(mat2(2.0))) && takes_float3x3_bf33(mat3(3.0))) && takes_float4x4_bf44(mat4(4.0))) && takes_half_bh(1.0)) && takes_half2_bh2(vec2(2.0))) && takes_half3_bh3(vec3(3.0))) && takes_half4_bh4(vec4(4.0))) && takes_half2x2_bh22(mat2(2.0))) && takes_half3x3_bh33(mat3(3.0))) && takes_half4x4_bh44(mat4(4.0))) && takes_bool_bb(true)) && takes_bool2_bb2(bvec2(true))) && takes_bool3_bb3(bvec3(true))) && takes_bool4_bb4(bvec4(true))) && takes_int_bi(1)) && takes_int2_bi2(ivec2(2))) && takes_int3_bi3(ivec3(3))) && takes_int4_bi4(ivec4(4)) ? colorGreen : colorRed;
}
