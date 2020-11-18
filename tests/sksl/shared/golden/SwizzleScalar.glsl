
out vec4 sk_FragColor;
void main() {
    float x = sqrt(4.0);
    sk_FragColor = vec4(vec2(x), 0.0, 1.0);
    sk_FragColor = vec4(vec2(sqrt(4.0)), 0.0, 1.0);
    sk_FragColor = vec4(0.0, sqrt(4.0), 0.0, 1.0);
    sk_FragColor = vec3(vec2(sqrt(4.0)), 0.0).zxzy;
}
