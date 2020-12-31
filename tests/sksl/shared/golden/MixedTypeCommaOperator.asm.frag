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
OpDecorate %sk_Clockwise RelaxedPrecision
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
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%24 = OpConstantComposite %v2float %float_1 %float_1
%float_1_0 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%28 = OpConstantComposite %v3float %float_1_0 %float_1_0 %float_1_0
%int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%int_3 = OpConstant %int 3
%main = OpFunction %void None %11
%12 = OpLabel
%14 = OpSelect %float %true %float_1 %float_0
%13 = OpConvertSToF %float %int_1
%20 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %20 %13
%23 = OpConvertSToF %float %int_1
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %26 %23
%27 = OpConvertSToF %float %int_1
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %31 %27
%35 = OpCompositeConstruct %v2float %float_1 %float_0
%36 = OpCompositeConstruct %v2float %float_0 %float_1
%34 = OpCompositeConstruct %mat2v2float %35 %36
%33 = OpConvertSToF %float %int_1
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %38 %33
OpReturn
OpFunctionEnd
