
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 expected = vec4(-71.61973, 0.0, 42.9718361, 128.915512);
    const vec4 allowedDelta = vec4(0.05);
    return ((abs(degrees(testInputs.x) - -71.61973) < 0.05 && all(lessThan(abs(degrees(testInputs.xy) - vec2(-71.61973, 0.0)), vec2(0.05)))) && all(lessThan(abs(degrees(testInputs.xyz) - vec3(-71.61973, 0.0, 42.9718361)), vec3(0.05)))) && all(lessThan(abs(degrees(testInputs) - expected), allowedDelta)) ? colorGreen : colorRed;
}
