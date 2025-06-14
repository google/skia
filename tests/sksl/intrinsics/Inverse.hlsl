cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
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

float4 main(float2 _25)
{
    float2x2 matrix2x2 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 inv2x2 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
    float3x3 inv3x3 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
    float4x4 inv4x4 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
    float Zero = _11_colorGreen.z;
    bool _96 = false;
    if (all(bool2(float2(-2.0f, 1.0f).x == float2(-2.0f, 1.0f).x, float2(-2.0f, 1.0f).y == float2(-2.0f, 1.0f).y)) && all(bool2(float2(1.5f, -0.5f).x == float2(1.5f, -0.5f).x, float2(1.5f, -0.5f).y == float2(1.5f, -0.5f).y)))
    {
        _96 = (all(bool3(float3(-24.0f, 18.0f, 5.0f).x == float3(-24.0f, 18.0f, 5.0f).x, float3(-24.0f, 18.0f, 5.0f).y == float3(-24.0f, 18.0f, 5.0f).y, float3(-24.0f, 18.0f, 5.0f).z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(float3(20.0f, -15.0f, -4.0f).x == float3(20.0f, -15.0f, -4.0f).x, float3(20.0f, -15.0f, -4.0f).y == float3(20.0f, -15.0f, -4.0f).y, float3(20.0f, -15.0f, -4.0f).z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(float3(-5.0f, 4.0f, 1.0f).x == float3(-5.0f, 4.0f, 1.0f).x, float3(-5.0f, 4.0f, 1.0f).y == float3(-5.0f, 4.0f, 1.0f).y, float3(-5.0f, 4.0f, 1.0f).z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _96 = false;
    }
    bool _111 = false;
    if (_96)
    {
        _111 = ((all(bool4(float4(-2.0f, -0.5f, 1.0f, 0.5f).x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, float4(-2.0f, -0.5f, 1.0f, 0.5f).y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, float4(-2.0f, -0.5f, 1.0f, 0.5f).z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, float4(-2.0f, -0.5f, 1.0f, 0.5f).w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(float4(1.0f, 0.5f, 0.0f, -0.5f).x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, float4(1.0f, 0.5f, 0.0f, -0.5f).y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, float4(1.0f, 0.5f, 0.0f, -0.5f).z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, float4(1.0f, 0.5f, 0.0f, -0.5f).w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(float4(-8.0f, -1.0f, 2.0f, 2.0f).x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, float4(-8.0f, -1.0f, 2.0f, 2.0f).y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, float4(-8.0f, -1.0f, 2.0f, 2.0f).z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, float4(-8.0f, -1.0f, 2.0f, 2.0f).w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(float4(3.0f, 0.5f, -1.0f, -0.5f).x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, float4(3.0f, 0.5f, -1.0f, -0.5f).y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, float4(3.0f, 0.5f, -1.0f, -0.5f).z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, float4(3.0f, 0.5f, -1.0f, -0.5f).w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _111 = false;
    }
    bool _134 = false;
    if (_111)
    {
        float3x3 _114 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)));
        float3 _123 = _114[0];
        float3 _126 = _114[1];
        float3 _130 = _114[2];
        _134 = (any(bool3(_123.x != float3(-24.0f, 18.0f, 5.0f).x, _123.y != float3(-24.0f, 18.0f, 5.0f).y, _123.z != float3(-24.0f, 18.0f, 5.0f).z)) || any(bool3(_126.x != float3(20.0f, -15.0f, -4.0f).x, _126.y != float3(20.0f, -15.0f, -4.0f).y, _126.z != float3(20.0f, -15.0f, -4.0f).z))) || any(bool3(_130.x != float3(-5.0f, 4.0f, 1.0f).x, _130.y != float3(-5.0f, 4.0f, 1.0f).y, _130.z != float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _134 = false;
    }
    bool _150 = false;
    if (_134)
    {
        float2 _138 = _11_colorGreen.z.xx;
        float2x2 _137 = spvInverse(float2x2(float2(1.0f, 2.0f) + _138, float2(3.0f, 4.0f) + _138));
        float2 _143 = _137[0];
        float2 _146 = _137[1];
        _150 = all(bool2(_143.x == float2(-2.0f, 1.0f).x, _143.y == float2(-2.0f, 1.0f).y)) && all(bool2(_146.x == float2(1.5f, -0.5f).x, _146.y == float2(1.5f, -0.5f).y));
    }
    else
    {
        _150 = false;
    }
    bool _174 = false;
    if (_150)
    {
        float3 _157 = _11_colorGreen.z.xxx;
        float3x3 _153 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f) + _157, float3(0.0f, 1.0f, 4.0f) + _157, float3(5.0f, 6.0f, 0.0f) + _157));
        float3 _163 = _153[0];
        float3 _166 = _153[1];
        float3 _170 = _153[2];
        _174 = (all(bool3(_163.x == float3(-24.0f, 18.0f, 5.0f).x, _163.y == float3(-24.0f, 18.0f, 5.0f).y, _163.z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(_166.x == float3(20.0f, -15.0f, -4.0f).x, _166.y == float3(20.0f, -15.0f, -4.0f).y, _166.z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(_170.x == float3(-5.0f, 4.0f, 1.0f).x, _170.y == float3(-5.0f, 4.0f, 1.0f).y, _170.z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _174 = false;
    }
    bool _205 = false;
    if (_174)
    {
        float4 _183 = _11_colorGreen.z.xxxx;
        float4x4 _177 = spvInverse(float4x4(float4(1.0f, 0.0f, 0.0f, 1.0f) + _183, float4(0.0f, 2.0f, 1.0f, 2.0f) + _183, float4(2.0f, 1.0f, 0.0f, 1.0f) + _183, float4(2.0f, 0.0f, 1.0f, 4.0f) + _183));
        float4 _190 = _177[0];
        float4 _193 = _177[1];
        float4 _197 = _177[2];
        float4 _201 = _177[3];
        _205 = ((all(bool4(_190.x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, _190.y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, _190.z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, _190.w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(_193.x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, _193.y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, _193.z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, _193.w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(_197.x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, _197.y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, _197.z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, _197.w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(_201.x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, _201.y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, _201.z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, _201.w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _205 = false;
    }
    float4 _206 = 0.0f.xxxx;
    if (_205)
    {
        _206 = _11_colorGreen;
    }
    else
    {
        _206 = _11_colorRed;
    }
    return _206;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
