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
    bool _72 = false;
    if (spvFaceForward(_10_N.x, _10_I.x, _10_NRef.x) == (-1.0f))
    {
        float2 _58 = faceforward(_10_N.xy, _10_I.xy, _10_NRef.xy);
        _72 = all(bool2(_58.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, _58.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _72 = false;
    }
    bool _90 = false;
    if (_72)
    {
        float3 _75 = faceforward(_10_N.xyz, _10_I.xyz, _10_NRef.xyz);
        _90 = all(bool3(_75.x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, _75.y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, _75.z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _90 = false;
    }
    bool _103 = false;
    if (_90)
    {
        float4 _93 = faceforward(_10_N, _10_I, _10_NRef);
        _103 = all(bool4(_93.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _93.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _93.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _93.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _103 = false;
    }
    bool _107 = false;
    if (_103)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    bool _114 = false;
    if (_107)
    {
        _114 = all(bool2(float2(-1.0f, -2.0f).x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, float2(-1.0f, -2.0f).y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _114 = false;
    }
    bool _121 = false;
    if (_114)
    {
        _121 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, float3(1.0f, 2.0f, 3.0f).y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, float3(1.0f, 2.0f, 3.0f).z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _121 = false;
    }
    bool _124 = false;
    if (_121)
    {
        _124 = true;
    }
    else
    {
        _124 = false;
    }
    float4 _125 = 0.0f.xxxx;
    if (_124)
    {
        _125 = _10_colorGreen;
    }
    else
    {
        _125 = _10_colorRed;
    }
    return _125;
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
