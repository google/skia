static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int zero = 0;
static float globalArray[2] = { 0.0f, 0.0f };
static float2x2 globalMatrix = float2x2(0.0f.xx, 0.0f.xx);

float4 main(float2 _36)
{
    zero = 0;
    float _19[2] = { 1.0f, 1.0f };
    globalArray = _19;
    globalMatrix = float2x2(1.0f.xx, 1.0f.xx);
    float _40[2] = { 0.0f, 1.0f };
    float localArray[2] = _40;
    float2x2 localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
    return float4(globalArray[zero] * localArray[zero], 1.0f.xx[zero] * 1.0f.xx[zero], globalMatrix[zero] * localMatrix[zero]);
}

void frag_main()
{
    float2 _32 = 0.0f.xx;
    float4 _34 = main(_32);
    sk_FragColor = _34;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
