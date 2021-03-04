
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
vec4 main() {
    float h;
    h = colorWhite.x;

    vec2 h2;
    h2 = vec2(colorWhite.y);

    vec3 h3;
    h3 = vec3(colorWhite.z);

    vec4 h4;
    h4 = vec4(colorWhite.w);

    h3.y = colorWhite.x;

    h3.xz = vec2(colorWhite.y);

    h4.zwxy = vec4(colorWhite.w);

    mat2 h2x2;
    h2x2 = mat2(colorWhite.x);

    mat3 h3x3;
    h3x3 = mat3(colorWhite.y);

    mat4 h4x4;
    h4x4 = mat4(colorWhite.z);

    h3x3[1] = vec3(colorWhite.z);

    h4x4[3].w = colorWhite.x;

    h2x2[0].x = colorWhite.x;

    int i;
    i = int(colorWhite.x);

    ivec2 i2;
    i2 = ivec2(int(colorWhite.y));

    ivec3 i3;
    i3 = ivec3(int(colorWhite.z));

    ivec4 i4;
    i4 = ivec4(int(colorWhite.w));

    i4.xyz = ivec3(int(colorWhite.z));

    i2.y = int(colorWhite.x);

    float f;
    f = colorWhite.x;

    vec2 f2;
    f2 = vec2(colorWhite.y);

    vec3 f3;
    f3 = vec3(colorWhite.z);

    vec4 f4;
    f4 = vec4(colorWhite.w);

    f3.xy = vec2(colorWhite.y);

    f2.x = colorWhite.x;

    mat2 f2x2;
    f2x2 = mat2(colorWhite.x);

    mat3 f3x3;
    f3x3 = mat3(colorWhite.y);

    mat4 f4x4;
    f4x4 = mat4(colorWhite.z);

    f2x2[0].x = colorWhite.x;

    bool b;
    b = bool(colorWhite.x);

    bvec2 b2;
    b2 = bvec2(bool(colorWhite.y));

    bvec3 b3;
    b3 = bvec3(bool(colorWhite.z));

    bvec4 b4;
    b4 = bvec4(bool(colorWhite.w));

    b4.xw = bvec2(bool(colorWhite.y));

    b3.z = bool(colorWhite.x);

    bool ok = true;
    ok = 1.0 == (((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x;
    ok = ok && 1.0 == (((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x;
    ok = ok && 1 == ((i * i2.x) * i3.x) * i4.x;
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    return ok ? colorGreen : colorRed;
}
