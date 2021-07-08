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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inv2x2 "inv2x2"
OpName %inv3x3 "inv3x3"
OpName %inv4x4 "inv4x4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %inv2x2 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_n0_5 = OpConstant %float -0.5
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n24 = OpConstant %float -24
%float_18 = OpConstant %float 18
%float_5 = OpConstant %float 5
%float_20 = OpConstant %float 20
%float_n15 = OpConstant %float -15
%float_n4 = OpConstant %float -4
%float_n5 = OpConstant %float -5
%float_4 = OpConstant %float 4
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_n8 = OpConstant %float -8
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
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
%inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
%inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
%inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
%159 = OpVariable %_ptr_Function_v4float Function
%33 = OpCompositeConstruct %v2float %float_n2 %float_1
%34 = OpCompositeConstruct %v2float %float_1_5 %float_n0_5
%35 = OpCompositeConstruct %mat2v2float %33 %34
OpStore %inv2x2 %35
%48 = OpCompositeConstruct %v3float %float_n24 %float_18 %float_5
%49 = OpCompositeConstruct %v3float %float_20 %float_n15 %float_n4
%50 = OpCompositeConstruct %v3float %float_n5 %float_4 %float_1
%51 = OpCompositeConstruct %mat3v3float %48 %49 %50
OpStore %inv3x3 %51
%60 = OpCompositeConstruct %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%61 = OpCompositeConstruct %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%62 = OpCompositeConstruct %v4float %float_n8 %float_n1 %float_2 %float_2
%63 = OpCompositeConstruct %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%64 = OpCompositeConstruct %mat4v4float %60 %61 %62 %63
OpStore %inv4x4 %64
%66 = OpCompositeConstruct %v2float %float_n2 %float_1
%67 = OpCompositeConstruct %v2float %float_1_5 %float_n0_5
%68 = OpCompositeConstruct %mat2v2float %66 %67
%69 = OpLoad %mat2v2float %inv2x2
%71 = OpCompositeExtract %v2float %68 0
%72 = OpCompositeExtract %v2float %69 0
%73 = OpFOrdEqual %v2bool %71 %72
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v2float %68 1
%76 = OpCompositeExtract %v2float %69 1
%77 = OpFOrdEqual %v2bool %75 %76
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %74 %78
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpCompositeConstruct %v3float %float_n24 %float_18 %float_5
%83 = OpCompositeConstruct %v3float %float_20 %float_n15 %float_n4
%84 = OpCompositeConstruct %v3float %float_n5 %float_4 %float_1
%85 = OpCompositeConstruct %mat3v3float %82 %83 %84
%86 = OpLoad %mat3v3float %inv3x3
%88 = OpCompositeExtract %v3float %85 0
%89 = OpCompositeExtract %v3float %86 0
%90 = OpFOrdEqual %v3bool %88 %89
%91 = OpAll %bool %90
%92 = OpCompositeExtract %v3float %85 1
%93 = OpCompositeExtract %v3float %86 1
%94 = OpFOrdEqual %v3bool %92 %93
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %91 %95
%97 = OpCompositeExtract %v3float %85 2
%98 = OpCompositeExtract %v3float %86 2
%99 = OpFOrdEqual %v3bool %97 %98
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %96 %100
OpBranch %81
%81 = OpLabel
%102 = OpPhi %bool %false %25 %101 %80
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpCompositeConstruct %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%106 = OpCompositeConstruct %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%107 = OpCompositeConstruct %v4float %float_n8 %float_n1 %float_2 %float_2
%108 = OpCompositeConstruct %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%109 = OpCompositeConstruct %mat4v4float %105 %106 %107 %108
%110 = OpLoad %mat4v4float %inv4x4
%112 = OpCompositeExtract %v4float %109 0
%113 = OpCompositeExtract %v4float %110 0
%114 = OpFOrdEqual %v4bool %112 %113
%115 = OpAll %bool %114
%116 = OpCompositeExtract %v4float %109 1
%117 = OpCompositeExtract %v4float %110 1
%118 = OpFOrdEqual %v4bool %116 %117
%119 = OpAll %bool %118
%120 = OpLogicalAnd %bool %115 %119
%121 = OpCompositeExtract %v4float %109 2
%122 = OpCompositeExtract %v4float %110 2
%123 = OpFOrdEqual %v4bool %121 %122
%124 = OpAll %bool %123
%125 = OpLogicalAnd %bool %120 %124
%126 = OpCompositeExtract %v4float %109 3
%127 = OpCompositeExtract %v4float %110 3
%128 = OpFOrdEqual %v4bool %126 %127
%129 = OpAll %bool %128
%130 = OpLogicalAnd %bool %125 %129
OpBranch %104
%104 = OpLabel
%131 = OpPhi %bool %false %81 %130 %103
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%139 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%140 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%141 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%142 = OpCompositeConstruct %mat3v3float %139 %140 %141
%134 = OpExtInst %mat3v3float %1 MatrixInverse %142
%143 = OpLoad %mat3v3float %inv3x3
%144 = OpCompositeExtract %v3float %134 0
%145 = OpCompositeExtract %v3float %143 0
%146 = OpFOrdNotEqual %v3bool %144 %145
%147 = OpAny %bool %146
%148 = OpCompositeExtract %v3float %134 1
%149 = OpCompositeExtract %v3float %143 1
%150 = OpFOrdNotEqual %v3bool %148 %149
%151 = OpAny %bool %150
%152 = OpLogicalOr %bool %147 %151
%153 = OpCompositeExtract %v3float %134 2
%154 = OpCompositeExtract %v3float %143 2
%155 = OpFOrdNotEqual %v3bool %153 %154
%156 = OpAny %bool %155
%157 = OpLogicalOr %bool %152 %156
OpBranch %133
%133 = OpLabel
%158 = OpPhi %bool %false %104 %157 %132
OpSelectionMerge %163 None
OpBranchConditional %158 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%168 = OpLoad %v4float %164
OpStore %159 %168
OpBranch %163
%162 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%171 = OpLoad %v4float %169
OpStore %159 %171
OpBranch %163
%163 = OpLabel
%172 = OpLoad %v4float %159
OpReturnValue %172
OpFunctionEnd
