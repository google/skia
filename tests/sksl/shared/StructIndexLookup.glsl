
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct InnerLUT {
    vec3 values;
};
struct OuterLUT {
    InnerLUT inner[3];
};
struct Root {
    OuterLUT outer[3];
};
vec4 main() {
    Root data;
    data.outer[0].inner[0].values = vec3(1.0, 10.0, 100.0);
    data.outer[0].inner[1].values = vec3(2.0, 20.0, 200.0);
    data.outer[0].inner[2].values = vec3(3.0, 30.0, 300.0);
    data.outer[1].inner[0].values = vec3(4.0, 40.0, 400.0);
    data.outer[1].inner[1].values = vec3(5.0, 50.0, 500.0);
    data.outer[1].inner[2].values = vec3(6.0, 60.0, 600.0);
    data.outer[2].inner[0].values = vec3(7.0, 70.0, 700.0);
    data.outer[2].inner[1].values = vec3(8.0, 80.0, 800.0);
    data.outer[2].inner[2].values = vec3(9.0, 90.0, 900.0);
    vec3 expected = vec3(0.0);
    for (int i = 0;i < 3; ++i) {
        for (int j = 0;j < 3; ++j) {
            expected += vec3(1.0, 10.0, 100.0);
            if (data.outer[i].inner[j].values != expected) {
                return colorRed;
            }
            for (int k = 0;k < 3; ++k) {
                if (data.outer[i].inner[j].values[k] != expected[k]) {
                    return colorRed;
                }
            }
        }
    }
    return colorGreen;
}
