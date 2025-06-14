cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool checkIntrinsicAsFunctionArg_bf3i3(float3 _31, int3 _32)
{
    bool _48 = false;
    if (all(bool3(_31.x == 0.75f.xxx.x, _31.y == 0.75f.xxx.y, _31.z == 0.75f.xxx.z)))
    {
        _48 = all(bool3(_32.x == int3(3, 3, 3).x, _32.y == int3(3, 3, 3).y, _32.z == int3(3, 3, 3).z));
    }
    else
    {
        _48 = false;
    }
    return _48;
}

float4 main(float2 _50)
{
    float4 _60 = _12_colorGreen.yyyy * 6.0f;
    float4 value = _60;
    int4 _exp = int4(0, 0, 0, 0);
    int _72 = 0;
    float _68 = frexp(_60.x, _72);
    _exp.x = _72;
    float4 result = 0.0f.xxxx;
    result.x = _68;
    bool _84 = false;
    if (result.x == 0.75f)
    {
        _84 = _exp.x == 3;
    }
    else
    {
        _84 = false;
    }
    bool4 ok = bool4(false, false, false, false);
    ok.x = _84;
    int2 _90 = int2(0, 0);
    float2 _87 = frexp(value.xy, _90);
    int4 _94 = _exp;
    _exp = int4(_90.x, _90.y, _94.z, _94.w);
    float4 _96 = result;
    result = float4(_87.x, _87.y, _96.z, _96.w);
    bool _104 = false;
    if (float4(_87.x, _87.y, _96.z, _96.w).y == 0.75f)
    {
        _104 = int4(_90.x, _90.y, _94.z, _94.w).y == 3;
    }
    else
    {
        _104 = false;
    }
    ok.y = _104;
    int3 _110 = int3(0, 0, 0);
    float3 _107 = frexp(value.xyz, _110);
    int4 _112 = _exp;
    _exp = int4(_110.x, _110.y, _110.z, _112.w);
    float4 _114 = result;
    result = float4(_107.x, _107.y, _107.z, _114.w);
    bool _122 = false;
    if (float4(_107.x, _107.y, _107.z, _114.w).z == 0.75f)
    {
        _122 = int4(_110.x, _110.y, _110.z, _112.w).z == 3;
    }
    else
    {
        _122 = false;
    }
    ok.z = _122;
    int4 _127 = int4(0, 0, 0, 0);
    float4 _125 = frexp(value, _127);
    _exp = _127;
    result = _125;
    bool _135 = false;
    if (_125.w == 0.75f)
    {
        _135 = _127.w == 3;
    }
    else
    {
        _135 = false;
    }
    ok.w = _135;
    int3 _141 = int3(0, 0, 0);
    float3 _138 = frexp(value.wzy, _141);
    int4 _143 = _exp;
    _exp = int4(_141.y, _143.y, _141.x, _141.z);
    float3 _146 = _138.yxz;
    int3 _148 = int4(_141.y, _143.y, _141.x, _141.z).yxz;
    bool _149 = checkIntrinsicAsFunctionArg_bf3i3(_146, _148);
    bool funcOk = _149;
    bool _154 = false;
    if (all(ok))
    {
        _154 = _149;
    }
    else
    {
        _154 = false;
    }
    float4 _155 = 0.0f.xxxx;
    if (_154)
    {
        _155 = _12_colorGreen;
    }
    else
    {
        _155 = _12_colorRed;
    }
    return _155;
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
