
out vec4 sk_FragColor;
uniform vec4 pi;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    return ((abs(cos(pi.x)) < 0.0010000000474974513 && all(lessThan(abs(cos(pi.xy) - vec2(0.0, -1.0)), vec2(0.0010000000474974513)))) && all(lessThan(abs(cos(pi.xyz) - vec3(0.0, -1.0, 0.0)), vec3(0.0010000000474974513)))) && all(lessThan(abs(cos(pi) - vec4(0.0, -1.0, 0.0, 1.0)), vec4(0.0010000000474974513))) ? colorGreen : colorRed;
}
