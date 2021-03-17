
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
void out_half(out float v) {
    v = colorWhite.x;
}
void out_half2(out vec2 v) {
    v = vec2(colorWhite.y);
}
void out_half3(out vec3 v) {
    v = vec3(colorWhite.z);
}
void out_half4(out vec4 v) {
    v = vec4(colorWhite.w);
}
void out_half2x2(out mat2 v) {
    v = mat2(colorWhite.x);
}
void out_half3x3(out mat3 v) {
    v = mat3(colorWhite.y);
}
void out_half4x4(out mat4 v) {
    v = mat4(colorWhite.z);
}
void out_int(out int v) {
    v = int(colorWhite.x);
}
void out_int2(out ivec2 v) {
    v = ivec2(int(colorWhite.y));
}
void out_int3(out ivec3 v) {
    v = ivec3(int(colorWhite.z));
}
void out_int4(out ivec4 v) {
    v = ivec4(int(colorWhite.w));
}
void out_float(out float v) {
    v = colorWhite.x;
}
void out_float2(out vec2 v) {
    v = vec2(colorWhite.y);
}
void out_float3(out vec3 v) {
    v = vec3(colorWhite.z);
}
void out_float4(out vec4 v) {
    v = vec4(colorWhite.w);
}
void out_float2x2(out mat2 v) {
    v = mat2(colorWhite.x);
}
void out_float3x3(out mat3 v) {
    v = mat3(colorWhite.y);
}
void out_float4x4(out mat4 v) {
    v = mat4(colorWhite.z);
}
void out_bool(out bool v) {
    v = bool(colorWhite.x);
}
void out_bool2(out bvec2 v) {
    v = bvec2(bool(colorWhite.y));
}
void out_bool3(out bvec3 v) {
    v = bvec3(bool(colorWhite.z));
}
void out_bool4(out bvec4 v) {
    v = bvec4(bool(colorWhite.w));
}
vec4 main() {
    float h;
    out_half(h);
    vec2 h2;
    out_half2(h2);
    vec3 h3;
    out_half3(h3);
    vec4 h4;
    out_half4(h4);
    out_half(h3.y);
    out_half2(h3.xz);
    out_half4(h4.zwxy);
    mat2 h2x2;
    out_half2x2(h2x2);
    mat3 h3x3;
    out_half3x3(h3x3);
    mat4 h4x4;
    out_half4x4(h4x4);
    out_half3(h3x3[1]);
    out_half(h4x4[3].w);
    out_half(h2x2[0].x);
    int i;
    out_int(i);
    ivec2 i2;
    out_int2(i2);
    ivec3 i3;
    out_int3(i3);
    ivec4 i4;
    out_int4(i4);
    out_int3(i4.xyz);
    out_int(i2.y);
    float f;
    out_float(f);
    vec2 f2;
    out_float2(f2);
    vec3 f3;
    out_float3(f3);
    vec4 f4;
    out_float4(f4);
    out_float2(f3.xy);
    out_float(f2.x);
    mat2 f2x2;
    out_float2x2(f2x2);
    mat3 f3x3;
    out_float3x3(f3x3);
    mat4 f4x4;
    out_float4x4(f4x4);
    out_float(f2x2[0].x);
    bool b;
    out_bool(b);
    bvec2 b2;
    out_bool2(b2);
    bvec3 b3;
    out_bool3(b3);
    bvec4 b4;
    out_bool4(b4);
    out_bool2(b4.xw);
    out_bool(b3.z);
    bool ok = true;
    ok = ok && 1.0 == (((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x;
    ok = ok && 1.0 == (((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x;
    ok = ok && 1 == ((i * i2.x) * i3.x) * i4.x;
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    return ok ? colorGreen : colorRed;
}
