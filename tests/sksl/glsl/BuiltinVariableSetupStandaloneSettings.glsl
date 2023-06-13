
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
vec4 func1_f4() {
    bool sk_Clockwise = gl_FrontFacing;
    if (u_skRTFlip.y < 0.0) {
        sk_Clockwise = !sk_Clockwise;
    }
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    return sk_Clockwise ? sk_FragCoord : vec4(0.0);
}
vec4 func2_f4() {
    bool sk_Clockwise = gl_FrontFacing;
    if (u_skRTFlip.y < 0.0) {
        sk_Clockwise = !sk_Clockwise;
    }
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    return sk_Clockwise ? vec4(0.0) : sk_FragCoord;
}
void main() {
    sk_FragColor = func1_f4() + func2_f4();
}
