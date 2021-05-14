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
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
OpName %_9_m44 "_9_m44"
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
OpDecorate %85 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%false = OpConstantFalse %bool
%float_8 = OpConstant %float 8
%v2bool = OpTypeVector %bool 2
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_18 = OpConstant %float 18
%v4bool = OpTypeVector %bool 4
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_9_m44 = OpVariable %_ptr_Function_mat4v4float Function
%173 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %34
%42 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%43 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%41 = OpCompositeConstruct %mat2v4float %42 %43
OpStore %_2_m24 %41
%49 = OpCompositeConstruct %v2float %float_4 %float_0
%50 = OpCompositeConstruct %v2float %float_0 %float_4
%51 = OpCompositeConstruct %v2float %float_0 %float_0
%48 = OpCompositeConstruct %mat3v2float %49 %50 %51
OpStore %_3_m32 %48
%57 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%58 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%59 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%56 = OpCompositeConstruct %mat3v4float %57 %58 %59
OpStore %_4_m34 %56
%65 = OpCompositeConstruct %v2float %float_6 %float_0
%66 = OpCompositeConstruct %v2float %float_0 %float_6
%67 = OpCompositeConstruct %v2float %float_0 %float_0
%68 = OpCompositeConstruct %v2float %float_0 %float_0
%64 = OpCompositeConstruct %mat4v2float %65 %66 %67 %68
OpStore %_5_m42 %64
%74 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%75 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%76 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%77 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%73 = OpCompositeConstruct %mat4v3float %74 %75 %76 %77
OpStore %_6_m43 %73
%81 = OpLoad %mat3v2float %_3_m32
%82 = OpLoad %mat2v3float %_1_m23
%83 = OpMatrixTimesMatrix %mat2v2float %81 %82
OpStore %_7_m22 %83
%85 = OpLoad %bool %_0_ok
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %mat2v2float %_7_m22
%91 = OpCompositeConstruct %v2float %float_8 %float_0
%92 = OpCompositeConstruct %v2float %float_0 %float_8
%90 = OpCompositeConstruct %mat2v2float %91 %92
%94 = OpCompositeExtract %v2float %88 0
%95 = OpCompositeExtract %v2float %90 0
%96 = OpFOrdEqual %v2bool %94 %95
%97 = OpAll %bool %96
%98 = OpCompositeExtract %v2float %88 1
%99 = OpCompositeExtract %v2float %90 1
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpLogicalAnd %bool %97 %101
OpBranch %87
%87 = OpLabel
%103 = OpPhi %bool %false %25 %102 %86
OpStore %_0_ok %103
%107 = OpLoad %mat4v3float %_6_m43
%108 = OpLoad %mat3v4float %_4_m34
%109 = OpMatrixTimesMatrix %mat3v3float %107 %108
OpStore %_8_m33 %109
%110 = OpLoad %bool %_0_ok
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %mat3v3float %_8_m33
%116 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%117 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%118 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%115 = OpCompositeConstruct %mat3v3float %116 %117 %118
%120 = OpCompositeExtract %v3float %113 0
%121 = OpCompositeExtract %v3float %115 0
%122 = OpFOrdEqual %v3bool %120 %121
%123 = OpAll %bool %122
%124 = OpCompositeExtract %v3float %113 1
%125 = OpCompositeExtract %v3float %115 1
%126 = OpFOrdEqual %v3bool %124 %125
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %123 %127
%129 = OpCompositeExtract %v3float %113 2
%130 = OpCompositeExtract %v3float %115 2
%131 = OpFOrdEqual %v3bool %129 %130
%132 = OpAll %bool %131
%133 = OpLogicalAnd %bool %128 %132
OpBranch %112
%112 = OpLabel
%134 = OpPhi %bool %false %87 %133 %111
OpStore %_0_ok %134
%138 = OpLoad %mat2v4float %_2_m24
%139 = OpLoad %mat4v2float %_5_m42
%140 = OpMatrixTimesMatrix %mat4v4float %138 %139
OpStore %_9_m44 %140
%141 = OpLoad %bool %_0_ok
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpLoad %mat4v4float %_9_m44
%147 = OpCompositeConstruct %v4float %float_18 %float_0 %float_0 %float_0
%148 = OpCompositeConstruct %v4float %float_0 %float_18 %float_0 %float_0
%149 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%150 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%146 = OpCompositeConstruct %mat4v4float %147 %148 %149 %150
%152 = OpCompositeExtract %v4float %144 0
%153 = OpCompositeExtract %v4float %146 0
%154 = OpFOrdEqual %v4bool %152 %153
%155 = OpAll %bool %154
%156 = OpCompositeExtract %v4float %144 1
%157 = OpCompositeExtract %v4float %146 1
%158 = OpFOrdEqual %v4bool %156 %157
%159 = OpAll %bool %158
%160 = OpLogicalAnd %bool %155 %159
%161 = OpCompositeExtract %v4float %144 2
%162 = OpCompositeExtract %v4float %146 2
%163 = OpFOrdEqual %v4bool %161 %162
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %160 %164
%166 = OpCompositeExtract %v4float %144 3
%167 = OpCompositeExtract %v4float %146 3
%168 = OpFOrdEqual %v4bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
OpBranch %143
%143 = OpLabel
%171 = OpPhi %bool %false %112 %170 %142
OpStore %_0_ok %171
%172 = OpLoad %bool %_0_ok
OpSelectionMerge %177 None
OpBranchConditional %172 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%182 = OpLoad %v4float %178
OpStore %173 %182
OpBranch %177
%176 = OpLabel
%183 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%185 = OpLoad %v4float %183
OpStore %173 %185
OpBranch %177
%177 = OpLabel
%186 = OpLoad %v4float %173
OpReturnValue %186
OpFunctionEnd
