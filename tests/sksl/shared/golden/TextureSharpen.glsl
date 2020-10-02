
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler1D one;
layout (binding = 1) uniform sampler2D two;
void main() {
    vec4 a = texture(one, 0.0, -0.5);
    vec4 b = texture(two, vec2(0.0), -0.5);
    vec4 c = textureProj(one, vec2(0.0), -0.5);
    vec4 d = textureProj(two, vec3(0.0), -0.5);
    sk_FragColor = vec4(a.x, b.x, c.x, d.x);
}
