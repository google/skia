
out vec4 sk_FragColor;
void main() {
    vec4 v = vec4(sqrt(1.0));
    float x = v[0];
    float y = v[1];
    float z = v[2];
    float w = v[3];
    sk_FragColor = vec4(x, y, z, w);
}
