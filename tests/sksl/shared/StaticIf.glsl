
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    vec4 result = colorRed;
    const float x = 5.0;
    const float y = 10.0;
    {
        result = colorGreen;
    }
    return result;
}
