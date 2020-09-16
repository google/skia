
out vec4 sk_FragColor;
uniform sampler1D one;
uniform sampler2D two;
void main() {
    vec4 a = texture(one, 0.0);
    vec4 b = texture(two, vec2(0.0));
    vec4 c = textureProj(one, vec2(0.0));
    vec4 d = textureProj(two, vec3(0.0));
    sk_FragColor = vec4(a.x, b.x, c.x, d.x);
}
