cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

// Returns the inverse of a matrix, by using the algorithm of calculating the classical
// adjoint and dividing by the determinant. The contents of the matrix are changed.
float2x2 spvInverse(float2x2 m)
{
    float2x2 adj;	// The adjoint matrix (inverse after dividing by determinant)

    // Create the transpose of the cofactors, as the classical adjoint of the matrix.
    adj[0][0] =  m[1][1];
    adj[0][1] = -m[0][1];

    adj[1][0] = -m[1][0];
    adj[1][1] =  m[0][0];

    // Calculate the determinant as a combination of the cofactors of the first row.
    float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]);

    // Divide the classical adjoint matrix by the determinant.
    // If determinant is zero, matrix is not invertable, so leave it unchanged.
    return (det != 0.0f) ? (adj * (1.0f / det)) : m;
}

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

// Returns the determinant of a 3x3 matrix.
float spvDet3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
    return a1 * spvDet2x2(b2, b3, c2, c3) - b1 * spvDet2x2(a2, a3, c2, c3) + c1 * spvDet2x2(a2, a3, b2, b3);
}

// Returns the inverse of a matrix, by using the algorithm of calculating the classical
// adjoint and dividing by the determinant. The contents of the matrix are changed.
float4x4 spvInverse(float4x4 m)
{
    float4x4 adj;	// The adjoint matrix (inverse after dividing by determinant)

    // Create the transpose of the cofactors, as the classical adjoint of the matrix.
    adj[0][0] =  spvDet3x3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
    adj[0][1] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
    adj[0][2] =  spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[3][1], m[3][2], m[3][3]);
    adj[0][3] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3]);

    adj[1][0] = -spvDet3x3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
    adj[1][1] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
    adj[1][2] = -spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[3][0], m[3][2], m[3][3]);
    adj[1][3] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3]);

    adj[2][0] =  spvDet3x3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
    adj[2][1] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
    adj[2][2] =  spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], m[3][3]);
    adj[2][3] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3]);

    adj[3][0] = -spvDet3x3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
    adj[3][1] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
    adj[3][2] = -spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[3][0], m[3][1], m[3][2]);
    adj[3][3] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);

    // Calculate the determinant as a combination of the cofactors of the first row.
    float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]) + (adj[0][3] * m[3][0]);

    // Divide the classical adjoint matrix by the determinant.
    // If determinant is zero, matrix is not invertable, so leave it unchanged.
    return (det != 0.0f) ? (adj * (1.0f / det)) : m;
}

