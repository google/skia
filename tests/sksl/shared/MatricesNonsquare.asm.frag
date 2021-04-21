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
OpName %test_half_b "test_half_b"
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %m44 "m44"
OpName %main "main"
OpName %_0_m23 "_0_m23"
OpName %_1_m24 "_1_m24"
OpName %_2_m32 "_2_m32"
OpName %_3_m34 "_3_m34"
OpName %_4_m42 "_4_m42"
OpName %_5_m43 "_5_m43"
OpName %_6_m22 "_6_m22"
OpName %_7_m33 "_7_m33"
OpName %_8_m44 "_8_m44"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %m44 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_23 = OpConstant %float 23
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_24 = OpConstant %float 24
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_32 = OpConstant %float 32
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_34 = OpConstant %float 34
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_42 = OpConstant %float 42
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_44 = OpConstant %float 44
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%103 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
%m44 = OpVariable %_ptr_Function_mat4v4float Function
%32 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%33 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%31 = OpCompositeConstruct %mat2v3float %32 %33
OpStore %m23 %31
%39 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%40 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%38 = OpCompositeConstruct %mat2v4float %39 %40
OpStore %m24 %38
%46 = OpCompositeConstruct %v2float %float_32 %float_0
%47 = OpCompositeConstruct %v2float %float_0 %float_32
%48 = OpCompositeConstruct %v2float %float_0 %float_0
%45 = OpCompositeConstruct %mat3v2float %46 %47 %48
OpStore %m32 %45
%54 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%56 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%53 = OpCompositeConstruct %mat3v4float %54 %55 %56
OpStore %m34 %53
%62 = OpCompositeConstruct %v2float %float_42 %float_0
%63 = OpCompositeConstruct %v2float %float_0 %float_42
%64 = OpCompositeConstruct %v2float %float_0 %float_0
%65 = OpCompositeConstruct %v2float %float_0 %float_0
%61 = OpCompositeConstruct %mat4v2float %62 %63 %64 %65
OpStore %m42 %61
%71 = OpCompositeConstruct %v3float %float_44 %float_0 %float_0
%72 = OpCompositeConstruct %v3float %float_0 %float_44 %float_0
%73 = OpCompositeConstruct %v3float %float_0 %float_0 %float_44
%74 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%70 = OpCompositeConstruct %mat4v3float %71 %72 %73 %74
OpStore %m43 %70
%78 = OpLoad %mat3v2float %m32
%79 = OpLoad %mat2v3float %m23
%80 = OpMatrixTimesMatrix %mat2v2float %78 %79
OpStore %m22 %80
%81 = OpLoad %mat2v2float %m22
%82 = OpLoad %mat2v2float %m22
%83 = OpMatrixTimesMatrix %mat2v2float %81 %82
OpStore %m22 %83
%87 = OpLoad %mat4v3float %m43
%88 = OpLoad %mat3v4float %m34
%89 = OpMatrixTimesMatrix %mat3v3float %87 %88
OpStore %m33 %89
%90 = OpLoad %mat3v3float %m33
%91 = OpLoad %mat3v3float %m33
%92 = OpMatrixTimesMatrix %mat3v3float %90 %91
OpStore %m33 %92
%96 = OpLoad %mat2v4float %m24
%97 = OpLoad %mat4v2float %m42
%98 = OpMatrixTimesMatrix %mat4v4float %96 %97
OpStore %m44 %98
%99 = OpLoad %mat4v4float %m44
%100 = OpLoad %mat4v4float %m44
%101 = OpMatrixTimesMatrix %mat4v4float %99 %100
OpStore %m44 %101
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %103
%104 = OpFunctionParameter %_ptr_Function_v2float
%105 = OpLabel
%_0_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_1_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_2_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_3_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_4_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_5_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_6_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_8_m44 = OpVariable %_ptr_Function_mat4v4float Function
%162 = OpVariable %_ptr_Function_v4float Function
%108 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%109 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%107 = OpCompositeConstruct %mat2v3float %108 %109
OpStore %_0_m23 %107
%112 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%113 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%111 = OpCompositeConstruct %mat2v4float %112 %113
OpStore %_1_m24 %111
%116 = OpCompositeConstruct %v2float %float_32 %float_0
%117 = OpCompositeConstruct %v2float %float_0 %float_32
%118 = OpCompositeConstruct %v2float %float_0 %float_0
%115 = OpCompositeConstruct %mat3v2float %116 %117 %118
OpStore %_2_m32 %115
%121 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%123 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%120 = OpCompositeConstruct %mat3v4float %121 %122 %123
OpStore %_3_m34 %120
%126 = OpCompositeConstruct %v2float %float_42 %float_0
%127 = OpCompositeConstruct %v2float %float_0 %float_42
%128 = OpCompositeConstruct %v2float %float_0 %float_0
%129 = OpCompositeConstruct %v2float %float_0 %float_0
%125 = OpCompositeConstruct %mat4v2float %126 %127 %128 %129
OpStore %_4_m42 %125
%132 = OpCompositeConstruct %v3float %float_44 %float_0 %float_0
%133 = OpCompositeConstruct %v3float %float_0 %float_44 %float_0
%134 = OpCompositeConstruct %v3float %float_0 %float_0 %float_44
%135 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%131 = OpCompositeConstruct %mat4v3float %132 %133 %134 %135
OpStore %_5_m43 %131
%137 = OpLoad %mat3v2float %_2_m32
%138 = OpLoad %mat2v3float %_0_m23
%139 = OpMatrixTimesMatrix %mat2v2float %137 %138
OpStore %_6_m22 %139
%140 = OpLoad %mat2v2float %_6_m22
%141 = OpLoad %mat2v2float %_6_m22
%142 = OpMatrixTimesMatrix %mat2v2float %140 %141
OpStore %_6_m22 %142
%144 = OpLoad %mat4v3float %_5_m43
%145 = OpLoad %mat3v4float %_3_m34
%146 = OpMatrixTimesMatrix %mat3v3float %144 %145
OpStore %_7_m33 %146
%147 = OpLoad %mat3v3float %_7_m33
%148 = OpLoad %mat3v3float %_7_m33
%149 = OpMatrixTimesMatrix %mat3v3float %147 %148
OpStore %_7_m33 %149
%151 = OpLoad %mat2v4float %_1_m24
%152 = OpLoad %mat4v2float %_4_m42
%153 = OpMatrixTimesMatrix %mat4v4float %151 %152
OpStore %_8_m44 %153
%154 = OpLoad %mat4v4float %_8_m44
%155 = OpLoad %mat4v4float %_8_m44
%156 = OpMatrixTimesMatrix %mat4v4float %154 %155
OpStore %_8_m44 %156
OpSelectionMerge %159 None
OpBranchConditional %true %158 %159
%158 = OpLabel
%160 = OpFunctionCall %bool %test_half_b
OpBranch %159
%159 = OpLabel
%161 = OpPhi %bool %false %105 %160 %158
OpSelectionMerge %166 None
OpBranchConditional %161 %164 %165
%164 = OpLabel
%167 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%171 = OpLoad %v4float %167
OpStore %162 %171
OpBranch %166
%165 = OpLabel
%172 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%174 = OpLoad %v4float %172
OpStore %162 %174
OpBranch %166
%166 = OpLabel
%175 = OpLoad %v4float %162
OpReturnValue %175
OpFunctionEnd
