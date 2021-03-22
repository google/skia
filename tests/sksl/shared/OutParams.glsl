
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
void out_half_vh(out float v) {
    v = colorWhite.x;
}
void out_half2_vh2(out vec2 v) {
    v = vec2(colorWhite.y);
}
void out_half3_vh3(out vec3 v) {
    v = vec3(colorWhite.z);
}
void out_half4_vh4(out vec4 v) {
    v = vec4(colorWhite.w);
}
void out_half2x2_vh22(out mat2 v) {
    v = mat2(colorWhite.x);
}
void out_half3x3_vh33(out mat3 v) {
    v = mat3(colorWhite.y);
}
void out_half4x4_vh44(out mat4 v) {
    v = mat4(colorWhite.z);
}
void out_int_vi(out int v) {
    v = int(colorWhite.x);
}
void out_int2_vi2(out ivec2 v) {
    v = ivec2(int(colorWhite.y));
}
void out_int3_vi3(out ivec3 v) {
    v = ivec3(int(colorWhite.z));
}
void out_int4_vi4(out ivec4 v) {
    v = ivec4(int(colorWhite.w));
}
void out_float_vf(out float v) {
    v = colorWhite.x;
}
void out_float2_vf2(out vec2 v) {
    v = vec2(colorWhite.y);
}
void out_float3_vf3(out vec3 v) {
    v = vec3(colorWhite.z);
}
void out_float4_vf4(out vec4 v) {
    v = vec4(colorWhite.w);
}
void out_float2x2_vf22(out mat2 v) {
    v = mat2(colorWhite.x);
}
void out_float3x3_vf33(out mat3 v) {
    v = mat3(colorWhite.y);
}
void out_float4x4_vf44(out mat4 v) {
    v = mat4(colorWhite.z);
}
void out_bool_vb(out bool v) {
    v = bool(colorWhite.x);
}
void out_bool2_vb2(out bvec2 v) {
    v = bvec2(bool(colorWhite.y));
}
void out_bool3_vb3(out bvec3 v) {
    v = bvec3(bool(colorWhite.z));
}
void out_bool4_vb4(out bvec4 v) {
    v = bvec4(bool(colorWhite.w));
}
vec4 main() {
    float h;
    out_half_vh(h);
    vec2 h2;
    out_half2_vh2(h2);
    vec3 h3;
    out_half3_vh3(h3);
    vec4 h4;
    out_half4_vh4(h4);
    out_half_vh(h3.y);
    out_half2_vh2(h3.xz);
    out_half4_vh4(h4.zwxy);
    mat2 h2x2;
    out_half2x2_vh22(h2x2);
    mat3 h3x3;
    out_half3x3_vh33(h3x3);
    mat4 h4x4;
    out_half4x4_vh44(h4x4);
    out_half3_vh3(h3x3[1]);
    out_half_vh(h4x4[3].w);
    out_half_vh(h2x2[0].x);
    int i;
    out_int_vi(i);
    ivec2 i2;
    out_int2_vi2(i2);
    ivec3 i3;
    out_int3_vi3(i3);
    ivec4 i4;
    out_int4_vi4(i4);
    out_int3_vi3(i4.xyz);
    out_int_vi(i2.y);
    float f;
    out_float_vf(f);
    vec2 f2;
    out_float2_vf2(f2);
    vec3 f3;
    out_float3_vf3(f3);
    vec4 f4;
    out_float4_vf4(f4);
    out_float2_vf2(f3.xy);
    out_float_vf(f2.x);
    mat2 f2x2;
    out_float2x2_vf22(f2x2);
    mat3 f3x3;
    out_float3x3_vf33(f3x3);
    mat4 f4x4;
    out_float4x4_vf44(f4x4);
    out_float_vf(f2x2[0].x);
    bool b;
    out_bool_vb(b);
    bvec2 b2;
    out_bool2_vb2(b2);
    bvec3 b3;
    out_bool3_vb3(b3);
    bvec4 b4;
    out_bool4_vb4(b4);
    out_bool2_vb2(b4.xw);
    out_bool_vb(b3.z);
    bool ok = true;
    ok = ok && 1.0 == (((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x;
    ok = ok && 1.0 == (((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x;
    ok = ok && 1 == ((i * i2.x) * i3.x) * i4.x;
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    return ok ? colorGreen : colorRed;
}
