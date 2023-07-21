
out vec4 sk_FragColor;
uniform vec4 N;
uniform vec4 I;
uniform vec4 NRef;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float huge = faceforward(1.0, 1e+30, 1e+30);
    vec2 huge2 = faceforward(vec2(1.0), vec2(1e+30), vec2(1e+30));
    vec3 huge3 = faceforward(vec3(1.0), vec3(1e+30), vec3(1e+30));
    vec4 huge4 = faceforward(vec4(1.0), vec4(1e+30), vec4(1e+30));
    vec4 expectedPos = vec4(huge) + huge2.xxxx;
    vec4 expectedNeg = huge3.xxxx + huge4.xxxx;
    expectedPos = vec4(1.0, 2.0, 3.0, 4.0);
    expectedNeg = vec4(-1.0, -2.0, -3.0, -4.0);
    return ((((((faceforward(N.x, I.x, NRef.x) == expectedNeg.x && faceforward(N.xy, I.xy, NRef.xy) == expectedNeg.xy) && faceforward(N.xyz, I.xyz, NRef.xyz) == expectedPos.xyz) && faceforward(N, I, NRef) == expectedPos) && -1.0 == expectedNeg.x) && vec2(-1.0, -2.0) == expectedNeg.xy) && vec3(1.0, 2.0, 3.0) == expectedPos.xyz) && vec4(1.0, 2.0, 3.0, 4.0) == expectedPos ? colorGreen : colorRed;
}
