cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _38 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
    float4 f4 = _38;
    float3 _43 = float3(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x);
    float3 _46 = float3(_10_testMatrix2x2[1].y, _38.xy);
    bool _61 = all(bool3(_43.x == float3(1.0f, 2.0f, 3.0f).x, _43.y == float3(1.0f, 2.0f, 3.0f).y, _43.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_46.x == float3(4.0f, 1.0f, 2.0f).x, _46.y == float3(4.0f, 1.0f, 2.0f).y, _46.z == float3(4.0f, 1.0f, 2.0f).z));
    bool ok = _61;
    bool _86 = false;
    if (_61)
    {
        float4 _66 = _38.wxyz;
        float4 _71 = float4(_38.xyz, _66.x);
        float4 _75 = float4(_66.yzw, _10_testMatrix2x2[1].y);
        _86 = all(bool4(_71.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _71.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _71.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _71.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_75.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _75.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _75.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _75.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _86 = false;
    }
    ok = _86;
    bool _110 = false;
    if (_86)
    {
        float2 _90 = _38.zw;
        float3 _94 = float3(_38.xy, _90.x);
        float3 _96 = float3(_90.y, _10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y);
        float3 _97 = float3(_10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y, _10_testMatrix2x2[0].x);
        _110 = (all(bool3(_94.x == float3(1.0f, 2.0f, 3.0f).x, _94.y == float3(1.0f, 2.0f, 3.0f).y, _94.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_96.x == float3(4.0f, 1.0f, 2.0f).x, _96.y == float3(4.0f, 1.0f, 2.0f).y, _96.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_97.x == float3(3.0f, 4.0f, 1.0f).x, _97.y == float3(3.0f, 4.0f, 1.0f).y, _97.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _110 = false;
    }
    ok = _110;
    bool _143 = false;
    if (_110)
    {
        float3 _113 = _38.xyz;
        float4 _114 = _38.wxyz;
        float2 _117 = float2(_113.xy);
        float2 _120 = float2(_113.z, _114.x);
        float2 _123 = float2(_114.yz);
        float2 _125 = float2(_114.w, _10_testMatrix2x2[1].y);
        _143 = ((all(bool2(_117.x == float2(1.0f, 2.0f).x, _117.y == float2(1.0f, 2.0f).y)) && all(bool2(_120.x == float2(3.0f, 4.0f).x, _120.y == float2(3.0f, 4.0f).y))) && all(bool2(_123.x == float2(1.0f, 2.0f).x, _123.y == float2(1.0f, 2.0f).y))) && all(bool2(_125.x == float2(3.0f, 4.0f).x, _125.y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _143 = false;
    }
    ok = _143;
    bool _175 = false;
    if (_143)
    {
        float4 _146 = _38.yzwx;
        float4 _147 = _38.yzwx;
        float3 _148 = _38.yzw;
        float3 _151 = float3(_10_testMatrix2x2[0].x, _146.xy);
        float3 _155 = float3(_146.zw, _147.x);
        float3 _159 = float3(_147.yzw);
        _175 = ((all(bool3(_151.x == float3(1.0f, 2.0f, 3.0f).x, _151.y == float3(1.0f, 2.0f, 3.0f).y, _151.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_155.x == float3(4.0f, 1.0f, 2.0f).x, _155.y == float3(4.0f, 1.0f, 2.0f).y, _155.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_159.x == float3(3.0f, 4.0f, 1.0f).x, _159.y == float3(3.0f, 4.0f, 1.0f).y, _159.z == float3(3.0f, 4.0f, 1.0f).z))) && all(bool3(_148.x == float3(2.0f, 3.0f, 4.0f).x, _148.y == float3(2.0f, 3.0f, 4.0f).y, _148.z == float3(2.0f, 3.0f, 4.0f).z));
    }
    else
    {
        _175 = false;
    }
    ok = _175;
    float4 _176 = 0.0f.xxxx;
    if (_175)
    {
        _176 = _10_colorGreen;
    }
    else
    {
        _176 = _10_colorRed;
    }
    return _176;
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
