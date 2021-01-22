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
OpDecorate %25 RelaxedPrecision
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
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%main = OpFunction %void None %11
%12 = OpLabel
%r = OpVariable %_ptr_Function_float Function
%g = OpVariable %_ptr_Function_float Function
%16 = OpExtInst %float %1 Sqrt %float_1
OpStore %r %16
%18 = OpExtInst %float %1 Sqrt %float_0
OpStore %g %18
%20 = OpLoad %float %r
%21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %21 %20
%25 = OpLoad %float %g
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %26 %25
OpReturn
OpFunctionEnd
