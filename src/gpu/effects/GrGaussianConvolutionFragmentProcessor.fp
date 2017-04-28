in uniform sampler2D tex;
in int direction;
in int radius;
in float gaussianSigma;
in bool useBounds;
in uniform int bounds[2];
in int width = 2 * radius + 1;
in arrayCount = (width + 3) / 4;
in vec4 kernel[arrayCount];
uniform vec2 imageIncrement;

@body {
    static void fill_in_1D_guassian_kernel(float* kernel, int width, float gaussianSigma,
                                           int radius) {
        const float denom = 1.0f / (2.0f * gaussianSigma * gaussianSigma);

        float sum = 0.0f;
        for (int i = 0; i < width; ++i) {
            float x = static_cast<float>(i - radius);
            // Note that the constant term (1/(sqrt(2*pi*sigma^2)) of the Gaussian
            // is dropped here, since we renormalize the kernel below.
            kernel[i] = sk_float_exp(-x * x * denom);
            sum += kernel[i];
        }
        // Normalize the kernel
        float scale = 1.0f / sum;
        for (int i = 0; i < width; ++i) {
            kernel[i] *= scale;
        }
    }
}

void main() {
    sk_OutColor = vec4(0, 0, 0, 0);
    vec2 coord = sk_TransformedCoords2D[0] - radius * imageIncrement;

    @unroll
    for (int i = 0; i < width; i += 4) {
        if (direction == 0 && (coord.x >= bounds.x && coord.x <= bounds.y) ||
            direction == 1 && (coord.y >= bounds.x && coord.y <= bounds.y)) {
            sk_OutColor += texture(tex, coord) * kernel[i / 4].x;
        }
        coord += imageIncrement;
        if (direction == 0 && (coord.x >= bounds.x && coord.x <= bounds.y) ||
            direction == 1 && (coord.y >= bounds.x && coord.y <= bounds.y)) {
            sk_OutColor += texture(tex, coord) * kernel[i / 4].y;
        }
        coord += imageIncrement;
        if (direction == 0 && (coord.x >= bounds.x && coord.x <= bounds.y) ||
            direction == 1 && (coord.y >= bounds.x && coord.y <= bounds.y)) {
            sk_OutColor += texture(tex, coord) * kernel[i / 4].z;
        }
        coord += imageIncrement;
        if (direction == 0 && (coord.x >= bounds.x && coord.x <= bounds.y) ||
            direction == 1 && (coord.y >= bounds.x && coord.y <= bounds.y)) {
            sk_OutColor += texture(tex, coord) * kernel[i / 4].w;
        }
        coord += imageIncrement;
    }
    sk_OutColor *= sk_InColor;
