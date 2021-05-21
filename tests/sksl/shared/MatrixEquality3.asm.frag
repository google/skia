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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_tempMtx1 "_1_tempMtx1"
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
OpDecorate %_1_tempMtx1 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%47 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_tempMtx1 = OpVariable %_ptr_Function_v4float Function
%140 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpLoad %bool %_0_ok
OpSelectionMerge %37 None
OpBranchConditional %35 %36 %37
%36 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%42 = OpLoad %mat2v2float %38
OpStore %_1_tempMtx1 %47
%48 = OpLoad %v4float %_1_tempMtx1
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpLoad %v4float %_1_tempMtx1
%51 = OpVectorShuffle %v2float %50 %50 2 3
%52 = OpCompositeConstruct %mat2v2float %49 %51
%54 = OpCompositeExtract %v2float %42 0
%55 = OpCompositeExtract %v2float %52 0
%56 = OpFOrdEqual %v2bool %54 %55
%57 = OpAll %bool %56
%58 = OpCompositeExtract %v2float %42 1
%59 = OpCompositeExtract %v2float %52 1
%60 = OpFOrdEqual %v2bool %58 %59
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %57 %61
OpBranch %37
%37 = OpLabel
%63 = OpPhi %bool %false %28 %62 %36
OpStore %_0_ok %63
%64 = OpLoad %bool %_0_ok
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%70 = OpLoad %mat3v3float %67
%77 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%78 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%79 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%76 = OpCompositeConstruct %mat3v3float %77 %78 %79
%81 = OpCompositeExtract %v3float %70 0
%82 = OpCompositeExtract %v3float %76 0
%83 = OpFOrdEqual %v3bool %81 %82
%84 = OpAll %bool %83
%85 = OpCompositeExtract %v3float %70 1
%86 = OpCompositeExtract %v3float %76 1
%87 = OpFOrdEqual %v3bool %85 %86
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %84 %88
%90 = OpCompositeExtract %v3float %70 2
%91 = OpCompositeExtract %v3float %76 2
%92 = OpFOrdEqual %v3bool %90 %91
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %89 %93
OpBranch %66
%66 = OpLabel
%95 = OpPhi %bool %false %37 %94 %65
OpStore %_0_ok %95
%96 = OpLoad %bool %_0_ok
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%100 = OpLoad %mat2v2float %99
%103 = OpCompositeConstruct %v2float %float_100 %float_0
%104 = OpCompositeConstruct %v2float %float_0 %float_100
%102 = OpCompositeConstruct %mat2v2float %103 %104
%105 = OpCompositeExtract %v2float %100 0
%106 = OpCompositeExtract %v2float %102 0
%107 = OpFOrdNotEqual %v2bool %105 %106
%108 = OpAny %bool %107
%109 = OpCompositeExtract %v2float %100 1
%110 = OpCompositeExtract %v2float %102 1
%111 = OpFOrdNotEqual %v2bool %109 %110
%112 = OpAny %bool %111
%113 = OpLogicalOr %bool %108 %112
OpBranch %98
%98 = OpLabel
%114 = OpPhi %bool %false %66 %113 %97
OpStore %_0_ok %114
%115 = OpLoad %bool %_0_ok
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%119 = OpLoad %mat3v3float %118
%121 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%122 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%123 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%120 = OpCompositeConstruct %mat3v3float %121 %122 %123
%124 = OpCompositeExtract %v3float %119 0
%125 = OpCompositeExtract %v3float %120 0
%126 = OpFOrdNotEqual %v3bool %124 %125
%127 = OpAny %bool %126
%128 = OpCompositeExtract %v3float %119 1
%129 = OpCompositeExtract %v3float %120 1
%130 = OpFOrdNotEqual %v3bool %128 %129
%131 = OpAny %bool %130
%132 = OpLogicalOr %bool %127 %131
%133 = OpCompositeExtract %v3float %119 2
%134 = OpCompositeExtract %v3float %120 2
%135 = OpFOrdNotEqual %v3bool %133 %134
%136 = OpAny %bool %135
%137 = OpLogicalOr %bool %132 %136
OpBranch %117
%117 = OpLabel
%138 = OpPhi %bool %false %98 %137 %116
OpStore %_0_ok %138
%139 = OpLoad %bool %_0_ok
OpSelectionMerge %143 None
OpBranchConditional %139 %141 %142
%141 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%147 = OpLoad %v4float %144
OpStore %140 %147
OpBranch %143
%142 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%150 = OpLoad %v4float %148
OpStore %140 %150
OpBranch %143
%143 = OpLabel
%151 = OpLoad %v4float %140
OpReturnValue %151
OpFunctionEnd
