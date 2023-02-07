
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
vec4 main() {
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    return vec4(sk_FragCoord.yx, 1.0, 1.0);
}
