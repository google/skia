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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %28 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%22 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%131 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%28 = OpLoad %bool %_0_ok
OpSelectionMerge %30 None
OpBranchConditional %28 %29 %30
%29 = OpLabel
%31 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%35 = OpLoad %mat2v2float %31
%41 = OpCompositeConstruct %v2float %float_1 %float_2
%42 = OpCompositeConstruct %v2float %float_3 %float_4
%40 = OpCompositeConstruct %mat2v2float %41 %42
%44 = OpCompositeExtract %v2float %35 0
%45 = OpCompositeExtract %v2float %40 0
%46 = OpFOrdEqual %v2bool %44 %45
%47 = OpAll %bool %46
%48 = OpCompositeExtract %v2float %35 1
%49 = OpCompositeExtract %v2float %40 1
%50 = OpFOrdEqual %v2bool %48 %49
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %47 %51
OpBranch %30
%30 = OpLabel
%53 = OpPhi %bool %false %23 %52 %29
OpStore %_0_ok %53
%54 = OpLoad %bool %_0_ok
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%60 = OpLoad %mat3v3float %57
%67 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%68 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%69 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%66 = OpCompositeConstruct %mat3v3float %67 %68 %69
%71 = OpCompositeExtract %v3float %60 0
%72 = OpCompositeExtract %v3float %66 0
%73 = OpFOrdEqual %v3bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v3float %60 1
%76 = OpCompositeExtract %v3float %66 1
%77 = OpFOrdEqual %v3bool %75 %76
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %74 %78
%80 = OpCompositeExtract %v3float %60 2
%81 = OpCompositeExtract %v3float %66 2
%82 = OpFOrdEqual %v3bool %80 %81
%83 = OpAll %bool %82
%84 = OpLogicalAnd %bool %79 %83
OpBranch %56
%56 = OpLabel
%85 = OpPhi %bool %false %30 %84 %55
OpStore %_0_ok %85
%86 = OpLoad %bool %_0_ok
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%90 = OpLoad %mat2v2float %89
%94 = OpCompositeConstruct %v2float %float_100 %float_0
%95 = OpCompositeConstruct %v2float %float_0 %float_100
%92 = OpCompositeConstruct %mat2v2float %94 %95
%96 = OpCompositeExtract %v2float %90 0
%97 = OpCompositeExtract %v2float %92 0
%98 = OpFOrdNotEqual %v2bool %96 %97
%99 = OpAny %bool %98
%100 = OpCompositeExtract %v2float %90 1
%101 = OpCompositeExtract %v2float %92 1
%102 = OpFOrdNotEqual %v2bool %100 %101
%103 = OpAny %bool %102
%104 = OpLogicalOr %bool %99 %103
OpBranch %88
%88 = OpLabel
%105 = OpPhi %bool %false %56 %104 %87
OpStore %_0_ok %105
%106 = OpLoad %bool %_0_ok
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%110 = OpLoad %mat3v3float %109
%112 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%113 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%114 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%111 = OpCompositeConstruct %mat3v3float %112 %113 %114
%115 = OpCompositeExtract %v3float %110 0
%116 = OpCompositeExtract %v3float %111 0
%117 = OpFOrdNotEqual %v3bool %115 %116
%118 = OpAny %bool %117
%119 = OpCompositeExtract %v3float %110 1
%120 = OpCompositeExtract %v3float %111 1
%121 = OpFOrdNotEqual %v3bool %119 %120
%122 = OpAny %bool %121
%123 = OpLogicalOr %bool %118 %122
%124 = OpCompositeExtract %v3float %110 2
%125 = OpCompositeExtract %v3float %111 2
%126 = OpFOrdNotEqual %v3bool %124 %125
%127 = OpAny %bool %126
%128 = OpLogicalOr %bool %123 %127
OpBranch %108
%108 = OpLabel
%129 = OpPhi %bool %false %88 %128 %107
OpStore %_0_ok %129
%130 = OpLoad %bool %_0_ok
OpSelectionMerge %135 None
OpBranchConditional %130 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%139 = OpLoad %v4float %136
OpStore %131 %139
OpBranch %135
%134 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%142 = OpLoad %v4float %140
OpStore %131 %142
OpBranch %135
%135 = OpLabel
%143 = OpLoad %v4float %131
OpReturnValue %143
OpFunctionEnd
