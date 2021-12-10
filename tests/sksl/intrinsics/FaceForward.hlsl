cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_N : packoffset(c0);
    float4 _10_I : packoffset(c1);
    float4 _10_NRef : packoffset(c2);
    float4 _10_colorGreen : packoffset(c3);
    float4 _10_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvFaceForward(float n, float i, float nref)
{
    return i * nref < 0.0 ? n : -n;
}

float4 main(float2 _24)
{
    float4 expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
    float4 expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
    bool _75 = false;
    if (spvFaceForward(_10_N.x, _10_I.x, _10_NRef.x) == expectedNeg.x)
    {
        float2 _60 = faceforward(_10_N.xy, _10_I.xy, _10_NRef.xy);
        _75 = all(bool2(_60.x == expectedNeg.xy.x, _60.y == expectedNeg.xy.y));
    }
    else
    {
        _75 = false;
    }
    bool _94 = false;
    if (_75)
    {
        float3 _78 = faceforward(_10_N.xyz, _10_I.xyz, _10_NRef.xyz);
        _94 = all(bool3(_78.x == expectedPos.xyz.x, _78.y == expectedPos.xyz.y, _78.z == expectedPos.xyz.z));
    }
    else
    {
        _94 = false;
    }
    bool _108 = false;
    if (_94)
    {
        float4 _97 = faceforward(_10_N, _10_I, _10_NRef);
        _108 = all(bool4(_97.x == expectedPos.x, _97.y == expectedPos.y, _97.z == expectedPos.z, _97.w == expectedPos.w));
    }
    else
    {
        _108 = false;
    }
    bool _114 = false;
    if (_108)
    {
        _114 = (-1.0f) == expectedNeg.x;
    }
    else
    {
        _114 = false;
    }
    bool _122 = false;
    if (_114)
    {
        _122 = all(bool2(float2(-1.0f, -2.0f).x == expectedNeg.xy.x, float2(-1.0f, -2.0f).y == expectedNeg.xy.y));
    }
    else
    {
        _122 = false;
    }
    bool _130 = false;
    if (_122)
    {
        _130 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == expectedPos.xyz.x, float3(1.0f, 2.0f, 3.0f).y == expectedPos.xyz.y, float3(1.0f, 2.0f, 3.0f).z == expectedPos.xyz.z));
    }
    else
    {
        _130 = false;
    }
    bool _136 = false;
    if (_130)
    {
        _136 = all(bool4(float4(1.0f, 2.0f, 3.0f, 4.0f).x == expectedPos.x, float4(1.0f, 2.0f, 3.0f, 4.0f).y == expectedPos.y, float4(1.0f, 2.0f, 3.0f, 4.0f).z == expectedPos.z, float4(1.0f, 2.0f, 3.0f, 4.0f).w == expectedPos.w));
    }
    else
    {
        _136 = false;
    }
    float4 _137 = 0.0f.xxxx;
    if (_136)
    {
        _137 = _10_colorGreen;
    }
    else
    {
        _137 = _10_colorRed;
    }
    return _137;
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
