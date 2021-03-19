OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %11
%12 = OpLabel
%17 = OpCompositeExtract %float %16 0
%18 = OpConvertFToS %int %17
%20 = OpCompositeExtract %float %16 1
%21 = OpConvertFToS %int %20
%22 = OpCompositeConstruct %v2int %18 %21
%13 = OpExtInst %v2int %1 SAbs %22
%24 = OpCompositeExtract %int %13 0
%25 = OpConvertSToF %float %24
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %26 %25
OpReturn
OpFunctionEnd
