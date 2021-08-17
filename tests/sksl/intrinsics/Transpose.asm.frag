OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "testMatrix3x3"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %testMatrix2x3 "testMatrix2x3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 80
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 96
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_1 = OpConstant %int 1
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
%115 = OpVariable %_ptr_Function_v4float Function
%38 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%39 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%40 = OpCompositeConstruct %mat2v3float %38 %39
OpStore %testMatrix2x3 %40
%43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%47 = OpLoad %mat2v2float %43
%42 = OpTranspose %mat2v2float %47
%48 = OpCompositeConstruct %v2float %float_1 %float_3
%49 = OpCompositeConstruct %v2float %float_2 %float_4
%50 = OpCompositeConstruct %mat2v2float %48 %49
%52 = OpCompositeExtract %v2float %42 0
%53 = OpCompositeExtract %v2float %50 0
%54 = OpFOrdEqual %v2bool %52 %53
%55 = OpAll %bool %54
%56 = OpCompositeExtract %v2float %42 1
%57 = OpCompositeExtract %v2float %50 1
%58 = OpFOrdEqual %v2bool %56 %57
%59 = OpAll %bool %58
%60 = OpLogicalAnd %bool %55 %59
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpLoad %mat2v3float %testMatrix2x3
%63 = OpTranspose %mat3v2float %64
%66 = OpCompositeConstruct %v2float %float_1 %float_4
%67 = OpCompositeConstruct %v2float %float_2 %float_5
%68 = OpCompositeConstruct %v2float %float_3 %float_6
%69 = OpCompositeConstruct %mat3v2float %66 %67 %68
%70 = OpCompositeExtract %v2float %63 0
%71 = OpCompositeExtract %v2float %69 0
%72 = OpFOrdEqual %v2bool %70 %71
%73 = OpAll %bool %72
%74 = OpCompositeExtract %v2float %63 1
%75 = OpCompositeExtract %v2float %69 1
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %73 %77
%79 = OpCompositeExtract %v2float %63 2
%80 = OpCompositeExtract %v2float %69 2
%81 = OpFOrdEqual %v2bool %79 %80
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %78 %82
OpBranch %62
%62 = OpLabel
%84 = OpPhi %bool %false %28 %83 %61
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_1
%91 = OpLoad %mat3v3float %88
%87 = OpTranspose %mat3v3float %91
%95 = OpCompositeConstruct %v3float %float_1 %float_4 %float_7
%96 = OpCompositeConstruct %v3float %float_2 %float_5 %float_8
%97 = OpCompositeConstruct %v3float %float_3 %float_6 %float_9
%98 = OpCompositeConstruct %mat3v3float %95 %96 %97
%100 = OpCompositeExtract %v3float %87 0
%101 = OpCompositeExtract %v3float %98 0
%102 = OpFOrdEqual %v3bool %100 %101
%103 = OpAll %bool %102
%104 = OpCompositeExtract %v3float %87 1
%105 = OpCompositeExtract %v3float %98 1
%106 = OpFOrdEqual %v3bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %103 %107
%109 = OpCompositeExtract %v3float %87 2
%110 = OpCompositeExtract %v3float %98 2
%111 = OpFOrdEqual %v3bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
OpBranch %86
%86 = OpLabel
%114 = OpPhi %bool %false %62 %113 %85
OpSelectionMerge %119 None
OpBranchConditional %114 %117 %118
%117 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%123 = OpLoad %v4float %120
OpStore %115 %123
OpBranch %119
%118 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%126 = OpLoad %v4float %124
OpStore %115 %126
OpBranch %119
%119 = OpLabel
%127 = OpLoad %v4float %115
OpReturnValue %127
OpFunctionEnd
