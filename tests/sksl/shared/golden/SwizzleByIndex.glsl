
out vec4 sk_FragColor;
void main() {
    vec4 v = vec4(sqrt(1.0));
    float x = v.x;
    float y = v.y;
    float z = v.z;
    float w = v.w;
    sk_FragColor = vec4(x, y, z, w);
    v = vec4(2.0);
    x = 2.0;
    y = 2.0;
    z = 2.0;
    w = 2.0;
    sk_FragColor = vec4(2.0, 2.0, 2.0, 2.0);
}
