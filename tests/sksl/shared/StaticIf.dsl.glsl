
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
vec4 main() {
    vec4 result = colorRed;
    {
        result = colorGreen;
    }
    return result;
}
