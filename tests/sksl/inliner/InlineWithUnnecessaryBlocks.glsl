
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blocky(vec4 c) {
    {
        return c;
    }
}
void main() {
    sk_FragColor = blocky(color);
}
