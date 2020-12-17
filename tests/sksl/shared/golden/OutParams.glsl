
out vec4 sk_FragColor;
void main() {
    vec3 h3;
    h3 = vec3(3.0);

    vec4 h4;
    h4 = vec4(4.0);

    h3.y = 1.0;

    h3.xz = vec2(2.0);

    h4.zwxy = vec4(4.0);

    sk_FragColor = vec4(1.0, 2.0, h3.x, h4.x);
    mat2 h2x2;
    h2x2 = mat2(2.0);

    mat3 h3x3;
    h3x3 = mat3(3.0);

    mat4 h4x4;
    h4x4 = mat4(4.0);

    h3x3[1] = vec3(3.0);

    h4x4[3].w = 1.0;

    h2x2[0].x = 1.0;

    sk_FragColor = vec4(h2x2[0].x, h3x3[0].x, h4x4[0].x, 1.0);
    ivec2 i2;
    i2 = ivec2(2);

    ivec4 i4;
    i4 = ivec4(4);

    i4.xyz = ivec3(3);

    i2.y = 1;

    sk_FragColor = vec4(1.0, float(i2.x), 3.0, float(i4.x));
    vec2 f2;
    f2 = vec2(2.0);

    vec3 f3;
    f3 = vec3(3.0);

    f3.xy = vec2(2.0);

    f2.x = 1.0;

    sk_FragColor = vec4(1.0, f2.x, f3.x, 4.0);
    mat2 f2x2;
    f2x2 = mat2(2.0);

    f2x2[0].x = 1.0;

    sk_FragColor = vec4(f2x2[0].x, mat3(3.0)[0].x, mat4(4.0)[0].x, 1.0);
    bvec3 b3;
    b3 = bvec3(true);

    bvec4 b4;
    b4 = bvec4(false);

    b4.xw = bvec2(false);

    b3.z = true;

    sk_FragColor = vec4(1.0, false ? 1.0 : 0.0, b3.x ? 1.0 : 0.0, b4.x ? 1.0 : 0.0);
}
