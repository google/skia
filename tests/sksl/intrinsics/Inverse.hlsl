cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

// Returns the determinant of a 2x2 matrix.
float spvDet2x2(float a1, float a2, float b1, float b2)
{
    return a1 * b2 - b1 * a2;
}

// Returns the inverse of a matrix, by using the algorithm of calculating the classical
// adjoint and dividing by the determinant. The contents of the matrix are changed.
float3x3 spvInverse(float3x3 m)
{
    float3x3 adj;	// The adjoint matrix (inverse after dividing by determinant)

    // Create the transpose of the cofactors, as the classical adjoint of the matrix.
    adj[0][0] =  spvDet2x2(m[1][1], m[1][2], m[2][1], m[2][2]);
    adj[0][1] = -spvDet2x2(m[0][1], m[0][2], m[2][1], m[2][2]);
    adj[0][2] =  spvDet2x2(m[0][1], m[0][2], m[1][1], m[1][2]);

    adj[1][0] = -spvDet2x2(m[1][0], m[1][2], m[2][0], m[2][2]);
    adj[1][1] =  spvDet2x2(m[0][0], m[0][2], m[2][0], m[2][2]);
    adj[1][2] = -spvDet2x2(m[0][0], m[0][2], m[1][0], m[1][2]);

    adj[2][0] =  spvDet2x2(m[1][0], m[1][1], m[2][0], m[2][1]);
    adj[2][1] = -spvDet2x2(m[0][0], m[0][1], m[2][0], m[2][1]);
    adj[2][2] =  spvDet2x2(m[0][0], m[0][1], m[1][0], m[1][1]);

    // Calculate the determinant as a combination of the cofactors of the first row.
    float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]);

    // Divide the classical adjoint matrix by the determinant.
    // If determinant is zero, matrix is not invertable, so leave it unchanged.
    return (det != 0.0f) ? (adj * (1.0f / det)) : m;
}

float4 main(float2 _24)
{
    float2x2 inv2x2 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
    float3x3 inv3x3 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
    float4x4 inv4x4 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
    float2x2 _68 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
    float2 _71 = _68[0];
    float2 _75 = _68[1];
    bool _102 = false;
    if (all(bool2(_71.x == inv2x2[0].x, _71.y == inv2x2[0].y)) && all(bool2(_75.x == inv2x2[1].x, _75.y == inv2x2[1].y)))
    {
        float3x3 _85 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
        float3 _88 = _85[0];
        float3 _92 = _85[1];
        float3 _97 = _85[2];
        _102 = (all(bool3(_88.x == inv3x3[0].x, _88.y == inv3x3[0].y, _88.z == inv3x3[0].z)) && all(bool3(_92.x == inv3x3[1].x, _92.y == inv3x3[1].y, _92.z == inv3x3[1].z))) && all(bool3(_97.x == inv3x3[2].x, _97.y == inv3x3[2].y, _97.z == inv3x3[2].z));
    }
    else
    {
        _102 = false;
    }
    bool _131 = false;
    if (_102)
    {
        float4x4 _109 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
        float4 _112 = _109[0];
        float4 _116 = _109[1];
        float4 _121 = _109[2];
        float4 _126 = _109[3];
        _131 = ((all(bool4(_112.x == inv4x4[0].x, _112.y == inv4x4[0].y, _112.z == inv4x4[0].z, _112.w == inv4x4[0].w)) && all(bool4(_116.x == inv4x4[1].x, _116.y == inv4x4[1].y, _116.z == inv4x4[1].z, _116.w == inv4x4[1].w))) && all(bool4(_121.x == inv4x4[2].x, _121.y == inv4x4[2].y, _121.z == inv4x4[2].z, _121.w == inv4x4[2].w))) && all(bool4(_126.x == inv4x4[3].x, _126.y == inv4x4[3].y, _126.z == inv4x4[3].z, _126.w == inv4x4[3].w));
    }
    else
    {
        _131 = false;
    }
    bool _158 = false;
    if (_131)
    {
        float3x3 _134 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)));
        float3 _144 = _134[0];
        float3 _148 = _134[1];
        float3 _153 = _134[2];
        _158 = (any(bool3(_144.x != inv3x3[0].x, _144.y != inv3x3[0].y, _144.z != inv3x3[0].z)) || any(bool3(_148.x != inv3x3[1].x, _148.y != inv3x3[1].y, _148.z != inv3x3[1].z))) || any(bool3(_153.x != inv3x3[2].x, _153.y != inv3x3[2].y, _153.z != inv3x3[2].z));
    }
    else
    {
        _158 = false;
    }
    float4 _159 = 0.0f.xxxx;
    if (_158)
    {
        _159 = _10_colorGreen;
    }
    else
    {
        _159 = _10_colorRed;
    }
    return _159;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
