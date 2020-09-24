
out vec4 sk_FragColor;
uniform sampler2D test2D;
uniform sampler2D test2DRect;
void main() {
    sk_FragColor = texture(test2D, vec2(0.5));
    sk_FragColor = texture(test2DRect, vec2(0.5));
    sk_FragColor = textureProj(test2DRect, vec3(0.5));
}
