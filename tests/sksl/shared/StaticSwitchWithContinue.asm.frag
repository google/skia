OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %result "result"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%41 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
OpStore %x %int_0
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpLoad %int %x
%34 = OpSLessThanEqual %bool %32 %int_1
OpBranchConditional %34 %29 %31
%29 = OpLabel
%35 = OpExtInst %float %1 FAbs %float_2
OpStore %result %35
OpBranch %30
%30 = OpLabel
%37 = OpLoad %int %x
%38 = OpIAdd %int %37 %int_1
OpStore %x %38
OpBranch %27
%31 = OpLabel
%39 = OpLoad %float %result
%40 = OpFOrdEqual %bool %39 %float_2
OpSelectionMerge %45 None
OpBranchConditional %40 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %46
OpStore %41 %48
OpBranch %45
%44 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %49
OpStore %41 %50
OpBranch %45
%45 = OpLabel
%51 = OpLoad %v4float %41
OpReturnValue %51
OpFunctionEnd
