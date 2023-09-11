cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorGreen : packoffset(c0);
    float4 _8_colorRed : packoffset(c1);
    float _8_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(float2 _37, float2 _38, float2 _39, float3 _40, int2 _41, int2 _42, float2 _43, float2 _44, float4 _45, int2 _46, bool4 _47, float2 _48, float2 _49, float2 _50, bool2 _51, bool2 _52, bool3 _53, int4 _54)
{
    return (((((((((((((((((_37.x + _38.x) + _39.x) + _40.x) + float(_41.x)) + float(_42.x)) + _43.x) + _44.x) + _45.x) + float(_46.x)) + float(_47.x)) + _48.x) + _49.x) + _50.x) + float(_51.x)) + float(_52.x)) + float(_53.x)) + float(_54.x)) == 18.0f;
}

float4 main(float2 _121)
{
    float2 v1 = 1.0f.xx;
    float2 v2 = float2(1.0f, 2.0f);
    float2 v3 = 1.0f.xx;
    float3 v4 = 1.0f.xxx;
    int2 v5 = int2(1, 1);
    int2 v6 = int2(1, 2);
    float2 v7 = float2(1.0f, 2.0f);
    float2 _141 = float2(float(1), float(1));
    float2 v8 = _141;
    float4 _149 = float4(float(1), _8_unknownInput, 3.0f, 4.0f);
    float4 v9 = _149;
    int2 _153 = int2(3, int(1.0f));
    int2 v10 = _153;
    bool4 v11 = bool4(true, false, true, false);
    float2 v12 = float2(1.0f, 0.0f);
    float2 v13 = 0.0f.xx;
    float2 v14 = 0.0f.xx;
    bool2 v15 = bool2(true, true);
    bool2 v16 = bool2(true, true);
    bool3 v17 = bool3(true, true, true);
    int4 v18 = int4(1, 1, 1, 1);
    float2 _169 = 1.0f.xx;
    float2 _170 = float2(1.0f, 2.0f);
    float2 _171 = 1.0f.xx;
    float3 _172 = 1.0f.xxx;
    int2 _173 = int2(1, 1);
    int2 _174 = int2(1, 2);
    float2 _175 = float2(1.0f, 2.0f);
    float2 _176 = _141;
    float4 _177 = _149;
    int2 _178 = _153;
    bool4 _179 = bool4(true, false, true, false);
    float2 _180 = float2(1.0f, 0.0f);
    float2 _181 = 0.0f.xx;
    float2 _182 = 0.0f.xx;
    bool2 _183 = bool2(true, true);
    bool2 _184 = bool2(true, true);
    bool3 _185 = bool3(true, true, true);
    int4 _186 = int4(1, 1, 1, 1);
    float4 _188 = 0.0f.xxxx;
    if (check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(_169, _170, _171, _172, _173, _174, _175, _176, _177, _178, _179, _180, _181, _182, _183, _184, _185, _186))
    {
        _188 = _8_colorGreen;
    }
    else
    {
        _188 = _8_colorRed;
    }
    return _188;
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
