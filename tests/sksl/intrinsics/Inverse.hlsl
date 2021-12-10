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
    bool _83 = false;
    if (all(bool2(float2(-2.0f, 1.0f).x == float2(-2.0f, 1.0f).x, float2(-2.0f, 1.0f).y == float2(-2.0f, 1.0f).y)) && all(bool2(float2(1.5f, -0.5f).x == float2(1.5f, -0.5f).x, float2(1.5f, -0.5f).y == float2(1.5f, -0.5f).y)))
    {
        _83 = (all(bool3(float3(-24.0f, 18.0f, 5.0f).x == float3(-24.0f, 18.0f, 5.0f).x, float3(-24.0f, 18.0f, 5.0f).y == float3(-24.0f, 18.0f, 5.0f).y, float3(-24.0f, 18.0f, 5.0f).z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(float3(20.0f, -15.0f, -4.0f).x == float3(20.0f, -15.0f, -4.0f).x, float3(20.0f, -15.0f, -4.0f).y == float3(20.0f, -15.0f, -4.0f).y, float3(20.0f, -15.0f, -4.0f).z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(float3(-5.0f, 4.0f, 1.0f).x == float3(-5.0f, 4.0f, 1.0f).x, float3(-5.0f, 4.0f, 1.0f).y == float3(-5.0f, 4.0f, 1.0f).y, float3(-5.0f, 4.0f, 1.0f).z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _83 = false;
    }
    bool _98 = false;
    if (_83)
    {
        _98 = ((all(bool4(float4(-2.0f, -0.5f, 1.0f, 0.5f).x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, float4(-2.0f, -0.5f, 1.0f, 0.5f).y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, float4(-2.0f, -0.5f, 1.0f, 0.5f).z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, float4(-2.0f, -0.5f, 1.0f, 0.5f).w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(float4(1.0f, 0.5f, 0.0f, -0.5f).x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, float4(1.0f, 0.5f, 0.0f, -0.5f).y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, float4(1.0f, 0.5f, 0.0f, -0.5f).z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, float4(1.0f, 0.5f, 0.0f, -0.5f).w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(float4(-8.0f, -1.0f, 2.0f, 2.0f).x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, float4(-8.0f, -1.0f, 2.0f, 2.0f).y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, float4(-8.0f, -1.0f, 2.0f, 2.0f).z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, float4(-8.0f, -1.0f, 2.0f, 2.0f).w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(float4(3.0f, 0.5f, -1.0f, -0.5f).x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, float4(3.0f, 0.5f, -1.0f, -0.5f).y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, float4(3.0f, 0.5f, -1.0f, -0.5f).z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, float4(3.0f, 0.5f, -1.0f, -0.5f).w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _98 = false;
    }
    bool _121 = false;
    if (_98)
    {
        float3x3 _101 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)));
        float3 _110 = _101[0];
        float3 _113 = _101[1];
        float3 _117 = _101[2];
        _121 = (any(bool3(_110.x != float3(-24.0f, 18.0f, 5.0f).x, _110.y != float3(-24.0f, 18.0f, 5.0f).y, _110.z != float3(-24.0f, 18.0f, 5.0f).z)) || any(bool3(_113.x != float3(20.0f, -15.0f, -4.0f).x, _113.y != float3(20.0f, -15.0f, -4.0f).y, _113.z != float3(20.0f, -15.0f, -4.0f).z))) || any(bool3(_117.x != float3(-5.0f, 4.0f, 1.0f).x, _117.y != float3(-5.0f, 4.0f, 1.0f).y, _117.z != float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _121 = false;
    }
    float4 _122 = 0.0f.xxxx;
    if (_121)
    {
        _122 = _10_colorGreen;
    }
    else
    {
        _122 = _10_colorRed;
    }
    return _122;
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
