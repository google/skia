layout(location=0) in float defaultVarying;
layout(location=1) in noperspective float linearVarying;
layout(location=2) in flat float flatVarying;

void main() {
    sk_FragColor = half4(defaultVarying, linearVarying, flatVarying, 1.0);
}
