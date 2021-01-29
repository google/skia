
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorWhite;
vec4 main() {
    vec4 vector2 = 2.0 * colorWhite;
    return ((((((smoothstep(0.0, 2.0, testInputs.x) == 0.0 && smoothstep(0.0, 2.0, testInputs.xy) == vec2(0.0, 0.0)) && smoothstep(0.0, 2.0, testInputs.xyz) == vec3(0.0, 0.0, 0.31640625)) && smoothstep(0.0, 2.0, testInputs) == vec4(0.0, 0.0, 0.31640625, 1.0)) && smoothstep(0.0, vector2.x, testInputs.x) == 0.0) && smoothstep(vec2(0.0), vector2.xy, testInputs.xy) == vec2(0.0, 0.0)) && smoothstep(vec3(0.0), vector2.xyz, testInputs.xyz) == vec3(0.0, 0.0, 0.31640625)) && smoothstep(vec4(0.0), vector2, testInputs) == vec4(0.0, 0.0, 0.31640625, 1.0) ? colorGreen : colorRed;
}
