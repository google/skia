layout(location=0) out float defaultVarying;
layout(location=1) out noperspective float linearVarying;
layout(location=2) out flat float flatVarying;

void main() {
    defaultVarying = 1.0;
    linearVarying = 2.0;
    flatVarying = 3.0;
}
