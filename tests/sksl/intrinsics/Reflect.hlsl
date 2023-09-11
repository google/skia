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
    float expectedX = spvReflect(996878592.0f, -1.9999999580429535907214788975919e+34f);
    expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _75 = false;
    if (spvReflect(_10_I.x, _10_N.x) == (-49.0f))
    {
        float2 _65 = reflect(_10_I.xy, _10_N.xy);
        _75 = all(bool2(_65.x == float2(-169.0f, 202.0f).x, _65.y == float2(-169.0f, 202.0f).y));
    }
    else
    {
        _75 = false;
    }
    bool _88 = false;
    if (_75)
    {
        float3 _78 = reflect(_10_I.xyz, _10_N.xyz);
        _88 = all(bool3(_78.x == float3(-379.0f, 454.0f, -529.0f).x, _78.y == float3(-379.0f, 454.0f, -529.0f).y, _78.z == float3(-379.0f, 454.0f, -529.0f).z));
    }
    else
    {
        _88 = false;
    }
    bool _99 = false;
    if (_88)
    {
        float4 _91 = reflect(_10_I, _10_N);
        _99 = all(bool4(_91.x == float4(-699.0f, 838.0f, -977.0f, 1116.0f).x, _91.y == float4(-699.0f, 838.0f, -977.0f, 1116.0f).y, _91.z == float4(-699.0f, 838.0f, -977.0f, 1116.0f).z, _91.w == float4(-699.0f, 838.0f, -977.0f, 1116.0f).w));
    }
    else
    {
        _99 = false;
    }
    bool _103 = false;
    if (_99)
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
    bool _112 = false;
    if (_109)
    {
        _112 = true;
    }
    else
    {
        _112 = false;
    }
    float4 _113 = 0.0f.xxxx;
    if (_112)
    {
        _113 = _10_colorGreen;
    }
    else
    {
        _113 = _10_colorRed;
    }
    return _113;
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
