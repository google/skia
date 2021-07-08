OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %main "main"
OpName %h22 "h22"
OpName %h44 "h44"
OpName %f43 "f43"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %h22 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %h44 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Private_mat3v3float = OpTypePointer Private %mat3v3float
%a = OpVariable %_ptr_Private_mat3v3float Private
%b = OpVariable %_ptr_Private_mat3v3float Private
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
%c = OpVariable %_ptr_Private_mat4v4float Private
%d = OpVariable %_ptr_Private_mat4v4float Private
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%23 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_0 = OpConstant %float 0
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_3 = OpConstant %float 3
%float_5_5 = OpConstant %float 5.5
%float_8 = OpConstant %float 8
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v4bool = OpTypeVector %bool 4
%v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%main = OpFunction %void None %23
%24 = OpLabel
%h22 = OpVariable %_ptr_Function_mat2v2float Function
%h44 = OpVariable %_ptr_Function_mat4v4float Function
%f43 = OpVariable %_ptr_Function_mat4v3float Function
%61 = OpVariable %_ptr_Function_mat3v3float Function
%83 = OpVariable %_ptr_Function_mat4v4float Function
%176 = OpVariable %_ptr_Function_v4float Function
%33 = OpCompositeConstruct %v2float %float_0 %float_5
%34 = OpCompositeConstruct %v2float %float_10 %float_15
%35 = OpCompositeConstruct %mat2v2float %33 %34
OpStore %h22 %35
%42 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%43 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%44 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%45 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%46 = OpCompositeConstruct %mat4v4float %42 %43 %44 %45
OpStore %h44 %46
%56 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%57 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%58 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%59 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%60 = OpCompositeConstruct %mat4v3float %56 %57 %58 %59
OpStore %f43 %60
%64 = OpLoad %mat3v3float %a
%65 = OpLoad %mat3v3float %b
%66 = OpCompositeExtract %v3float %64 0
%67 = OpCompositeExtract %v3float %65 0
%68 = OpFMul %v3float %66 %67
%69 = OpCompositeExtract %v3float %64 1
%70 = OpCompositeExtract %v3float %65 1
%71 = OpFMul %v3float %69 %70
%72 = OpCompositeExtract %v3float %64 2
%73 = OpCompositeExtract %v3float %65 2
%74 = OpFMul %v3float %72 %73
%75 = OpCompositeConstruct %mat3v3float %68 %71 %74
OpStore %61 %75
%78 = OpAccessChain %_ptr_Function_v3float %61 %int_0
%80 = OpLoad %v3float %78
%81 = OpLoad %v4float %sk_FragColor
%82 = OpVectorShuffle %v4float %81 %80 4 5 6 3
OpStore %sk_FragColor %82
%85 = OpLoad %mat4v4float %c
%86 = OpLoad %mat4v4float %d
%87 = OpCompositeExtract %v4float %85 0
%88 = OpCompositeExtract %v4float %86 0
%89 = OpFMul %v4float %87 %88
%90 = OpCompositeExtract %v4float %85 1
%91 = OpCompositeExtract %v4float %86 1
%92 = OpFMul %v4float %90 %91
%93 = OpCompositeExtract %v4float %85 2
%94 = OpCompositeExtract %v4float %86 2
%95 = OpFMul %v4float %93 %94
%96 = OpCompositeExtract %v4float %85 3
%97 = OpCompositeExtract %v4float %86 3
%98 = OpFMul %v4float %96 %97
%99 = OpCompositeConstruct %mat4v4float %89 %92 %95 %98
OpStore %83 %99
%100 = OpAccessChain %_ptr_Function_v4float %83 %int_0
%102 = OpLoad %v4float %100
OpStore %sk_FragColor %102
%104 = OpLoad %mat2v2float %h22
%105 = OpCompositeConstruct %v2float %float_0 %float_5
%106 = OpCompositeConstruct %v2float %float_10 %float_15
%107 = OpCompositeConstruct %mat2v2float %105 %106
%109 = OpCompositeExtract %v2float %104 0
%110 = OpCompositeExtract %v2float %107 0
%111 = OpFOrdEqual %v2bool %109 %110
%112 = OpAll %bool %111
%113 = OpCompositeExtract %v2float %104 1
%114 = OpCompositeExtract %v2float %107 1
%115 = OpFOrdEqual %v2bool %113 %114
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %112 %116
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpLoad %mat4v4float %h44
%121 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%123 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%124 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%125 = OpCompositeConstruct %mat4v4float %121 %122 %123 %124
%127 = OpCompositeExtract %v4float %120 0
%128 = OpCompositeExtract %v4float %125 0
%129 = OpFOrdEqual %v4bool %127 %128
%130 = OpAll %bool %129
%131 = OpCompositeExtract %v4float %120 1
%132 = OpCompositeExtract %v4float %125 1
%133 = OpFOrdEqual %v4bool %131 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %130 %134
%136 = OpCompositeExtract %v4float %120 2
%137 = OpCompositeExtract %v4float %125 2
%138 = OpFOrdEqual %v4bool %136 %137
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %135 %139
%141 = OpCompositeExtract %v4float %120 3
%142 = OpCompositeExtract %v4float %125 3
%143 = OpFOrdEqual %v4bool %141 %142
%144 = OpAll %bool %143
%145 = OpLogicalAnd %bool %140 %144
OpBranch %119
%119 = OpLabel
%146 = OpPhi %bool %false %24 %145 %118
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %mat4v3float %f43
%150 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%151 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%152 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%153 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%154 = OpCompositeConstruct %mat4v3float %150 %151 %152 %153
%156 = OpCompositeExtract %v3float %149 0
%157 = OpCompositeExtract %v3float %154 0
%158 = OpFOrdEqual %v3bool %156 %157
%159 = OpAll %bool %158
%160 = OpCompositeExtract %v3float %149 1
%161 = OpCompositeExtract %v3float %154 1
%162 = OpFOrdEqual %v3bool %160 %161
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %159 %163
%165 = OpCompositeExtract %v3float %149 2
%166 = OpCompositeExtract %v3float %154 2
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
%170 = OpCompositeExtract %v3float %149 3
%171 = OpCompositeExtract %v3float %154 3
%172 = OpFOrdEqual %v3bool %170 %171
%173 = OpAll %bool %172
%174 = OpLogicalAnd %bool %169 %173
OpBranch %148
%148 = OpLabel
%175 = OpPhi %bool %false %119 %174 %147
OpSelectionMerge %179 None
OpBranchConditional %175 %177 %178
%177 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%182 = OpLoad %v4float %180
OpStore %176 %182
OpBranch %179
%178 = OpLabel
%183 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%185 = OpLoad %v4float %183
OpStore %176 %185
OpBranch %179
%179 = OpLabel
%186 = OpLoad %v4float %176
OpStore %sk_FragColor %186
OpReturn
OpFunctionEnd
