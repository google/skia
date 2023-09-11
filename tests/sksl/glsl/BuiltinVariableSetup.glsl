#version 400
uniform vec2 u_skRTFlip;
in vec4 sk_FragCoord_Workaround;
out vec4 sk_FragColor;
vec4 func1_f4() {
    bool sk_Clockwise = gl_FrontFacing;
    if (u_skRTFlip.y < 0.0) {
        sk_Clockwise = !sk_Clockwise;
    }
    float sk_FragCoord_InvW = 1. / sk_FragCoord_Workaround.w;
    vec4 sk_FragCoord_Resolved = vec4(sk_FragCoord_Workaround.xyz * sk_FragCoord_InvW, sk_FragCoord_InvW);
    sk_FragCoord_Resolved.xy = floor(sk_FragCoord_Resolved.xy) + vec2(.5);
    return sk_Clockwise ? sk_FragCoord_Resolved : vec4(0.0);
}
vec4 func2_f4() {
    bool sk_Clockwise = gl_FrontFacing;
    if (u_skRTFlip.y < 0.0) {
        sk_Clockwise = !sk_Clockwise;
    }
    float sk_FragCoord_InvW = 1. / sk_FragCoord_Workaround.w;
    vec4 sk_FragCoord_Resolved = vec4(sk_FragCoord_Workaround.xyz * sk_FragCoord_InvW, sk_FragCoord_InvW);
    sk_FragCoord_Resolved.xy = floor(sk_FragCoord_Resolved.xy) + vec2(.5);
    return sk_Clockwise ? vec4(0.0) : sk_FragCoord_Resolved;
}
void main() {
    sk_FragColor = func1_f4() + func2_f4();
}
