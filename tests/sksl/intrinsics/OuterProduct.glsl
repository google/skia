
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec3 h3;
uniform vec4 h4;
uniform vec2 f2;
uniform vec3 f3;
uniform vec4 f4;
void main() {
    sk_FragColor = outerProduct(f2, f2)[1].xyyy;
    sk_FragColor = outerProduct(f3, f3)[2].xyzz;
    sk_FragColor = outerProduct(f4, f4)[3];
    sk_FragColor = outerProduct(f3, f2)[1].xyzz;
    sk_FragColor = outerProduct(f2, f3)[2].xyyy;
    sk_FragColor = outerProduct(f4, f2)[1];
    sk_FragColor = outerProduct(f2, f4)[3].xyyy;
    sk_FragColor = outerProduct(f4, f3)[2];
    sk_FragColor = outerProduct(f3, f4)[3].xyzz;
    sk_FragColor = outerProduct(h2, h2)[1].xyyy;
    sk_FragColor = outerProduct(h3, h3)[2].xyzz;
    sk_FragColor = outerProduct(h4, h4)[3];
    sk_FragColor = outerProduct(h3, h2)[1].xyzz;
    sk_FragColor = outerProduct(h2, h3)[2].xyyy;
    sk_FragColor = outerProduct(h4, h2)[1];
    sk_FragColor = outerProduct(h2, h4)[3].xyyy;
    sk_FragColor = outerProduct(h4, h3)[2];
    sk_FragColor = outerProduct(h3, h4)[3].xyzz;
}
