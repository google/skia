
out vec4 sk_FragColor;
layout (binding = 0) uniform readonlyTexture2D t;
void main() {
    uvec2 dims = textureSize(t);
    sk_FragColor = vec4(vec2(dims), vec2(dims));
}
