#version 400
out vec4 sk_FragColor;
void main() {
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTHeight - gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    sk_FragColor.xy = sk_FragCoord.xy;
}
