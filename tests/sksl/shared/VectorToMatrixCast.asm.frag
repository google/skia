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
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%ok = OpVariable %_ptr_Function_bool Function
%89 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%30 = OpLoad %bool %ok
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%37 = OpLoad %v4float %33
%39 = OpCompositeExtract %float %37 0
%40 = OpCompositeExtract %float %37 1
%41 = OpCompositeExtract %float %37 2
%42 = OpCompositeExtract %float %37 3
%43 = OpCompositeConstruct %v2float %39 %40
%44 = OpCompositeConstruct %v2float %41 %42
%38 = OpCompositeConstruct %mat2v2float %43 %44
%50 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%51 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%49 = OpCompositeConstruct %mat2v2float %50 %51
%53 = OpCompositeExtract %v2float %38 0
%54 = OpCompositeExtract %v2float %49 0
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpCompositeExtract %v2float %38 1
%58 = OpCompositeExtract %v2float %49 1
%59 = OpFOrdEqual %v2bool %57 %58
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %56 %60
OpBranch %32
%32 = OpLabel
%62 = OpPhi %bool %false %25 %61 %31
OpStore %ok %62
%63 = OpLoad %bool %ok
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%67 = OpLoad %v4float %66
%69 = OpCompositeExtract %float %67 0
%70 = OpCompositeExtract %float %67 1
%71 = OpCompositeExtract %float %67 2
%72 = OpCompositeExtract %float %67 3
%73 = OpCompositeConstruct %v2float %69 %70
%74 = OpCompositeConstruct %v2float %71 %72
%68 = OpCompositeConstruct %mat2v2float %73 %74
%76 = OpCompositeConstruct %v2float %float_n1_25 %float_0
%77 = OpCompositeConstruct %v2float %float_0_75 %float_2_25
%75 = OpCompositeConstruct %mat2v2float %76 %77
%78 = OpCompositeExtract %v2float %68 0
%79 = OpCompositeExtract %v2float %75 0
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v2float %68 1
%83 = OpCompositeExtract %v2float %75 1
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
OpBranch %65
%65 = OpLabel
%87 = OpPhi %bool %false %32 %86 %64
OpStore %ok %87
%88 = OpLoad %bool %ok
OpSelectionMerge %93 None
OpBranchConditional %88 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%96 = OpLoad %v4float %94
OpStore %89 %96
OpBranch %93
%92 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%99 = OpLoad %v4float %97
OpStore %89 %99
OpBranch %93
%93 = OpLabel
%100 = OpLoad %v4float %89
OpReturnValue %100
OpFunctionEnd
