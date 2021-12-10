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
    v = float4(0.0f, v.zyx);
    v = float4(0.0f, 0.0f, v.xw);
    v = float4(1.0f, 1.0f, v.wx);
    v = float4(v.zy, 1.0f, 1.0f);
    v = float4(v.xx, 1.0f, 1.0f);
    v = v.wzwz;
    float4 _82 = v;
    v = float3(fn_hh4(_82), 123.0f, 456.0f).yyzz;
    float4 _89 = v;
    v = float3(fn_hh4(_89), 123.0f, 456.0f).yyzz;
    float4 _94 = v;
    v = float4(123.0f, 456.0f, 456.0f, fn_hh4(_94));
    float4 _98 = v;
    v = float4(123.0f, 456.0f, 456.0f, fn_hh4(_98));
    float4 _102 = v;
    v = float3(fn_hh4(_102), 123.0f, 456.0f).yxxz;
    float4 _107 = v;
    v = float3(fn_hh4(_107), 123.0f, 456.0f).yxxz;
    v = float4(1.0f, 1.0f, 2.0f, 3.0f);
    v = float4(_11_colorRed.xyz, 1.0f);
    v = float4(_11_colorRed.x, 1.0f, _11_colorRed.yz);
    v = float4(v.w, v.z, v.y, v.x);
    v = float4(v.yz.x, v.y, v.z, v.yz.y);
    float3 _142 = float3(v.ww, 1.0f);
    v = float4(_142.z, _142.y, _142.x, v.w);
    float4 _150 = 0.0f.xxxx;
    if (all(bool4(v.x == 1.0f.xxxx.x, v.y == 1.0f.xxxx.y, v.z == 1.0f.xxxx.z, v.w == 1.0f.xxxx.w)))
    {
        _150 = _11_colorGreen;
    }
    else
    {
        _150 = _11_colorRed;
    }
    return _150;
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
