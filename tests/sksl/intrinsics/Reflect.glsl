
out vec4 sk_FragColor;
uniform vec4 I;
uniform vec4 N;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float expectedX = -49.0;
    vec2 expectedXY = vec2(-169.0, 202.0);
    vec3 expectedXYZ = vec3(-379.0, 454.0, -529.0);
    vec4 expectedXYZW = vec4(-699.0, 838.0, -977.0, 1116.0);
    return ((((((reflect(I.x, N.x) == expectedX && reflect(I.xy, N.xy) == expectedXY) && reflect(I.xyz, N.xyz) == expectedXYZ) && reflect(I, N) == expectedXYZW) && -49.0 == expectedX) && vec2(-169.0, 202.0) == expectedXY) && vec3(-379.0, 454.0, -529.0) == expectedXYZ) && vec4(-699.0, 838.0, -977.0, 1116.0) == expectedXYZW ? colorGreen : colorRed;
}
