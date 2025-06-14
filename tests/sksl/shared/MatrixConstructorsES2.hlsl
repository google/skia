cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 _38 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y);
    float4 f4 = _38;
    float3 _42 = _38.xyz;
    float2 _47 = float2(_42.xy);
    float2 _49 = float2(_42.z, 4.0f);
    bool _62 = all(bool2(_47.x == float2(1.0f, 2.0f).x, _47.y == float2(1.0f, 2.0f).y)) && all(bool2(_49.x == float2(3.0f, 4.0f).x, _49.y == float2(3.0f, 4.0f).y));
    bool ok = _62;
    bool _90 = false;
    if (_62)
    {
        float2 _67 = _38.zw;
        float3 _71 = float3(_38.xy, _67.x);
        float3 _73 = float3(_67.y, _11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y);
        float3 _74 = float3(_11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y, _11_testMatrix2x2[0].x);
        _90 = (all(bool3(_71.x == float3(1.0f, 2.0f, 3.0f).x, _71.y == float3(1.0f, 2.0f, 3.0f).y, _71.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_73.x == float3(4.0f, 1.0f, 2.0f).x, _73.y == float3(4.0f, 1.0f, 2.0f).y, _73.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_74.x == float3(3.0f, 4.0f, 1.0f).x, _74.y == float3(3.0f, 4.0f, 1.0f).y, _74.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _90 = false;
    }
    ok = _90;
    bool _128 = false;
    if (_90)
    {
        float3 _94 = _38.wxy;
        float4 _95 = _38.zwxy;
        float4 _101 = float4(_38.xyz, _94.x);
        float4 _106 = float4(_94.yz, _95.xy);
        float4 _111 = float4(_95.zw, _38.zw);
        _128 = ((all(bool4(_101.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _101.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _101.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _101.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_106.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _106.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _106.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _106.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_111.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _111.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _111.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _111.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w))) && all(bool4(_38.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _38.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _38.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _38.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _128 = false;
    }
    ok = _128;
    float4 _129 = 0.0f.xxxx;
    if (_128)
    {
        _129 = _11_colorGreen;
    }
    else
    {
        _129 = _11_colorRed;
    }
    return _129;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
