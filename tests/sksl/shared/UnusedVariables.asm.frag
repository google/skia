OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %b "b"
OpName %d "d"
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
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%main = OpFunction %void None %11
%12 = OpLabel
%b = OpVariable %_ptr_Function_float Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %d %float_3
%18 = OpLoad %float %b
%20 = OpFAdd %float %18 %float_1
OpStore %b %20
%21 = OpLoad %float %d
%22 = OpFAdd %float %21 %float_1
OpStore %d %22
%23 = OpLoad %float %b
%24 = OpLoad %float %b
%25 = OpLoad %float %d
%26 = OpLoad %float %d
%27 = OpCompositeConstruct %v4float %23 %24 %25 %26
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
