cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 _35 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y);
    float4 f4 = _35;
    float3 _39 = _35.xyz;
    float2 _44 = float2(_39.xy);
    float2 _46 = float2(_39.z, 4.0f);
    bool _59 = all(bool2(_44.x == float2(1.0f, 2.0f).x, _44.y == float2(1.0f, 2.0f).y)) && all(bool2(_46.x == float2(3.0f, 4.0f).x, _46.y == float2(3.0f, 4.0f).y));
    bool ok = _59;
    bool _87 = false;
    if (_59)
    {
        float2 _64 = _35.zw;
        float3 _68 = float3(_35.xy, _64.x);
        float3 _70 = float3(_64.y, _7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y);
        float3 _71 = float3(_7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y, _7_testMatrix2x2[0].x);
        _87 = (all(bool3(_68.x == float3(1.0f, 2.0f, 3.0f).x, _68.y == float3(1.0f, 2.0f, 3.0f).y, _68.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_70.x == float3(4.0f, 1.0f, 2.0f).x, _70.y == float3(4.0f, 1.0f, 2.0f).y, _70.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_71.x == float3(3.0f, 4.0f, 1.0f).x, _71.y == float3(3.0f, 4.0f, 1.0f).y, _71.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _87 = false;
    }
    ok = _87;
    bool _125 = false;
    if (_87)
    {
        float3 _91 = _35.wxy;
        float4 _92 = _35.zwxy;
        float4 _98 = float4(_35.xyz, _91.x);
        float4 _103 = float4(_91.yz, _92.xy);
        float4 _108 = float4(_92.zw, _35.zw);
        _125 = ((all(bool4(_98.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _98.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _98.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _98.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_103.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _103.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _103.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _103.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_108.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _108.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _108.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _108.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_35.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _35.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _35.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _35.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _125 = false;
    }
    ok = _125;
    float4 _126 = 0.0f.xxxx;
    if (_125)
    {
        _126 = _7_colorGreen;
    }
    else
    {
        _126 = _7_colorRed;
    }
    return _126;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
