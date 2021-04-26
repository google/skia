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
OpDecorate %26 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %m44 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
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
%19 = OpTypeFunction %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_23 = OpConstant %float 23
%float_0 = OpConstant %float 0
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_24 = OpConstant %float 24
%v2float = OpTypeVector %float 2
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
%100 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %19
%20 = OpLabel
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
%m44 = OpVariable %_ptr_Function_mat4v4float Function
%28 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%29 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%26 = OpCompositeConstruct %mat2v3float %28 %29
OpStore %m23 %26
%35 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%36 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%34 = OpCompositeConstruct %mat2v4float %35 %36
OpStore %m24 %34
%43 = OpCompositeConstruct %v2float %float_32 %float_0
%44 = OpCompositeConstruct %v2float %float_0 %float_32
%45 = OpCompositeConstruct %v2float %float_0 %float_0
%42 = OpCompositeConstruct %mat3v2float %43 %44 %45
OpStore %m32 %42
%51 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%52 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%53 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%50 = OpCompositeConstruct %mat3v4float %51 %52 %53
OpStore %m34 %50
%59 = OpCompositeConstruct %v2float %float_42 %float_0
%60 = OpCompositeConstruct %v2float %float_0 %float_42
%61 = OpCompositeConstruct %v2float %float_0 %float_0
%62 = OpCompositeConstruct %v2float %float_0 %float_0
%58 = OpCompositeConstruct %mat4v2float %59 %60 %61 %62
OpStore %m42 %58
%68 = OpCompositeConstruct %v3float %float_44 %float_0 %float_0
%69 = OpCompositeConstruct %v3float %float_0 %float_44 %float_0
%70 = OpCompositeConstruct %v3float %float_0 %float_0 %float_44
%71 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%67 = OpCompositeConstruct %mat4v3float %68 %69 %70 %71
OpStore %m43 %67
%75 = OpLoad %mat3v2float %m32
%76 = OpLoad %mat2v3float %m23
%77 = OpMatrixTimesMatrix %mat2v2float %75 %76
OpStore %m22 %77
%78 = OpLoad %mat2v2float %m22
%79 = OpLoad %mat2v2float %m22
%80 = OpMatrixTimesMatrix %mat2v2float %78 %79
OpStore %m22 %80
%84 = OpLoad %mat4v3float %m43
%85 = OpLoad %mat3v4float %m34
%86 = OpMatrixTimesMatrix %mat3v3float %84 %85
OpStore %m33 %86
%87 = OpLoad %mat3v3float %m33
%88 = OpLoad %mat3v3float %m33
%89 = OpMatrixTimesMatrix %mat3v3float %87 %88
OpStore %m33 %89
%93 = OpLoad %mat2v4float %m24
%94 = OpLoad %mat4v2float %m42
%95 = OpMatrixTimesMatrix %mat4v4float %93 %94
OpStore %m44 %95
%96 = OpLoad %mat4v4float %m44
%97 = OpLoad %mat4v4float %m44
%98 = OpMatrixTimesMatrix %mat4v4float %96 %97
OpStore %m44 %98
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %100
%101 = OpLabel
%_0_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_1_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_2_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_3_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_4_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_5_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_6_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m33 = OpVariable %_ptr_Function_mat3v3float Function
%_8_m44 = OpVariable %_ptr_Function_mat4v4float Function
%158 = OpVariable %_ptr_Function_v4float Function
%104 = OpCompositeConstruct %v3float %float_23 %float_0 %float_0
%105 = OpCompositeConstruct %v3float %float_0 %float_23 %float_0
%103 = OpCompositeConstruct %mat2v3float %104 %105
OpStore %_0_m23 %103
%108 = OpCompositeConstruct %v4float %float_24 %float_0 %float_0 %float_0
%109 = OpCompositeConstruct %v4float %float_0 %float_24 %float_0 %float_0
%107 = OpCompositeConstruct %mat2v4float %108 %109
OpStore %_1_m24 %107
%112 = OpCompositeConstruct %v2float %float_32 %float_0
%113 = OpCompositeConstruct %v2float %float_0 %float_32
%114 = OpCompositeConstruct %v2float %float_0 %float_0
%111 = OpCompositeConstruct %mat3v2float %112 %113 %114
OpStore %_2_m32 %111
%117 = OpCompositeConstruct %v4float %float_34 %float_0 %float_0 %float_0
%118 = OpCompositeConstruct %v4float %float_0 %float_34 %float_0 %float_0
%119 = OpCompositeConstruct %v4float %float_0 %float_0 %float_34 %float_0
%116 = OpCompositeConstruct %mat3v4float %117 %118 %119
OpStore %_3_m34 %116
%122 = OpCompositeConstruct %v2float %float_42 %float_0
%123 = OpCompositeConstruct %v2float %float_0 %float_42
%124 = OpCompositeConstruct %v2float %float_0 %float_0
%125 = OpCompositeConstruct %v2float %float_0 %float_0
%121 = OpCompositeConstruct %mat4v2float %122 %123 %124 %125
OpStore %_4_m42 %121
%128 = OpCompositeConstruct %v3float %float_44 %float_0 %float_0
%129 = OpCompositeConstruct %v3float %float_0 %float_44 %float_0
%130 = OpCompositeConstruct %v3float %float_0 %float_0 %float_44
%131 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%127 = OpCompositeConstruct %mat4v3float %128 %129 %130 %131
OpStore %_5_m43 %127
%133 = OpLoad %mat3v2float %_2_m32
%134 = OpLoad %mat2v3float %_0_m23
%135 = OpMatrixTimesMatrix %mat2v2float %133 %134
OpStore %_6_m22 %135
%136 = OpLoad %mat2v2float %_6_m22
%137 = OpLoad %mat2v2float %_6_m22
%138 = OpMatrixTimesMatrix %mat2v2float %136 %137
OpStore %_6_m22 %138
%140 = OpLoad %mat4v3float %_5_m43
%141 = OpLoad %mat3v4float %_3_m34
%142 = OpMatrixTimesMatrix %mat3v3float %140 %141
OpStore %_7_m33 %142
%143 = OpLoad %mat3v3float %_7_m33
%144 = OpLoad %mat3v3float %_7_m33
%145 = OpMatrixTimesMatrix %mat3v3float %143 %144
OpStore %_7_m33 %145
%147 = OpLoad %mat2v4float %_1_m24
%148 = OpLoad %mat4v2float %_4_m42
%149 = OpMatrixTimesMatrix %mat4v4float %147 %148
OpStore %_8_m44 %149
%150 = OpLoad %mat4v4float %_8_m44
%151 = OpLoad %mat4v4float %_8_m44
%152 = OpMatrixTimesMatrix %mat4v4float %150 %151
OpStore %_8_m44 %152
OpSelectionMerge %155 None
OpBranchConditional %true %154 %155
%154 = OpLabel
%156 = OpFunctionCall %bool %test_half_b
OpBranch %155
%155 = OpLabel
%157 = OpPhi %bool %false %101 %156 %154
OpSelectionMerge %162 None
OpBranchConditional %157 %160 %161
%160 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%167 = OpLoad %v4float %163
OpStore %158 %167
OpBranch %162
%161 = OpLabel
%168 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%170 = OpLoad %v4float %168
OpStore %158 %170
OpBranch %162
%162 = OpLabel
%171 = OpLoad %v4float %158
OpReturnValue %171
OpFunctionEnd
