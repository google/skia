OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %r "r"
OpName %g "g"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_1_0 = OpConstant %float 1
%main = OpFunction %void None %11
%12 = OpLabel
%r = OpVariable %_ptr_Function_float Function
%g = OpVariable %_ptr_Function_float Function
%16 = OpExtInst %float %1 Sqrt %float_1
OpStore %r %16
%18 = OpExtInst %float %1 Sqrt %float_0
OpStore %g %18
%20 = OpLoad %float %r
%21 = OpLoad %float %g
%23 = OpCompositeConstruct %v4float %20 %21 %float_1_0 %float_1_0
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
