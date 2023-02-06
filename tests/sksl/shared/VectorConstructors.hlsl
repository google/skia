cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float _11_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(float2 _39, float2 _40, float2 _41, float3 _42, int2 _43, int2 _44, float2 _45, float2 _46, float4 _47, int2 _48, bool4 _49, float2 _50, float2 _51, float2 _52, bool2 _53, bool2 _54, bool3 _55, int4 _56)
{
    return (((((((((((((((((_39.x + _40.x) + _41.x) + _42.x) + float(_43.x)) + float(_44.x)) + _45.x) + _46.x) + _47.x) + float(_48.x)) + float(_49.x)) + _50.x) + _51.x) + _52.x) + float(_53.x)) + float(_54.x)) + float(_55.x)) + float(_56.x)) == 18.0f;
}

float4 main(float2 _123)
{
    float2 v1 = 1.0f.xx;
    float2 v2 = float2(1.0f, 2.0f);
    float2 v3 = 1.0f.xx;
    float3 v4 = 1.0f.xxx;
    int2 v5 = int2(1, 1);
    int2 v6 = int2(1, 2);
    float2 v7 = float2(1.0f, 2.0f);
    float2 _143 = float2(float(1), float(1));
    float2 v8 = _143;
    float4 _151 = float4(float(1), _11_unknownInput, 3.0f, 4.0f);
    float4 v9 = _151;
    int2 _155 = int2(3, int(1.0f));
    int2 v10 = _155;
    bool4 v11 = bool4(true, false, true, false);
    float2 v12 = float2(1.0f, 0.0f);
    float2 v13 = 0.0f.xx;
    float2 v14 = 0.0f.xx;
    bool2 v15 = bool2(true, true);
    bool2 v16 = bool2(true, true);
    bool3 v17 = bool3(true, true, true);
    int4 v18 = int4(1, 1, 1, 1);
    float2 _171 = 1.0f.xx;
    float2 _172 = float2(1.0f, 2.0f);
    float2 _173 = 1.0f.xx;
    float3 _174 = 1.0f.xxx;
    int2 _175 = int2(1, 1);
    int2 _176 = int2(1, 2);
    float2 _177 = float2(1.0f, 2.0f);
    float2 _178 = _143;
    float4 _179 = _151;
    int2 _180 = _155;
    bool4 _181 = bool4(true, false, true, false);
    float2 _182 = float2(1.0f, 0.0f);
    float2 _183 = 0.0f.xx;
    float2 _184 = 0.0f.xx;
    bool2 _185 = bool2(true, true);
    bool2 _186 = bool2(true, true);
    bool3 _187 = bool3(true, true, true);
    int4 _188 = int4(1, 1, 1, 1);
    float4 _190 = 0.0f.xxxx;
    if (check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(_171, _172, _173, _174, _175, _176, _177, _178, _179, _180, _181, _182, _183, _184, _185, _186, _187, _188))
    {
        _190 = _11_colorGreen;
    }
    else
    {
        _190 = _11_colorRed;
    }
    return _190;
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
