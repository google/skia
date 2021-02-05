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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%26 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%v2float = OpTypeVector %float 2
%28 = OpConstantComposite %v2float %float_2 %float_0
%float_5 = OpConstant %float 5
%32 = OpConstantComposite %v2float %float_3 %float_5
%36 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%40 = OpVariable %_ptr_Function_v4float Function
OpStore %result %26
%29 = OpLoad %v4float %result
%30 = OpVectorShuffle %v4float %29 %28 4 1 5 3
OpStore %result %30
%33 = OpLoad %v4float %result
%34 = OpVectorShuffle %v4float %33 %32 0 4 2 5
OpStore %result %34
%35 = OpLoad %v4float %result
%37 = OpFOrdEqual %v4bool %35 %36
%39 = OpAll %bool %37
OpSelectionMerge %43 None
OpBranchConditional %39 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %44
OpStore %40 %48
OpBranch %43
%42 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%51 = OpLoad %v4float %49
OpStore %40 %51
OpBranch %43
%43 = OpLabel
%52 = OpLoad %v4float %40
OpReturnValue %52
OpFunctionEnd
