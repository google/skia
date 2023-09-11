cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorRed : packoffset(c0);
    float4 _8_colorGreen : packoffset(c1);
    float4 _8_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float fn_hh4(float4 _23)
{
    for (int x = 1; x <= 2; x++)
    {
        return _23.x;
    }
}

float4 main(float2 _43)
{
    float4 v = _8_testInputs;
    float4 _54 = float4(0.0f, _8_testInputs.zyx);
    v = _54;
    float4 _58 = float4(0.0f, 0.0f, _54.xw);
    v = _58;
    float4 _63 = float4(1.0f, 1.0f, _58.wx);
    v = _63;
    float4 _67 = float4(_63.zy, 1.0f, 1.0f);
    v = _67;
    float4 _71 = float4(_67.xx, 1.0f, 1.0f);
    v = _71;
    float4 _72 = _71.wzwz;
    v = _72;
    float4 _73 = _72;
    float4 _78 = float3(fn_hh4(_73), 123.0f, 456.0f).yyzz;
    v = _78;
    float4 _79 = _78;
    float4 _82 = float3(fn_hh4(_79), 123.0f, 456.0f).yyzz;
    v = _82;
    float4 _83 = _82;
    float4 _85 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_83));
    v = _85;
    float4 _86 = _85;
    float4 _88 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_86));
    v = _88;
    float4 _89 = _88;
    float4 _92 = float3(fn_hh4(_89), 123.0f, 456.0f).yxxz;
    v = _92;
    float4 _93 = _92;
    v = float3(fn_hh4(_93), 123.0f, 456.0f).yxxz;
    v = float4(1.0f, 1.0f, 2.0f, 3.0f);
    v = float4(_8_colorRed.xyz, 1.0f);
    float4 _116 = float4(_8_colorRed.x, 1.0f, _8_colorRed.yz);
    v = _116;
    float4 _117 = v;
    float4 _118 = float4(_116.w, _116.z, _116.y, _116.x);
    v = _118;
    float2 _119 = _118.yz;
    float4 _120 = v;
    float4 _121 = float4(_119.x, _120.y, _120.z, _119.y);
    v = _121;
    float3 _125 = float3(_121.ww, 1.0f);
    float4 _126 = v;
    float4 _127 = float4(_125.z, _125.y, _125.x, _126.w);
    v = _127;
    float4 _132 = 0.0f.xxxx;
    if (all(bool4(_127.x == 1.0f.xxxx.x, _127.y == 1.0f.xxxx.y, _127.z == 1.0f.xxxx.z, _127.w == 1.0f.xxxx.w)))
    {
        _132 = _8_colorGreen;
    }
    else
    {
        _132 = _8_colorRed;
    }
    return _132;
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
