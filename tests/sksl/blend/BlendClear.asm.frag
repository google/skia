OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_clear "blend_clear"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%void = OpTypeVoid
%22 = OpTypeFunction %void
%blend_clear = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
OpReturnValue %20
OpFunctionEnd
%main = OpFunction %void None %22
%23 = OpLabel
%25 = OpVariable %_ptr_Function_v4float Function
%27 = OpVariable %_ptr_Function_v4float Function
%24 = OpLoad %v4float %src
OpStore %25 %24
%26 = OpLoad %v4float %dst
OpStore %27 %26
%28 = OpFunctionCall %v4float %blend_clear %25 %27
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
