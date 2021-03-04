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
%float_1 = OpConstant %float 1
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%float_2 = OpConstant %float 2
%16 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_6 = OpConstant %float 6
%18 = OpConstantComposite %v4float %float_6 %float_6 %float_6 %float_6
%float_7 = OpConstant %float 7
%20 = OpConstantComposite %v4float %float_7 %float_7 %float_7 %float_7
%float_n8 = OpConstant %float -8
%22 = OpConstantComposite %v4float %float_n8 %float_n8 %float_n8 %float_n8
%float_n9 = OpConstant %float -9
%24 = OpConstantComposite %v4float %float_n9 %float_n9 %float_n9 %float_n9
%float_10 = OpConstant %float 10
%26 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%float_11 = OpConstant %float 11
%28 = OpConstantComposite %v4float %float_11 %float_11 %float_11 %float_11
%float_13 = OpConstant %float 13
%30 = OpConstantComposite %v4float %float_13 %float_13 %float_13 %float_13
%float_15 = OpConstant %float 15
%32 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%float_16 = OpConstant %float 16
%34 = OpConstantComposite %v4float %float_16 %float_16 %float_16 %float_16
%float_18 = OpConstant %float 18
%36 = OpConstantComposite %v4float %float_18 %float_18 %float_18 %float_18
%float_19 = OpConstant %float 19
%38 = OpConstantComposite %v4float %float_19 %float_19 %float_19 %float_19
%float_20 = OpConstant %float 20
%40 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%float_21 = OpConstant %float 21
%42 = OpConstantComposite %v4float %float_21 %float_21 %float_21 %float_21
%main = OpFunction %void None %11
%12 = OpLabel
OpStore %sk_FragColor %14
OpStore %sk_FragColor %16
OpStore %sk_FragColor %18
OpStore %sk_FragColor %20
OpStore %sk_FragColor %22
OpStore %sk_FragColor %24
OpStore %sk_FragColor %26
OpStore %sk_FragColor %28
OpStore %sk_FragColor %30
OpStore %sk_FragColor %32
OpStore %sk_FragColor %34
OpStore %sk_FragColor %36
OpStore %sk_FragColor %38
OpStore %sk_FragColor %40
OpStore %sk_FragColor %42
OpReturn
OpFunctionEnd
