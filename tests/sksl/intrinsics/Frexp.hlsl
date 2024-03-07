cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool checkIntrinsicAsFunctionArg_bf3i3(float3 _28, int3 _29)
{
    bool _45 = false;
    if (all(bool3(_28.x == 0.75f.xxx.x, _28.y == 0.75f.xxx.y, _28.z == 0.75f.xxx.z)))
    {
        _45 = all(bool3(_29.x == int3(3, 3, 3).x, _29.y == int3(3, 3, 3).y, _29.z == int3(3, 3, 3).z));
    }
    else
    {
        _45 = false;
    }
    return _45;
}

float4 main(float2 _47)
{
    float4 _57 = _8_colorGreen.yyyy * 6.0f;
    float4 value = _57;
    int4 _exp = int4(0, 0, 0, 0);
    int _69 = 0;
    float _65 = frexp(_57.x, _69);
    _exp.x = _69;
    float4 result = 0.0f.xxxx;
    result.x = _65;
    bool _81 = false;
    if (result.x == 0.75f)
    {
        _81 = _exp.x == 3;
    }
    else
    {
        _81 = false;
    }
    bool4 ok = bool4(false, false, false, false);
    ok.x = _81;
    int2 _87 = int2(0, 0);
    float2 _84 = frexp(value.xy, _87);
    int4 _91 = _exp;
    _exp = int4(_87.x, _87.y, _91.z, _91.w);
    float4 _93 = result;
    result = float4(_84.x, _84.y, _93.z, _93.w);
    bool _101 = false;
    if (float4(_84.x, _84.y, _93.z, _93.w).y == 0.75f)
    {
        _101 = int4(_87.x, _87.y, _91.z, _91.w).y == 3;
    }
    else
    {
        _101 = false;
    }
    ok.y = _101;
    int3 _107 = int3(0, 0, 0);
    float3 _104 = frexp(value.xyz, _107);
    int4 _109 = _exp;
    _exp = int4(_107.x, _107.y, _107.z, _109.w);
    float4 _111 = result;
    result = float4(_104.x, _104.y, _104.z, _111.w);
    bool _119 = false;
    if (float4(_104.x, _104.y, _104.z, _111.w).z == 0.75f)
    {
        _119 = int4(_107.x, _107.y, _107.z, _109.w).z == 3;
    }
    else
    {
        _119 = false;
    }
    ok.z = _119;
    int4 _124 = int4(0, 0, 0, 0);
    float4 _122 = frexp(value, _124);
    _exp = _124;
    result = _122;
    bool _132 = false;
    if (_122.w == 0.75f)
    {
        _132 = _124.w == 3;
    }
    else
    {
        _132 = false;
    }
    ok.w = _132;
    int3 _138 = int3(0, 0, 0);
    float3 _135 = frexp(value.wzy, _138);
    int4 _140 = _exp;
    _exp = int4(_138.y, _140.y, _138.x, _138.z);
    float3 _143 = _135.yxz;
    int3 _145 = int4(_138.y, _140.y, _138.x, _138.z).yxz;
    bool _146 = checkIntrinsicAsFunctionArg_bf3i3(_143, _145);
    bool funcOk = _146;
    bool _151 = false;
    if (all(ok))
    {
        _151 = _146;
    }
    else
    {
        _151 = false;
    }
    float4 _152 = 0.0f.xxxx;
    if (_151)
    {
        _152 = _8_colorGreen;
    }
    else
    {
        _152 = _8_colorRed;
    }
    return _152;
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
