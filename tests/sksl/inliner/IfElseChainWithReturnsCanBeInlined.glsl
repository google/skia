
out vec4 sk_FragColor;
uniform vec4 color;
vec4 branchy(vec4 c) {
    c *= 0.5;
    if (c.x > 0.0) return c.xxxx; else if (c.y > 0.0) return c.yyyy; else if (c.z > 0.0) return c.zzzz; else return c.wwww;
}
vec4 branchyAndBlocky(vec4 c) {
    {
        {
            if (c.x > 0.0) {
                vec4 d = c * 0.5;
                return d.xxxx;
            } else {
                {
                    {
                        if (c.x < 0.0) {
                            return c.wwww;
                        } else {
                            return c.yyyy;
                        }
                    }
                }
            }
        }
    }
}
void main() {
    sk_FragColor = branchy(color) * branchyAndBlocky(color);
}
