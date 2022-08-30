
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float returns_float_f() {
    return 1.0;
}
vec2 returns_float2_f2() {
    return vec2(2.0);
}
vec3 returns_float3_f3() {
    return vec3(3.0);
}
vec4 returns_float4_f4() {
    return vec4(4.0);
}
mat2 returns_float2x2_f22() {
    return mat2(2.0);
}
mat3 returns_float3x3_f33() {
    return mat3(3.0);
}
mat4 returns_float4x4_f44() {
    return mat4(4.0);
}
float returns_half_h() {
    return 1.0;
}
vec2 returns_half2_h2() {
    return vec2(2.0);
}
vec3 returns_half3_h3() {
    return vec3(3.0);
}
vec4 returns_half4_h4() {
    return vec4(4.0);
}
mat2 returns_half2x2_h22() {
    return mat2(2.0);
}
mat3 returns_half3x3_h33() {
    return mat3(3.0);
}
mat4 returns_half4x4_h44() {
    return mat4(4.0);
}
bool returns_bool_b() {
    return true;
}
bvec2 returns_bool2_b2() {
    return bvec2(true);
}
bvec3 returns_bool3_b3() {
    return bvec3(true);
}
bvec4 returns_bool4_b4() {
    return bvec4(true);
}
int returns_int_i() {
    return 1;
}
ivec2 returns_int2_i2() {
    return ivec2(2);
}
ivec3 returns_int3_i3() {
    return ivec3(3);
}
ivec4 returns_int4_i4() {
    return ivec4(4);
}
vec4 main() {
    float x1 = returns_float_f();
    vec2 x2 = returns_float2_f2();
    vec3 x3 = returns_float3_f3();
    vec4 x4 = returns_float4_f4();
    mat2 x5 = returns_float2x2_f22();
    mat3 x6 = returns_float3x3_f33();
    mat4 x7 = returns_float4x4_f44();
    float x8 = returns_half_h();
    vec2 x9 = returns_half2_h2();
    vec3 x10 = returns_half3_h3();
    vec4 x11 = returns_half4_h4();
    mat2 x12 = returns_half2x2_h22();
    mat3 x13 = returns_half3x3_h33();
    mat4 x14 = returns_half4x4_h44();
    bool x15 = returns_bool_b();
    bvec2 x16 = returns_bool2_b2();
    bvec3 x17 = returns_bool3_b3();
    bvec4 x18 = returns_bool4_b4();
    int x19 = returns_int_i();
    ivec2 x20 = returns_int2_i2();
    ivec3 x21 = returns_int3_i3();
    ivec4 x22 = returns_int4_i4();
    return ((((((((((((((((((((x1 == returns_float_f() && x2 == returns_float2_f2()) && x3 == returns_float3_f3()) && x4 == returns_float4_f4()) && x5 == returns_float2x2_f22()) && x6 == returns_float3x3_f33()) && x7 == returns_float4x4_f44()) && x8 == returns_half_h()) && x9 == returns_half2_h2()) && x10 == returns_half3_h3()) && x11 == returns_half4_h4()) && x12 == returns_half2x2_h22()) && x13 == returns_half3x3_h33()) && x14 == returns_half4x4_h44()) && x15 == returns_bool_b()) && x16 == returns_bool2_b2()) && x17 == returns_bool3_b3()) && x18 == returns_bool4_b4()) && x19 == returns_int_i()) && x20 == returns_int2_i2()) && x21 == returns_int3_i3()) && x22 == returns_int4_i4() ? colorGreen : colorRed;
}
