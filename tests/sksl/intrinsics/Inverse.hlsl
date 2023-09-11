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

float4 main(float2 _24)
{
    float2x2 matrix2x2 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2x2 inv2x2 = float2x2(float2(-2.0f, 1.0f), float2(1.5f, -0.5f));
    float3x3 inv3x3 = float3x3(float3(-24.0f, 18.0f, 5.0f), float3(20.0f, -15.0f, -4.0f), float3(-5.0f, 4.0f, 1.0f));
    float4x4 inv4x4 = float4x4(float4(-2.0f, -0.5f, 1.0f, 0.5f), float4(1.0f, 0.5f, 0.0f, -0.5f), float4(-8.0f, -1.0f, 2.0f, 2.0f), float4(3.0f, 0.5f, -1.0f, -0.5f));
    float Zero = _10_colorGreen.z;
    bool _95 = false;
    if (all(bool2(float2(-2.0f, 1.0f).x == float2(-2.0f, 1.0f).x, float2(-2.0f, 1.0f).y == float2(-2.0f, 1.0f).y)) && all(bool2(float2(1.5f, -0.5f).x == float2(1.5f, -0.5f).x, float2(1.5f, -0.5f).y == float2(1.5f, -0.5f).y)))
    {
        _95 = (all(bool3(float3(-24.0f, 18.0f, 5.0f).x == float3(-24.0f, 18.0f, 5.0f).x, float3(-24.0f, 18.0f, 5.0f).y == float3(-24.0f, 18.0f, 5.0f).y, float3(-24.0f, 18.0f, 5.0f).z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(float3(20.0f, -15.0f, -4.0f).x == float3(20.0f, -15.0f, -4.0f).x, float3(20.0f, -15.0f, -4.0f).y == float3(20.0f, -15.0f, -4.0f).y, float3(20.0f, -15.0f, -4.0f).z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(float3(-5.0f, 4.0f, 1.0f).x == float3(-5.0f, 4.0f, 1.0f).x, float3(-5.0f, 4.0f, 1.0f).y == float3(-5.0f, 4.0f, 1.0f).y, float3(-5.0f, 4.0f, 1.0f).z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _95 = false;
    }
    bool _110 = false;
    if (_95)
    {
        _110 = ((all(bool4(float4(-2.0f, -0.5f, 1.0f, 0.5f).x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, float4(-2.0f, -0.5f, 1.0f, 0.5f).y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, float4(-2.0f, -0.5f, 1.0f, 0.5f).z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, float4(-2.0f, -0.5f, 1.0f, 0.5f).w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(float4(1.0f, 0.5f, 0.0f, -0.5f).x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, float4(1.0f, 0.5f, 0.0f, -0.5f).y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, float4(1.0f, 0.5f, 0.0f, -0.5f).z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, float4(1.0f, 0.5f, 0.0f, -0.5f).w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(float4(-8.0f, -1.0f, 2.0f, 2.0f).x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, float4(-8.0f, -1.0f, 2.0f, 2.0f).y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, float4(-8.0f, -1.0f, 2.0f, 2.0f).z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, float4(-8.0f, -1.0f, 2.0f, 2.0f).w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(float4(3.0f, 0.5f, -1.0f, -0.5f).x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, float4(3.0f, 0.5f, -1.0f, -0.5f).y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, float4(3.0f, 0.5f, -1.0f, -0.5f).z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, float4(3.0f, 0.5f, -1.0f, -0.5f).w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _110 = false;
    }
    bool _133 = false;
    if (_110)
    {
        float3x3 _113 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)));
        float3 _122 = _113[0];
        float3 _125 = _113[1];
        float3 _129 = _113[2];
        _133 = (any(bool3(_122.x != float3(-24.0f, 18.0f, 5.0f).x, _122.y != float3(-24.0f, 18.0f, 5.0f).y, _122.z != float3(-24.0f, 18.0f, 5.0f).z)) || any(bool3(_125.x != float3(20.0f, -15.0f, -4.0f).x, _125.y != float3(20.0f, -15.0f, -4.0f).y, _125.z != float3(20.0f, -15.0f, -4.0f).z))) || any(bool3(_129.x != float3(-5.0f, 4.0f, 1.0f).x, _129.y != float3(-5.0f, 4.0f, 1.0f).y, _129.z != float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _133 = false;
    }
    bool _149 = false;
    if (_133)
    {
        float2 _137 = _10_colorGreen.z.xx;
        float2x2 _136 = spvInverse(float2x2(float2(1.0f, 2.0f) + _137, float2(3.0f, 4.0f) + _137));
        float2 _142 = _136[0];
        float2 _145 = _136[1];
        _149 = all(bool2(_142.x == float2(-2.0f, 1.0f).x, _142.y == float2(-2.0f, 1.0f).y)) && all(bool2(_145.x == float2(1.5f, -0.5f).x, _145.y == float2(1.5f, -0.5f).y));
    }
    else
    {
        _149 = false;
    }
    bool _173 = false;
    if (_149)
    {
        float3 _156 = _10_colorGreen.z.xxx;
        float3x3 _152 = spvInverse(float3x3(float3(1.0f, 2.0f, 3.0f) + _156, float3(0.0f, 1.0f, 4.0f) + _156, float3(5.0f, 6.0f, 0.0f) + _156));
        float3 _162 = _152[0];
        float3 _165 = _152[1];
        float3 _169 = _152[2];
        _173 = (all(bool3(_162.x == float3(-24.0f, 18.0f, 5.0f).x, _162.y == float3(-24.0f, 18.0f, 5.0f).y, _162.z == float3(-24.0f, 18.0f, 5.0f).z)) && all(bool3(_165.x == float3(20.0f, -15.0f, -4.0f).x, _165.y == float3(20.0f, -15.0f, -4.0f).y, _165.z == float3(20.0f, -15.0f, -4.0f).z))) && all(bool3(_169.x == float3(-5.0f, 4.0f, 1.0f).x, _169.y == float3(-5.0f, 4.0f, 1.0f).y, _169.z == float3(-5.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _173 = false;
    }
    bool _204 = false;
    if (_173)
    {
        float4 _182 = _10_colorGreen.z.xxxx;
        float4x4 _176 = spvInverse(float4x4(float4(1.0f, 0.0f, 0.0f, 1.0f) + _182, float4(0.0f, 2.0f, 1.0f, 2.0f) + _182, float4(2.0f, 1.0f, 0.0f, 1.0f) + _182, float4(2.0f, 0.0f, 1.0f, 4.0f) + _182));
        float4 _189 = _176[0];
        float4 _192 = _176[1];
        float4 _196 = _176[2];
        float4 _200 = _176[3];
        _204 = ((all(bool4(_189.x == float4(-2.0f, -0.5f, 1.0f, 0.5f).x, _189.y == float4(-2.0f, -0.5f, 1.0f, 0.5f).y, _189.z == float4(-2.0f, -0.5f, 1.0f, 0.5f).z, _189.w == float4(-2.0f, -0.5f, 1.0f, 0.5f).w)) && all(bool4(_192.x == float4(1.0f, 0.5f, 0.0f, -0.5f).x, _192.y == float4(1.0f, 0.5f, 0.0f, -0.5f).y, _192.z == float4(1.0f, 0.5f, 0.0f, -0.5f).z, _192.w == float4(1.0f, 0.5f, 0.0f, -0.5f).w))) && all(bool4(_196.x == float4(-8.0f, -1.0f, 2.0f, 2.0f).x, _196.y == float4(-8.0f, -1.0f, 2.0f, 2.0f).y, _196.z == float4(-8.0f, -1.0f, 2.0f, 2.0f).z, _196.w == float4(-8.0f, -1.0f, 2.0f, 2.0f).w))) && all(bool4(_200.x == float4(3.0f, 0.5f, -1.0f, -0.5f).x, _200.y == float4(3.0f, 0.5f, -1.0f, -0.5f).y, _200.z == float4(3.0f, 0.5f, -1.0f, -0.5f).z, _200.w == float4(3.0f, 0.5f, -1.0f, -0.5f).w));
    }
    else
    {
        _204 = false;
    }
    float4 _205 = 0.0f.xxxx;
    if (_204)
    {
        _205 = _10_colorGreen;
    }
    else
    {
        _205 = _10_colorRed;
    }
    return _205;
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
