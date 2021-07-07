OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%mat2v2float = OpTypeMatrix %v2float 2
%float_1 = OpConstant %float 1
%v2bool = OpTypeVector %bool 2
%int_2 = OpConstant %int 2
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%82 = OpVariable %_ptr_Function_v4float Function
%27 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%31 = OpLoad %v4float %27
%32 = OpCompositeExtract %float %31 0
%33 = OpCompositeExtract %float %31 1
%34 = OpCompositeExtract %float %31 2
%35 = OpCompositeExtract %float %31 3
%36 = OpCompositeConstruct %v2float %32 %33
%37 = OpCompositeConstruct %v2float %34 %35
%38 = OpCompositeConstruct %mat2v2float %36 %37
%42 = OpCompositeConstruct %v2float %float_1 %float_0
%43 = OpCompositeConstruct %v2float %float_0 %float_1
%41 = OpCompositeConstruct %mat2v2float %42 %43
%45 = OpCompositeExtract %v2float %38 0
%46 = OpCompositeExtract %v2float %41 0
%47 = OpFOrdEqual %v2bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %38 1
%50 = OpCompositeExtract %v2float %41 1
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%58 = OpLoad %v4float %56
%59 = OpCompositeExtract %float %58 0
%60 = OpCompositeExtract %float %58 1
%61 = OpCompositeExtract %float %58 2
%62 = OpCompositeExtract %float %58 3
%63 = OpCompositeConstruct %v2float %59 %60
%64 = OpCompositeConstruct %v2float %61 %62
%65 = OpCompositeConstruct %mat2v2float %63 %64
%70 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%71 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%69 = OpCompositeConstruct %mat2v2float %70 %71
%72 = OpCompositeExtract %v2float %65 0
%73 = OpCompositeExtract %v2float %69 0
%74 = OpFOrdEqual %v2bool %72 %73
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v2float %65 1
%77 = OpCompositeExtract %v2float %69 1
%78 = OpFOrdEqual %v2bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
OpBranch %55
%55 = OpLabel
%81 = OpPhi %bool %false %25 %80 %54
OpSelectionMerge %86 None
OpBranchConditional %81 %84 %85
%84 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%89 = OpLoad %v4float %87
OpStore %82 %89
OpBranch %86
%85 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%91 = OpLoad %v4float %90
OpStore %82 %91
OpBranch %86
%86 = OpLabel
%92 = OpLoad %v4float %82
OpReturnValue %92
OpFunctionEnd
