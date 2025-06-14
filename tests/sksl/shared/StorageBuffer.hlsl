struct SomeData
{
    float4 a;
    float2 b;
};

RWByteAddressBuffer _7 : register(u0, space0);
RWByteAddressBuffer _16 : register(u1, space0);

static float4 sk_FragColor;
static int bufferIndex;

struct SPIRV_Cross_Input
{
    nointerpolation int bufferIndex : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _32)
{
    SomeData _41 = { 0.0f.xxxx, 0.0f.xx };
    _41.a = asfloat(_7.Load4(_7.Load(0) * 32 + 16));
    _41.b = asfloat(_7.Load2(_7.Load(0) * 32 + 32));
    _16.Store4(_7.Load(0) * 32 + 0, asuint(_41.a));
    _16.Store2(_7.Load(0) * 32 + 16, asuint(_41.b));
    return asfloat(_7.Load4(bufferIndex * 32 + 16)) * asfloat(_7.Load2(bufferIndex * 32 + 32)).x;
}

void frag_main()
{
    float2 _28 = 0.0f.xx;
    float4 _30 = main(_28);
    sk_FragColor = _30;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    bufferIndex = stage_input.bufferIndex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
