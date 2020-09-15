
out vec4 sk_FragColor;
void main() {
    sk_FragColor = vec4(vec2[2](vec2(1.0), vec2(2.0))[0], vec2[2](vec2(3.0), vec2(4.0))[1]);
}
