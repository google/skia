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
OpName %a "a"
OpName %b "b"
OpName %c "c"
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
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
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
%23 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%26 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%true = OpConstantTrue %bool
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
OpStore %a %23
OpStore %b %26
%29 = OpLoad %v4float %b
%28 = OpExtInst %v4float %1 FAbs %29
OpStore %c %28
%31 = OpLoad %v4float %a
%32 = OpLoad %v4float %b
%33 = OpFOrdEqual %v4bool %31 %32
%35 = OpAll %bool %33
OpSelectionMerge %37 None
OpBranchConditional %35 %37 %36
%36 = OpLabel
%38 = OpLoad %v4float %b
%39 = OpLoad %v4float %c
%40 = OpFOrdNotEqual %v4bool %38 %39
%41 = OpAny %bool %40
OpBranch %37
%37 = OpLabel
%42 = OpPhi %bool %true %19 %41 %36
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %46
OpReturnValue %50
%44 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %51
OpReturnValue %53
%45 = OpLabel
OpUnreachable
OpFunctionEnd
