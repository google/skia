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
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%35 = OpAccessChain %_ptr_Function_v4float %m44 %int_0
%36 = OpLoad %v4float %35
%37 = OpLoad %v4float %v4
%38 = OpCompositeExtract %float %37 0
%39 = OpVectorTimesScalar %v4float %36 %38
%41 = OpAccessChain %_ptr_Function_v4float %m44 %int_1
%42 = OpLoad %v4float %41
%43 = OpLoad %v4float %v4
%44 = OpCompositeExtract %float %43 1
%45 = OpVectorTimesScalar %v4float %42 %44
%46 = OpFAdd %v4float %39 %45
%48 = OpAccessChain %_ptr_Function_v4float %m44 %int_2
%49 = OpLoad %v4float %48
%50 = OpLoad %v4float %v4
%51 = OpCompositeExtract %float %50 2
%52 = OpVectorTimesScalar %v4float %49 %51
%53 = OpFAdd %v4float %46 %52
%55 = OpAccessChain %_ptr_Function_v4float %m44 %int_3
%56 = OpLoad %v4float %55
%57 = OpLoad %v4float %v4
%58 = OpCompositeExtract %float %57 3
%59 = OpVectorTimesScalar %v4float %56 %58
%60 = OpFAdd %v4float %53 %59
OpReturnValue %60
OpFunctionEnd
