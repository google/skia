
out vec4 sk_FragColor;
in vec4 a;
in vec4 b;
in uvec2 c;
in uvec2 d;
in ivec3 e;
in ivec3 f;
void main() {
    sk_FragColor.x = float(lessThan(a, b).x ? 1 : 0);
    sk_FragColor.y = float(lessThan(c, d).y ? 1 : 0);
    sk_FragColor.z = float(lessThan(e, f).z ? 1 : 0);
}
