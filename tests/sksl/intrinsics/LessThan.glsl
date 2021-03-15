
out vec4 sk_FragColor;
vec4 a;
vec4 b;
uvec2 c;
uvec2 d;
ivec3 e;
ivec3 f;
void main() {
    sk_FragColor.x = float(lessThan(a, b).x ? 1 : 0);
    sk_FragColor.y = float(lessThan(c, d).y ? 1 : 0);
    sk_FragColor.z = float(lessThan(e, f).z ? 1 : 0);
}
