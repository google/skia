cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _34_colorGreen : packoffset(c0);
    float4 _34_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool takes_void_b()
{
    return true;
}

bool takes_float_bf(float _53)
{
    return true;
}

bool takes_float2_bf2(float2 _56)
{
    return true;
}

bool takes_float3_bf3(float3 _61)
{
    return true;
}

bool takes_float4_bf4(float4 _65)
{
    return true;
}

bool takes_float2x2_bf22(float2x2 _70)
{
    return true;
}

bool takes_float3x3_bf33(float3x3 _75)
{
    return true;
}

bool takes_float4x4_bf44(float4x4 _80)
{
    return true;
}

bool takes_half_bh(float _82)
{
    return true;
}

bool takes_half2_bh2(float2 _84)
{
    return true;
}

bool takes_half3_bh3(float3 _86)
{
    return true;
}

bool takes_half4_bh4(float4 _88)
{
    return true;
}

bool takes_half2x2_bh22(float2x2 _90)
{
    return true;
}

bool takes_half3x3_bh33(float3x3 _92)
{
    return true;
}

bool takes_half4x4_bh44(float4x4 _94)
{
    return true;
}

bool takes_bool_bb(bool _98)
{
    return true;
}

bool takes_bool2_bb2(bool2 _103)
{
    return true;
}

bool takes_bool3_bb3(bool3 _108)
{
    return true;
}

bool takes_bool4_bb4(bool4 _113)
{
    return true;
}

bool takes_int_bi(int _117)
{
    return true;
}

bool takes_int2_bi2(int2 _122)
{
    return true;
}

bool takes_int3_bi3(int3 _127)
{
    return true;
}

bool takes_int4_bi4(int4 _132)
{
    return true;
}

float4 main(float2 _135)
{
    bool _141 = false;
    if (true)
    {
        _141 = takes_void_b();
    }
    else
    {
        _141 = false;
    }
    bool _147 = false;
    if (_141)
    {
        float _145 = 1.0f;
        _147 = takes_float_bf(_145);
    }
    else
    {
        _147 = false;
    }
    bool _154 = false;
    if (_147)
    {
        float2 _152 = 2.0f.xx;
        _154 = takes_float2_bf2(_152);
    }
    else
    {
        _154 = false;
    }
    bool _161 = false;
    if (_154)
    {
        float3 _159 = 3.0f.xxx;
        _161 = takes_float3_bf3(_159);
    }
    else
    {
        _161 = false;
    }
    bool _168 = false;
    if (_161)
    {
        float4 _166 = 4.0f.xxxx;
        _168 = takes_float4_bf4(_166);
    }
    else
    {
        _168 = false;
    }
    bool _176 = false;
    if (_168)
    {
        float2x2 _174 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _176 = takes_float2x2_bf22(_174);
    }
    else
    {
        _176 = false;
    }
    bool _185 = false;
    if (_176)
    {
        float3x3 _183 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _185 = takes_float3x3_bf33(_183);
    }
    else
    {
        _185 = false;
    }
    bool _195 = false;
    if (_185)
    {
        float4x4 _193 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _195 = takes_float4x4_bf44(_193);
    }
    else
    {
        _195 = false;
    }
    bool _200 = false;
    if (_195)
    {
        float _198 = 1.0f;
        _200 = takes_half_bh(_198);
    }
    else
    {
        _200 = false;
    }
    bool _205 = false;
    if (_200)
    {
        float2 _203 = 2.0f.xx;
        _205 = takes_half2_bh2(_203);
    }
    else
    {
        _205 = false;
    }
    bool _210 = false;
    if (_205)
    {
        float3 _208 = 3.0f.xxx;
        _210 = takes_half3_bh3(_208);
    }
    else
    {
        _210 = false;
    }
    bool _215 = false;
    if (_210)
    {
        float4 _213 = 4.0f.xxxx;
        _215 = takes_half4_bh4(_213);
    }
    else
    {
        _215 = false;
    }
    bool _220 = false;
    if (_215)
    {
        float2x2 _218 = float2x2(float2(2.0f, 0.0f), float2(0.0f, 2.0f));
        _220 = takes_half2x2_bh22(_218);
    }
    else
    {
        _220 = false;
    }
    bool _225 = false;
    if (_220)
    {
        float3x3 _223 = float3x3(float3(3.0f, 0.0f, 0.0f), float3(0.0f, 3.0f, 0.0f), float3(0.0f, 0.0f, 3.0f));
        _225 = takes_half3x3_bh33(_223);
    }
    else
    {
        _225 = false;
    }
    bool _230 = false;
    if (_225)
    {
        float4x4 _228 = float4x4(float4(4.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 4.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 4.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 4.0f));
        _230 = takes_half4x4_bh44(_228);
    }
    else
    {
        _230 = false;
    }
    bool _235 = false;
    if (_230)
    {
        bool _233 = true;
        _235 = takes_bool_bb(_233);
    }
    else
    {
        _235 = false;
    }
    bool _241 = false;
    if (_235)
    {
        bool2 _239 = bool2(true, true);
        _241 = takes_bool2_bb2(_239);
    }
    else
    {
        _241 = false;
    }
    bool _247 = false;
    if (_241)
    {
        bool3 _245 = bool3(true, true, true);
        _247 = takes_bool3_bb3(_245);
    }
    else
    {
        _247 = false;
    }
    bool _253 = false;
    if (_247)
    {
        bool4 _251 = bool4(true, true, true, true);
        _253 = takes_bool4_bb4(_251);
    }
    else
    {
        _253 = false;
    }
    bool _259 = false;
    if (_253)
    {
        int _257 = 1;
        _259 = takes_int_bi(_257);
    }
    else
    {
        _259 = false;
    }
    bool _266 = false;
    if (_259)
    {
        int2 _264 = int2(2, 2);
        _266 = takes_int2_bi2(_264);
    }
    else
    {
        _266 = false;
    }
    bool _273 = false;
    if (_266)
    {
        int3 _271 = int3(3, 3, 3);
        _273 = takes_int3_bi3(_271);
    }
    else
    {
        _273 = false;
    }
    bool _280 = false;
    if (_273)
    {
        int4 _278 = int4(4, 4, 4, 4);
        _280 = takes_int4_bi4(_278);
    }
    else
    {
        _280 = false;
    }
    float4 _281 = 0.0f.xxxx;
    if (_280)
    {
        _281 = _34_colorGreen;
    }
    else
    {
        _281 = _34_colorRed;
    }
    return _281;
}

void frag_main()
{
    float2 _44 = 0.0f.xx;
    sk_FragColor = main(_44);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
