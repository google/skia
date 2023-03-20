
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D t;
void main() {
    vec4 c = texture(t, vec2(0.0));
    sk_FragColor = c * textureProj(t, vec3(1.0));
}
