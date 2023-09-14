
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
    sk_FragColor.x = float(lessThan(a, b).x);
    sk_FragColor.y = float(lessThan(c, d).y);
    sk_FragColor.z = float(lessThan(e, f).z);
    sk_FragColor.w = float(any(expectTTFF) || any(expectFFTT));
}
