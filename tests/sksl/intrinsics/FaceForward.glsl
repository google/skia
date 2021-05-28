
out vec4 sk_FragColor;
uniform vec4 N;
uniform vec4 I;
uniform vec4 NRef;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 expectedPos = vec4(1.0, 2.0, 3.0, 4.0);
    vec4 expectedNeg = vec4(-1.0, -2.0, -3.0, -4.0);
    return ((((((faceforward(N.x, I.x, NRef.x) == expectedNeg.x && faceforward(N.xy, I.xy, NRef.xy) == expectedNeg.xy) && faceforward(N.xyz, I.xyz, NRef.xyz) == expectedPos.xyz) && faceforward(N, I, NRef) == expectedPos) && -1.0 == expectedNeg.x) && vec2(-1.0, -2.0) == expectedNeg.xy) && vec3(1.0, 2.0, 3.0) == expectedPos.xyz) && vec4(1.0, 2.0, 3.0, 4.0) == expectedPos ? colorGreen : colorRed;
}
