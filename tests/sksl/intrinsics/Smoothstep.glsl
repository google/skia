
out vec4 sk_FragColor;
uniform vec4 testInputs;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    const vec4 constVal = vec4(-1.25, 0.0, 0.75, 2.25);
    vec4 expectedA = vec4(0.0, 0.0, 0.84375, 1.0);
    vec4 expectedB = vec4(1.0, 0.0, 1.0, 1.0);
    return ((((((((((((((((((0.0 == expectedA.x && vec2(0.0, 0.0) == expectedA.xy) && vec3(0.0, 0.0, 0.84375) == expectedA.xyz) && vec4(0.0, 0.0, 0.84375, 1.0) == expectedA) && 0.0 == expectedA.x) && vec2(0.0, 0.0) == expectedA.xy) && vec3(0.0, 0.0, 0.84375) == expectedA.xyz) && vec4(0.0, 0.0, 0.84375, 1.0) == expectedA) && smoothstep(colorRed.y, colorGreen.y, -1.25) == expectedA.x) && smoothstep(colorRed.y, colorGreen.y, vec2(-1.25, 0.0)) == expectedA.xy) && smoothstep(colorRed.y, colorGreen.y, vec3(-1.25, 0.0, 0.75)) == expectedA.xyz) && smoothstep(colorRed.y, colorGreen.y, constVal) == expectedA) && 1.0 == expectedB.x) && vec2(1.0, 0.0) == expectedB.xy) && vec3(1.0, 0.0, 1.0) == expectedB.xyz) && vec4(1.0, 0.0, 1.0, 1.0) == expectedB) && smoothstep(colorRed.x, colorGreen.x, -1.25) == expectedB.x) && smoothstep(colorRed.xy, colorGreen.xy, vec2(-1.25, 0.0)) == expectedB.xy) && smoothstep(colorRed.xyz, colorGreen.xyz, vec3(-1.25, 0.0, 0.75)) == expectedB.xyz) && smoothstep(colorRed, colorGreen, constVal) == expectedB ? colorGreen : colorRed;
}
