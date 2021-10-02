
out vec4 sk_FragColor;
uniform vec4 a;
uniform vec4 b;
uniform uvec2 c;
uniform uvec2 d;
uniform ivec3 e;
uniform ivec3 f;
void main() {
    bvec4 expectTTFF = bvec4(true, true, false, false);
    bvec4 expectFFTT = bvec4(false, false, true, true);
    bvec4 expectTTTT = bvec4(true, true, true, true);
    sk_FragColor.x = float(equal(a, b).x ? 1 : 0);
    sk_FragColor.y = float(equal(c, d).y ? 1 : 0);
    sk_FragColor.z = float(equal(e, f).z ? 1 : 0);
    sk_FragColor.w = float((any(expectTTFF) || any(expectFFTT)) || any(expectTTTT) ? 1 : 0);
}
