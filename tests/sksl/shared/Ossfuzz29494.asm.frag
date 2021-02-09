OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %m "m"
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
%12 = OpTypeFunction %void
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%int_959548497 = OpConstant %int 959548497
%int_0 = OpConstant %int 0
%18 = OpConstantComposite %v2int %int_959548497 %int_0
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%m = OpFunction %void None %12
%13 = OpLabel
OpReturn
OpFunctionEnd
%main = OpFunction %void None %12
%19 = OpLabel
%20 = OpFunctionCall %void %m
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
