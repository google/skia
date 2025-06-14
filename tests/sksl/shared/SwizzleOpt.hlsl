cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorRed : packoffset(c0);
    float4 _12_colorGreen : packoffset(c1);
    float4 _12_testInputs : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float fn_hh4(float4 _27)
{
    for (int x = 1; x <= 2; x++)
    {
        return _27.x;
    }
}

float4 main(float2 _46)
{
    float4 v = _12_testInputs;
    float4 _57 = float4(0.0f, _12_testInputs.zyx);
    v = _57;
    float4 _61 = float4(0.0f, 0.0f, _57.xw);
    v = _61;
    float4 _66 = float4(1.0f, 1.0f, _61.wx);
    v = _66;
    float4 _70 = float4(_66.zy, 1.0f, 1.0f);
    v = _70;
    float4 _74 = float4(_70.xx, 1.0f, 1.0f);
    v = _74;
    float4 _75 = _74.wzwz;
    v = _75;
    float4 _76 = _75;
    float4 _81 = float3(fn_hh4(_76), 123.0f, 456.0f).yyzz;
    v = _81;
    float4 _82 = _81;
    float4 _85 = float3(fn_hh4(_82), 123.0f, 456.0f).yyzz;
    v = _85;
    float4 _86 = _85;
    float4 _88 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_86));
    v = _88;
    float4 _89 = _88;
    float4 _91 = float4(123.0f, 456.0f, 456.0f, fn_hh4(_89));
    v = _91;
    float4 _92 = _91;
    float4 _95 = float3(fn_hh4(_92), 123.0f, 456.0f).yxxz;
    v = _95;
    float4 _96 = _95;
    v = float3(fn_hh4(_96), 123.0f, 456.0f).yxxz;
    v = float4(1.0f, 1.0f, 2.0f, 3.0f);
    v = float4(_12_colorRed.xyz, 1.0f);
    float4 _119 = float4(_12_colorRed.x, 1.0f, _12_colorRed.yz);
    v = _119;
    float4 _120 = v;
    float4 _121 = float4(_119.w, _119.z, _119.y, _119.x);
    v = _121;
    float2 _122 = _121.yz;
    float4 _123 = v;
    float4 _124 = float4(_122.x, _123.y, _123.z, _122.y);
    v = _124;
    float3 _128 = float3(_124.ww, 1.0f);
    float4 _129 = v;
    float4 _130 = float4(_128.z, _128.y, _128.x, _129.w);
    v = _130;
    float4 _135 = 0.0f.xxxx;
    if (all(bool4(_130.x == 1.0f.xxxx.x, _130.y == 1.0f.xxxx.y, _130.z == 1.0f.xxxx.z, _130.w == 1.0f.xxxx.w)))
    {
        _135 = _12_colorGreen;
    }
    else
    {
        _135 = _12_colorRed;
    }
    return _135;
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
