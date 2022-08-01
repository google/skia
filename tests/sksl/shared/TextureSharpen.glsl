
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D s;
void main() {
    vec4 a = texture(s, vec2(0.0), -0.475);
    vec4 b = textureProj(s, vec3(0.0), -0.475);
    sk_FragColor = vec4(a.xy, b.xy);
}
