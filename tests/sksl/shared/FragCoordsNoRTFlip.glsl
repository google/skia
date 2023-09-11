
out vec4 sk_FragColor;
void main() {
    vec4 sk_FragCoord = vec4(gl_FragCoord.x, gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    sk_FragColor.xy = sk_FragCoord.yx;
}
