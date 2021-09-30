
out vec4 sk_FragColor;
uniform vec4 inputVal;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedVec = vec4(1.0, 0.0, 0.0, 0.0);
    return ((((((normalize(inputVal.x) == expectedVec.x && normalize(inputVal.xy) == expectedVec.xy) && normalize(inputVal.xyz) == expectedVec.xyz) && normalize(inputVal) == expectedVec) && 1.0 == expectedVec.x) && vec2(0.0, 1.0) == expectedVec.yx) && vec3(0.0, 1.0, 0.0) == expectedVec.zxy) && vec4(1.0, 0.0, 0.0, 0.0) == expectedVec ? colorGreen : colorRed;
}
