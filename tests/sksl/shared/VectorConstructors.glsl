
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float unknownInput;
bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(vec2 v1, vec2 v2, vec2 v3, vec3 v4, ivec2 v5, ivec2 v6, vec2 v7, vec2 v8, vec4 v9, ivec2 v10, bvec4 v11, vec2 v12, vec2 v13, vec2 v14, bvec2 v15, bvec2 v16, bvec3 v17) {
    return (((((((((((((((v1.x + v2.x) + v3.x) + v4.x) + float(v5.x)) + float(v6.x)) + v7.x) + v8.x) + v9.x) + float(v10.x)) + float(v11.x)) + v12.x) + v13.x) + v14.x) + float(v15.x)) + float(v16.x)) + float(v17.x) == 17.0;
}
vec4 main() {
    vec2 v1 = vec2(1.0);
    vec2 v2 = vec2(1.0, 2.0);
    vec2 v3 = vec2(1.0);
    vec3 v4 = vec3(vec2(1.0), 1.0);
    ivec2 v5 = ivec2(1);
    ivec2 v6 = ivec2(1, 2);
    vec2 v7 = vec2(1.0, 2.0);
    vec2 v8 = vec2(v5);
    vec4 v9 = vec4(float(v6.x), unknownInput, 3.0, 4.0);
    ivec2 v10 = ivec2(3, int(v1.x));
    bvec4 v11 = bvec4(true, false, true, false);
    vec2 v12 = vec2(1.0, 0.0);
    vec2 v13 = vec2(0.0);
    vec2 v14 = vec2(0.0);
    bvec2 v15 = bvec2(true);
    bvec2 v16 = bvec2(true);
    bvec3 v17 = bvec3(true, bvec2(true));
    return check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17) ? colorGreen : colorRed;
}
