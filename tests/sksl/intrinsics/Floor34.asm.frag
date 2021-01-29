OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v3float = OpTypeVector %float 3
%float_n2 = OpConstant %float -2
%float_0 = OpConstant %float 0
%31 = OpConstantComposite %v3float %float_n2 %float_0 %float_0
%v3bool = OpTypeVector %bool 3
%float_2 = OpConstant %float 2
%41 = OpConstantComposite %v4float %float_n2 %float_0 %float_0 %float_2
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%46 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%21 = OpExtInst %v3float %1 Floor %27
%32 = OpFOrdEqual %v3bool %21 %31
%34 = OpAll %bool %32
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%37 = OpExtInst %v4float %1 Floor %39
%42 = OpFOrdEqual %v4bool %37 %41
%44 = OpAll %bool %42
OpBranch %36
%36 = OpLabel
%45 = OpPhi %bool %false %19 %44 %35
OpSelectionMerge %50 None
OpBranchConditional %45 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%53 = OpLoad %v4float %51
OpStore %46 %53
OpBranch %50
%49 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%56 = OpLoad %v4float %54
OpStore %46 %56
OpBranch %50
%50 = OpLabel
%57 = OpLoad %v4float %46
OpReturnValue %57
OpFunctionEnd
