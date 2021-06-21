
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform float c;
uniform vec4 d;
uniform vec4 e;
void main() {
    sk_FragColor.x = refract(a, b, c);
    sk_FragColor = refract(d, e, c);
    sk_FragColor.xy = vec2(0.5, -0.86602538824081421);
    sk_FragColor.xyz = vec3(0.5, 0.0, -0.86602538824081421);
    sk_FragColor = vec4(0.5, 0.0, 0.0, -0.86602538824081421);
}
