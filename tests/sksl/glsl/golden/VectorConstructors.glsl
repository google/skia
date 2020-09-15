
out vec4 sk_FragColor;
vec2 v1 = vec2(1.0);
vec2 v2 = vec2(1.0, 2.0);
vec2 v3 = vec2(1.0);
vec3 v4 = vec3(vec2(1.0), 1.0);
ivec2 v5 = ivec2(1);
ivec2 v6 = ivec2(vec2(1.0, 2.0));
vec2 v7 = vec2(ivec2(1, 2));
void main() {
    sk_FragColor.x = (((((v1.x + v2.x) + v3.x) + v4.x) + float(v5.x)) + float(v6.x)) + v7.x;
}
