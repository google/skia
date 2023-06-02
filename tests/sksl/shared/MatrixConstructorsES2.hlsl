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
    float3 _41 = _38.xyz;
    float2 _46 = float2(_41.xy);
    float2 _48 = float2(_41.z, 4.0f);
    bool _61 = all(bool2(_46.x == float2(1.0f, 2.0f).x, _46.y == float2(1.0f, 2.0f).y)) && all(bool2(_48.x == float2(3.0f, 4.0f).x, _48.y == float2(3.0f, 4.0f).y));
    bool ok = _61;
    bool _89 = false;
    if (_61)
    {
        float2 _66 = _38.zw;
        float3 _70 = float3(_38.xy, _66.x);
        float3 _72 = float3(_66.y, _10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y);
        float3 _73 = float3(_10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y, _10_testMatrix2x2[0].x);
        _89 = (all(bool3(_70.x == float3(1.0f, 2.0f, 3.0f).x, _70.y == float3(1.0f, 2.0f, 3.0f).y, _70.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_72.x == float3(4.0f, 1.0f, 2.0f).x, _72.y == float3(4.0f, 1.0f, 2.0f).y, _72.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_73.x == float3(3.0f, 4.0f, 1.0f).x, _73.y == float3(3.0f, 4.0f, 1.0f).y, _73.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _89 = false;
    }
    ok = _89;
    bool _127 = false;
    if (_89)
    {
        float3 _93 = _38.wxy;
        float4 _94 = _38.zwxy;
        float4 _100 = float4(_38.xyz, _93.x);
        float4 _105 = float4(_93.yz, _94.xy);
        float4 _110 = float4(_94.zw, _38.zw);
        _127 = ((all(bool4(_100.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _100.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _100.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _100.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_105.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _105.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _105.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _105.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_110.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _110.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _110.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _110.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_38.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _38.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _38.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _38.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _127 = false;
    }
    ok = _127;
    float4 _128 = 0.0f.xxxx;
    if (_127)
    {
        _128 = _10_colorGreen;
    }
    else
    {
        _128 = _10_colorRed;
    }
    return _128;
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
