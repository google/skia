cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_I : packoffset(c0);
    float4 _10_N : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvReflect(float i, float n)
{
    return i - 2.0 * dot(n, i) * n;
}

float4 main(float2 _24)
{
    float expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _72 = false;
    if (spvReflect(_10_I.x, _10_N.x) == (-49.0f))
    {
        float2 _62 = reflect(_10_I.xy, _10_N.xy);
        _72 = all(bool2(_62.x == float2(-169.0f, 202.0f).x, _62.y == float2(-169.0f, 202.0f).y));
    }
    else
    {
        _72 = false;
    }
    bool _85 = false;
    if (_72)
    {
        float3 _75 = reflect(_10_I.xyz, _10_N.xyz);
        _85 = all(bool3(_75.x == float3(-379.0f, 454.0f, -529.0f).x, _75.y == float3(-379.0f, 454.0f, -529.0f).y, _75.z == float3(-379.0f, 454.0f, -529.0f).z));
    }
    else
    {
        _85 = false;
    }
    bool _96 = false;
    if (_85)
    {
        float4 _88 = reflect(_10_I, _10_N);
        _96 = all(bool4(_88.x == float4(-699.0f, 838.0f, -977.0f, 1116.0f).x, _88.y == float4(-699.0f, 838.0f, -977.0f, 1116.0f).y, _88.z == float4(-699.0f, 838.0f, -977.0f, 1116.0f).z, _88.w == float4(-699.0f, 838.0f, -977.0f, 1116.0f).w));
    }
    else
    {
        _96 = false;
    }
    bool _100 = false;
    if (_96)
    {
        _100 = true;
    }
    else
    {
        _100 = false;
    }
    bool _103 = false;
    if (_100)
    {
        _103 = true;
    }
    else
    {
        _103 = false;
    }
    bool _106 = false;
    if (_103)
    {
        _106 = true;
    }
    else
    {
        _106 = false;
    }
    bool _109 = false;
    if (_106)
    {
        _109 = true;
    }
    else
    {
        _109 = false;
    }
    float4 _110 = 0.0f.xxxx;
    if (_109)
    {
        _110 = _10_colorGreen;
    }
    else
    {
        _110 = _10_colorRed;
    }
    return _110;
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
