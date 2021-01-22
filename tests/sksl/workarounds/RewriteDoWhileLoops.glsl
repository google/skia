#version 400
out vec4 sk_FragColor;
void main() {
    int i = 0;
    bool _tmpLoopSeenOnce0 = false;
    while (true) {
        if (_tmpLoopSeenOnce0) {
            if (!(i < 10)) {
                break;
            }
        }
        _tmpLoopSeenOnce0 = true;
        {
            ++i;
            bool _tmpLoopSeenOnce1 = false;
            while (true) {
                if (_tmpLoopSeenOnce1) {
                    if (!true) {
                        break;
                    }
                }
                _tmpLoopSeenOnce1 = true;
                {
                    i++;
                }
            }
        }
    }
    sk_FragColor = vec4(float(i));
}
