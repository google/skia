
out vec4 sk_FragColor;
uniform vec4 a;
uniform vec4 b;
uniform uvec2 c;
uniform uvec2 d;
uniform ivec3 e;
uniform ivec3 f;
void main() {
    sk_FragColor.x = float(lessThan(a, b).x ? 1 : 0);
    sk_FragColor.y = float(lessThan(c, d).y ? 1 : 0);
    sk_FragColor.z = float(lessThan(e, f).z ? 1 : 0);
}
