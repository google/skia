
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 expected = vec4(-0.021816615, 0.0, 0.01308997, 0.03926991);
    const vec4 allowedDelta = vec4(0.0005);
    return ((abs(radians(testInputs.x) - -0.021816615) < 0.0005 && all(lessThan(abs(radians(testInputs.xy) - vec2(-0.021816615, 0.0)), vec2(0.0005)))) && all(lessThan(abs(radians(testInputs.xyz) - vec3(-0.021816615, 0.0, 0.01308997)), vec3(0.0005)))) && all(lessThan(abs(radians(testInputs) - expected), allowedDelta)) ? colorGreen : colorRed;
}
