cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_I : packoffset(c0);
    float4 _7_N : packoffset(c1);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
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

float4 main(float2 _21)
{
    float expectedX = spvReflect(996878592.0f, -1.9999999580429535907214788975919e+34f);
    expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _73 = false;
    if (spvReflect(_7_I.x, _7_N.x) == (-49.0f))
    {
        float2 _63 = reflect(_7_I.xy, _7_N.xy);
        _73 = all(bool2(_63.x == float2(-169.0f, 202.0f).x, _63.y == float2(-169.0f, 202.0f).y));
    }
    else
    {
        _73 = false;
    }
    bool _86 = false;
    if (_73)
    {
        float3 _76 = reflect(_7_I.xyz, _7_N.xyz);
        _86 = all(bool3(_76.x == float3(-379.0f, 454.0f, -529.0f).x, _76.y == float3(-379.0f, 454.0f, -529.0f).y, _76.z == float3(-379.0f, 454.0f, -529.0f).z));
    }
    else
    {
        _86 = false;
    }
    bool _97 = false;
    if (_86)
    {
        float4 _89 = reflect(_7_I, _7_N);
        _97 = all(bool4(_89.x == float4(-699.0f, 838.0f, -977.0f, 1116.0f).x, _89.y == float4(-699.0f, 838.0f, -977.0f, 1116.0f).y, _89.z == float4(-699.0f, 838.0f, -977.0f, 1116.0f).z, _89.w == float4(-699.0f, 838.0f, -977.0f, 1116.0f).w));
    }
    else
    {
        _97 = false;
    }
    bool _101 = false;
    if (_97)
    {
        _101 = true;
    }
    else
    {
        _101 = false;
    }
    bool _104 = false;
    if (_101)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    bool _107 = false;
    if (_104)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    bool _110 = false;
    if (_107)
    {
        _110 = true;
    }
    else
    {
        _110 = false;
    }
    float4 _111 = 0.0f.xxxx;
    if (_110)
    {
        _111 = _7_colorGreen;
    }
    else
    {
        _111 = _7_colorRed;
    }
    return _111;
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
