OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %m44 "m44"
OpName %v4 "v4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %m44 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %v4 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_123 = OpConstant %float 123
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%32 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%m44 = OpVariable %_ptr_Function_mat4v4float Function
%v4 = OpVariable %_ptr_Function_v4float Function
%23 = OpCompositeConstruct %v4float %float_123 %float_0 %float_0 %float_0
%24 = OpCompositeConstruct %v4float %float_0 %float_123 %float_0 %float_0
%25 = OpCompositeConstruct %v4float %float_0 %float_0 %float_123 %float_0
%26 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_123
%21 = OpCompositeConstruct %mat4v4float %23 %24 %25 %26
OpStore %m44 %21
OpStore %v4 %32
%33 = OpLoad %mat4v4float %m44
%34 = OpLoad %v4float %v4
%35 = OpMatrixTimesVector %v4float %33 %34
OpReturnValue %35
OpFunctionEnd
