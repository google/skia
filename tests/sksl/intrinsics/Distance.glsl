
out vec4 sk_FragColor;
uniform vec4 pos1;
uniform vec4 pos2;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expected = vec4(3.0, 3.0, 5.0, 13.0);
    return ((((((distance(pos1.x, pos2.x) == expected.x && distance(pos1.xy, pos2.xy) == expected.y) && distance(pos1.xyz, pos2.xyz) == expected.z) && distance(pos1, pos2) == expected.w) && 3.0 == expected.x) && 3.0 == expected.y) && 5.0 == expected.z) && 13.0 == expected.w ? colorGreen : colorRed;
}
