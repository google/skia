
out vec4 sk_FragColor;
struct SomeData {
    vec4 a;
    vec2 b;
};
layout (binding = 0, set = 0) readonly buffer storageBuffer {
    uint offset;
    SomeData[] inputData;
};
layout (binding = 1, set = 0) buffer outputBuffer {
    SomeData[] outputData;
};
layout (location = 2) flat in int bufferIndex;
vec4 main() {
    outputData[offset] = inputData[offset];
    return inputData[bufferIndex].a * inputData[bufferIndex].b.x;
}
