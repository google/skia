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
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
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
%143 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpLoad %bool %_0_ok
OpSelectionMerge %37 None
OpBranchConditional %35 %36 %37
%36 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%42 = OpLoad %mat2v2float %38
OpStore %_1_tempMtx1 %47
%48 = OpLoad %v4float %_1_tempMtx1
%50 = OpCompositeExtract %float %48 0
%51 = OpCompositeExtract %float %48 1
%52 = OpCompositeExtract %float %48 2
%53 = OpCompositeExtract %float %48 3
%54 = OpCompositeConstruct %v2float %50 %51
%55 = OpCompositeConstruct %v2float %52 %53
%49 = OpCompositeConstruct %mat2v2float %54 %55
%57 = OpCompositeExtract %v2float %42 0
%58 = OpCompositeExtract %v2float %49 0
%59 = OpFOrdEqual %v2bool %57 %58
%60 = OpAll %bool %59
%61 = OpCompositeExtract %v2float %42 1
%62 = OpCompositeExtract %v2float %49 1
%63 = OpFOrdEqual %v2bool %61 %62
%64 = OpAll %bool %63
%65 = OpLogicalAnd %bool %60 %64
OpBranch %37
%37 = OpLabel
%66 = OpPhi %bool %false %28 %65 %36
OpStore %_0_ok %66
%67 = OpLoad %bool %_0_ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%73 = OpLoad %mat3v3float %70
%80 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%81 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%82 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%79 = OpCompositeConstruct %mat3v3float %80 %81 %82
%84 = OpCompositeExtract %v3float %73 0
%85 = OpCompositeExtract %v3float %79 0
%86 = OpFOrdEqual %v3bool %84 %85
%87 = OpAll %bool %86
%88 = OpCompositeExtract %v3float %73 1
%89 = OpCompositeExtract %v3float %79 1
%90 = OpFOrdEqual %v3bool %88 %89
%91 = OpAll %bool %90
%92 = OpLogicalAnd %bool %87 %91
%93 = OpCompositeExtract %v3float %73 2
%94 = OpCompositeExtract %v3float %79 2
%95 = OpFOrdEqual %v3bool %93 %94
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %92 %96
OpBranch %69
%69 = OpLabel
%98 = OpPhi %bool %false %37 %97 %68
OpStore %_0_ok %98
%99 = OpLoad %bool %_0_ok
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%103 = OpLoad %mat2v2float %102
%106 = OpCompositeConstruct %v2float %float_100 %float_0
%107 = OpCompositeConstruct %v2float %float_0 %float_100
%105 = OpCompositeConstruct %mat2v2float %106 %107
%108 = OpCompositeExtract %v2float %103 0
%109 = OpCompositeExtract %v2float %105 0
%110 = OpFOrdNotEqual %v2bool %108 %109
%111 = OpAny %bool %110
%112 = OpCompositeExtract %v2float %103 1
%113 = OpCompositeExtract %v2float %105 1
%114 = OpFOrdNotEqual %v2bool %112 %113
%115 = OpAny %bool %114
%116 = OpLogicalOr %bool %111 %115
OpBranch %101
%101 = OpLabel
%117 = OpPhi %bool %false %69 %116 %100
OpStore %_0_ok %117
%118 = OpLoad %bool %_0_ok
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%122 = OpLoad %mat3v3float %121
%124 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%125 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%126 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%123 = OpCompositeConstruct %mat3v3float %124 %125 %126
%127 = OpCompositeExtract %v3float %122 0
%128 = OpCompositeExtract %v3float %123 0
%129 = OpFOrdNotEqual %v3bool %127 %128
%130 = OpAny %bool %129
%131 = OpCompositeExtract %v3float %122 1
%132 = OpCompositeExtract %v3float %123 1
%133 = OpFOrdNotEqual %v3bool %131 %132
%134 = OpAny %bool %133
%135 = OpLogicalOr %bool %130 %134
%136 = OpCompositeExtract %v3float %122 2
%137 = OpCompositeExtract %v3float %123 2
%138 = OpFOrdNotEqual %v3bool %136 %137
%139 = OpAny %bool %138
%140 = OpLogicalOr %bool %135 %139
OpBranch %120
%120 = OpLabel
%141 = OpPhi %bool %false %101 %140 %119
OpStore %_0_ok %141
%142 = OpLoad %bool %_0_ok
OpSelectionMerge %146 None
OpBranchConditional %142 %144 %145
%144 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%150 = OpLoad %v4float %147
OpStore %143 %150
OpBranch %146
%145 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%153 = OpLoad %v4float %151
OpStore %143 %153
OpBranch %146
%146 = OpLabel
%154 = OpLoad %v4float %143
OpReturnValue %154
OpFunctionEnd
