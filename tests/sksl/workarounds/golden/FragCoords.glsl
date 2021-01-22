#version 400
in vec4 sk_FragCoord_Workaround;
out vec4 sk_FragColor;
void main() {
    float sk_FragCoord_InvW = 1. / sk_FragCoord_Workaround.w;
    vec4 sk_FragCoord_Resolved = vec4(sk_FragCoord_Workaround.xyz * sk_FragCoord_InvW, sk_FragCoord_InvW);
    sk_FragCoord_Resolved.xy = floor(sk_FragCoord_Resolved.xy) + vec2(.5);
    sk_FragColor.xy = sk_FragCoord_Resolved.xy;
}
