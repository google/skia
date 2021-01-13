
out vec4 sk_FragColor;
vec2 v1 = vec2(1.0);
vec2 v2 = vec2(1.0, 2.0);
vec2 v3 = vec2(1.0);
vec3 v4 = vec3(vec2(1.0), 1.0);
ivec2 v5 = ivec2(1);
ivec2 v6 = ivec2(vec2(1.0, 2.0));
vec2 v7 = vec2(ivec2(1, 2));
vec2 v8 = vec2(v5);
vec4 v9 = vec4(float(v6.x), sqrt(2.0), vec2(ivec2(3, 4)));
ivec2 v10 = ivec2(3, int(v1.x));
bvec4 v11 = bvec4(bvec2(true, false), true, false);
void main() {
    sk_FragColor.x = (((((((((v1.x + v2.x) + v3.x) + v4.x) + float(v5.x)) + float(v6.x)) + v7.x) + v8.x) + v9.x) + float(v10.x)) + float(v11.x);
}