float4 main(float2 _21)
{
    float2x2 matrix2x2 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 inv2x2 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
    float3x3 inv3x3 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
    float4x4 inv4x4 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
    float Zero = _7_colorGreen.z;
    bool _93 = false;
    if (all(bool2(float2(-2.0f, 1.0f).x == float2(-2.0f, 1.0f).x, float2(-2.0f, 1.0f).y == float2(-2.0f, 1.0f).y)) && all(bool2(float2(1.5f, -0.5f).x == float2(1.5f, -0.5f).x, float2(1.5f, -0.5f).y == float2(1.5f, -0.5f).y)))
    {
        _93 = (all(bool3(float3(-24.0f, 18.0f, 5.0f).x == float3(-24.0f, 18.0f, 5.0f).x, float3(-24.0f, 18.0f, 5.0f).y == float3(-24.0f, 18.0f, 5.0f).y, float3(-24.0f, 18.0f, 5.0f).z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(float3(20.0f, -15.0f, -4.0f).x == float3(20.0f, -15.0f, -4.0f).x, float3(20.0f, -15.0f, -4.0f).y == float3(20.0f, -15.0f, -4.0f).y, float3(20.0f, -15.0f, -4.0f).z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(float3(-5.0f, 4.0f, 1.0f).x == float3(-5.0f, 4.0f, 1.0f).x, float3(-5.0f, 4.0f, 1.0f).y == float3(-5.0f, 4.0f, 1.0f).y, float3(-5.0f, 4.0f, 1.0f).z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _93 = false;
    }
    bool _108 = false;
    if (_93)
    {
        _108 = ((all(bool4(float4(-2.0f, -0.5f, 1.0f, 0.5f).x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, float4(-2.0f, -0.5f, 1.0f, 0.5f).y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, float4(-2.0f, -0.5f, 1.0f, 0.5f).z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, float4(-2.0f, -0.5f, 1.0f, 0.5f).w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(float4(1.0f, 0.5f, 0.0f, -0.5f).x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, float4(1.0f, 0.5f, 0.0f, -0.5f).y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, float4(1.0f, 0.5f, 0.0f, -0.5f).z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, float4(1.0f, 0.5f, 0.0f, -0.5f).w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(float4(-8.0f, -1.0f, 2.0f, 2.0f).x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, float4(-8.0f, -1.0f, 2.0f, 2.0f).y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, float4(-8.0f, -1.0f, 2.0f, 2.0f).z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, float4(-8.0f, -1.0f, 2.0f, 2.0f).w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(float4(3.0f, 0.5f, -1.0f, -0.5f).x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, float4(3.0f, 0.5f, -1.0f, -0.5f).y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, float4(3.0f, 0.5f, -1.0f, -0.5f).z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, float4(3.0f, 0.5f, -1.0f, -0.5f).w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _108 = false;
    }
    bool _131 = false;
    if (_108)
    {
        float3x3 _111 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)));
        float3 _120 = _111[0];
        float3 _123 = _111[1];
        float3 _127 = _111[2];
        _131 = (any(bool3(_120.x != float3(-24.0f, 18.0f, 5.0f).x, _120.y != float3(-24.0f, 18.0f, 5.0f).y, _120.z != float3(-24.0f, 18.0f, 5.0f).z)) || any(bool3(_123.x != float3(20.0f, -15.0f, -4.0f).x, _123.y != float3(20.0f, -15.0f, -4.0f).y, _123.z != float3(20.0f, -15.0f, -4.0f).z))) || any(bool3(_127.x != float3(-5.0f, 4.0f, 1.0f).x, _127.y != float3(-5.0f, 4.0f, 1.0f).y, _127.z != float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _131 = false;
    }
    bool _147 = false;
    if (_131)
    {
        float2 _135 = _7_colorGreen.z.xx;
        float2x2 _134 = spvInverse(float2x2(float2(1.0f, 2.0f) + _135, float2(3.0f, 4.0f) + _135));
        float2 _140 = _134[0];
        float2 _143 = _134[1];
        _147 = all(bool2(_140.x == float2(-2.0f, 1.0f).x, _140.y == float2(-2.0f, 1.0f).y)) && all(bool2(_143.x == float2(1.5f, -0.5f).x, _143.y == float2(1.5f, -0.5f).y));
    }
    else
    {
        _147 = false;
    }
    bool _171 = false;
    if (_147)
    {
        float3 _154 = _7_colorGreen.z.xxx;
        float3x3 _150 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f) + _154, float3(0.0f, 1.0f, 4.0f) + _154, float3(5.0f, 6.0f, 0.0f) + _154));
        float3 _160 = _150[0];
        float3 _163 = _150[1];
        float3 _167 = _150[2];
        _171 = (all(bool3(_160.x == float3(-24.0f, 18.0f, 5.0f).x, _160.y == float3(-24.0f, 18.0f, 5.0f).y, _160.z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(_163.x == float3(20.0f, -15.0f, -4.0f).x, _163.y == float3(20.0f, -15.0f, -4.0f).y, _163.z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(_167.x == float3(-5.0f, 4.0f, 1.0f).x, _167.y == float3(-5.0f, 4.0f, 1.0f).y, _167.z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _171 = false;
    }
    bool _202 = false;
    if (_171)
    {
        float4 _180 = _7_colorGreen.z.xxxx;
        float4x4 _174 = spvInverse(float4x4(float4(1.0f, 0.0f, 0.0f, 1.0f) + _180, float4(0.0f, 2.0f, 1.0f, 2.0f) + _180, float4(2.0f, 1.0f, 0.0f, 1.0f) + _180, float4(2.0f, 0.0f, 1.0f, 4.0f) + _180));
        float4 _187 = _174[0];
        float4 _190 = _174[1];
        float4 _194 = _174[2];
        float4 _198 = _174[3];
        _202 = ((all(bool4(_187.x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, _187.y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, _187.z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, _187.w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(_190.x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, _190.y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, _190.z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, _190.w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(_194.x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, _194.y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, _194.z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, _194.w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(_198.x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, _198.y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, _198.z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, _198.w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _202 = false;
    }
    float4 _203 = 0.0f.xxxx;
    if (_202)
    {
        _203 = _7_colorGreen;
    }
    else
    {
        _203 = _7_colorRed;
    }
    return _203;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
