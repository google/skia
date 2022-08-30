
out vec4 sk_FragColor;
uniform vec4 colorGreen;
float scalar_fff(float x, float y) {
    x = length(x);
    x = distance(x, y);
    x = dot(x, y);
    x = normalize(x);
    return x;
}
vec2 vector_f2f2f2(vec2 x, vec2 y) {
    x = vec2(length(x));
    x = vec2(distance(x, y));
    x = vec2(dot(x, y));
    x = normalize(x);
    return x;
}
vec4 main() {
    scalar_fff(1.0, 2.0);
    vector_f2f2f2(vec2(1.0, 2.0), vec2(3.0, 4.0));
    return colorGreen;
}
