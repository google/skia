
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D t;
void main() {
    vec4 c = textureLod(t, vec2(0.0), 0.0);
    sk_FragColor = c * textureLod(t, vec3(1.0), 0.0);
}
