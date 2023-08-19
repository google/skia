
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    bvec4 result = equal(lessThan(colorRed, vec4(2.0)), greaterThan(vec4(3.0), colorGreen));
    return all(result) ? colorGreen : colorRed;
}
