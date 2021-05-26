
out vec4 sk_FragColor;
uniform vec4 input;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedVec = vec4(1.0, 0.0, 0.0, 0.0);
    return ((((((normalize(input.x) == expectedVec.x && normalize(input.xy) == expectedVec.xy) && normalize(input.xyz) == expectedVec.xyz) && normalize(input) == expectedVec) && 1.0 == expectedVec.x) && vec2(0.0, 1.0) == expectedVec.yx) && vec3(0.0, 1.0, 0.0) == expectedVec.zxy) && vec4(1.0, 0.0, 0.0, 0.0) == expectedVec ? colorGreen : colorRed;
}
