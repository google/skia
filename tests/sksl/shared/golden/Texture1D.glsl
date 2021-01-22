
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler1D tex;
void main() {
    vec4 a = texture(tex, 0.0);
    vec4 b = textureProj(tex, vec2(0.0));
    sk_FragColor = vec4(a.xy, b.zw);
}
