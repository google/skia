cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorRed : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float fn_hh4(float4 _26)
{
    for (int x = 1; x <= 2; x++)
    {
        return _26.x;
    }
}

float4 main(float2 _45)
{
    float4 v = _11_testInputs;
    float4 _56 = float4(0.0f, _11_testInputs.zyx);
    v = _56;
    float4 _60 = float4(0.0f, 0.0f, _56.xw);
    v = _60;
    float4 _65 = float4(1.0f, 1.0f, _60.wx);
    v = _65;
    float4 _69 = float4(_65.zy, 1.0f, 1.0f);
    v = _69;
    float4 _73 = float4(_69.xx, 1.0f, 1.0f);
    v = _73;
    float4 _74 = _73.wzwz;
    v = _74;
    float4 _75 = _74;
    float4 _80 = float3(fn_hh4(_75), 123.0f, 456.0f).yyzz;
    v = _80;
    float4 _81 = _80;
    float4 _84 = float3(fn_hh4(_81), 123.0f, 456.0f).yyzz;
    v = _84;
    float4 _85 = _84;
    float4 _87 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_85));
    v = _87;
    float4 _88 = _87;
    float4 _90 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_88));
    v = _90;
    float4 _91 = _90;
    float4 _94 = float3(fn_hh4(_91), 123.0f, 456.0f).yxxz;
    v = _94;
    float4 _95 = _94;
    v = float3(fn_hh4(_95), 123.0f, 456.0f).yxxz;
    v = float4(1.0f, 1.0f, 2.0f, 3.0f);
    v = float4(_11_colorRed.xyz, 1.0f);
    float4 _118 = float4(_11_colorRed.x, 1.0f, _11_colorRed.yz);
    v = _118;
    float4 _119 = v;
    float4 _120 = float4(_118.w, _118.z, _118.y, _118.x);
    v = _120;
    float2 _121 = _120.yz;
    float4 _122 = v;
    float4 _123 = float4(_121.x, _122.y, _122.z, _121.y);
    v = _123;
    float3 _127 = float3(_123.ww, 1.0f);
    float4 _128 = v;
    float4 _129 = float4(_127.z, _127.y, _127.x, _128.w);
    v = _129;
    float4 _134 = 0.0f.xxxx;
    if (all(bool4(_129.x == 1.0f.xxxx.x, _129.y == 1.0f.xxxx.y, _129.z == 1.0f.xxxx.z, _129.w == 1.0f.xxxx.w)))
    {
        _134 = _11_colorGreen;
    }
    else
    {
        _134 = _11_colorRed;
    }
    return _134;
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
