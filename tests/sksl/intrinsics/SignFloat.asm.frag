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
OpDecorate %34 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%38 = OpConstantComposite %v2float %float_n1 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%51 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
%v3bool = OpTypeVector %bool 3
%61 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
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
%66 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FSign %27
%29 = OpFOrdEqual %bool %21 %float_n1
OpSelectionMerge %31 None
OpBranchConditional %29 %30 %31
%30 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %33
%35 = OpVectorShuffle %v2float %34 %34 0 1
%32 = OpExtInst %v2float %1 FSign %35
%39 = OpFOrdEqual %v2bool %32 %38
%41 = OpAll %bool %39
OpBranch %31
%31 = OpLabel
%42 = OpPhi %bool %false %19 %41 %30
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v3float %47 %47 0 1 2
%45 = OpExtInst %v3float %1 FSign %48
%52 = OpFOrdEqual %v3bool %45 %51
%54 = OpAll %bool %52
OpBranch %44
%44 = OpLabel
%55 = OpPhi %bool %false %31 %54 %43
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%58 = OpExtInst %v4float %1 FSign %60
%62 = OpFOrdEqual %v4bool %58 %61
%64 = OpAll %bool %62
OpBranch %57
%57 = OpLabel
%65 = OpPhi %bool %false %44 %64 %56
OpSelectionMerge %70 None
OpBranchConditional %65 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%73 = OpLoad %v4float %71
OpStore %66 %73
OpBranch %70
%69 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%76 = OpLoad %v4float %74
OpStore %66 %76
OpBranch %70
%70 = OpLabel
%77 = OpLoad %v4float %66
OpReturnValue %77
OpFunctionEnd
