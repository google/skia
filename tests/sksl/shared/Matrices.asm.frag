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
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m6 "m6"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_2_m1 "_2_m1"
OpName %_4_m3 "_4_m3"
OpName %_5_m4 "_5_m4"
OpName %_6_m5 "_6_m5"
OpName %_7_m6 "_7_m6"
OpName %_10_m10 "_10_m10"
OpName %_11_m11 "_11_m11"
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
OpDecorate %m1 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %m6 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%true = OpConstantTrue %bool
%97 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m6 = OpVariable %_ptr_Function_mat2v2float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
%34 = OpCompositeConstruct %v2float %float_1 %float_2
%35 = OpCompositeConstruct %v2float %float_3 %float_4
%33 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %m1 %33
%37 = OpLoad %mat2v2float %m1
OpStore %m3 %37
%40 = OpCompositeConstruct %v2float %float_1 %float_0
%41 = OpCompositeConstruct %v2float %float_0 %float_1
%39 = OpCompositeConstruct %mat2v2float %40 %41
OpStore %m4 %39
%42 = OpLoad %mat2v2float %m3
%43 = OpLoad %mat2v2float %m4
%44 = OpMatrixTimesMatrix %mat2v2float %42 %43
OpStore %m3 %44
%48 = OpAccessChain %_ptr_Function_v2float %m1 %int_0
%49 = OpLoad %v2float %48
%50 = OpCompositeExtract %float %49 0
%52 = OpCompositeConstruct %v2float %50 %float_0
%53 = OpCompositeConstruct %v2float %float_0 %50
%51 = OpCompositeConstruct %mat2v2float %52 %53
OpStore %m5 %51
%56 = OpCompositeConstruct %v2float %float_1 %float_2
%57 = OpCompositeConstruct %v2float %float_3 %float_4
%55 = OpCompositeConstruct %mat2v2float %56 %57
OpStore %m6 %55
%58 = OpLoad %mat2v2float %m6
%59 = OpLoad %mat2v2float %m5
%60 = OpCompositeExtract %v2float %58 0
%61 = OpCompositeExtract %v2float %59 0
%62 = OpFAdd %v2float %60 %61
%63 = OpCompositeExtract %v2float %58 1
%64 = OpCompositeExtract %v2float %59 1
%65 = OpFAdd %v2float %63 %64
%66 = OpCompositeConstruct %mat2v2float %62 %65
OpStore %m6 %66
%71 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%74 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%70 = OpCompositeConstruct %mat4v4float %71 %72 %73 %74
OpStore %m10 %70
%77 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%80 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%76 = OpCompositeConstruct %mat4v4float %77 %78 %79 %80
OpStore %m11 %76
%81 = OpLoad %mat4v4float %m11
%82 = OpLoad %mat4v4float %m10
%83 = OpCompositeExtract %v4float %81 0
%84 = OpCompositeExtract %v4float %82 0
%85 = OpFSub %v4float %83 %84
%86 = OpCompositeExtract %v4float %81 1
%87 = OpCompositeExtract %v4float %82 1
%88 = OpFSub %v4float %86 %87
%89 = OpCompositeExtract %v4float %81 2
%90 = OpCompositeExtract %v4float %82 2
%91 = OpFSub %v4float %89 %90
%92 = OpCompositeExtract %v4float %81 3
%93 = OpCompositeExtract %v4float %82 3
%94 = OpFSub %v4float %92 %93
%95 = OpCompositeConstruct %mat4v4float %85 %88 %91 %94
OpStore %m11 %95
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %97
%98 = OpFunctionParameter %_ptr_Function_v2float
%99 = OpLabel
%_2_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_10_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_11_m11 = OpVariable %_ptr_Function_mat4v4float Function
%165 = OpVariable %_ptr_Function_v4float Function
%102 = OpCompositeConstruct %v2float %float_1 %float_2
%103 = OpCompositeConstruct %v2float %float_3 %float_4
%101 = OpCompositeConstruct %mat2v2float %102 %103
OpStore %_2_m1 %101
%105 = OpLoad %mat2v2float %_2_m1
OpStore %_4_m3 %105
%108 = OpCompositeConstruct %v2float %float_1 %float_0
%109 = OpCompositeConstruct %v2float %float_0 %float_1
%107 = OpCompositeConstruct %mat2v2float %108 %109
OpStore %_5_m4 %107
%110 = OpLoad %mat2v2float %_4_m3
%111 = OpLoad %mat2v2float %_5_m4
%112 = OpMatrixTimesMatrix %mat2v2float %110 %111
OpStore %_4_m3 %112
%114 = OpAccessChain %_ptr_Function_v2float %_2_m1 %int_0
%115 = OpLoad %v2float %114
%116 = OpCompositeExtract %float %115 0
%118 = OpCompositeConstruct %v2float %116 %float_0
%119 = OpCompositeConstruct %v2float %float_0 %116
%117 = OpCompositeConstruct %mat2v2float %118 %119
OpStore %_6_m5 %117
%122 = OpCompositeConstruct %v2float %float_1 %float_2
%123 = OpCompositeConstruct %v2float %float_3 %float_4
%121 = OpCompositeConstruct %mat2v2float %122 %123
OpStore %_7_m6 %121
%124 = OpLoad %mat2v2float %_7_m6
%125 = OpLoad %mat2v2float %_6_m5
%126 = OpCompositeExtract %v2float %124 0
%127 = OpCompositeExtract %v2float %125 0
%128 = OpFAdd %v2float %126 %127
%129 = OpCompositeExtract %v2float %124 1
%130 = OpCompositeExtract %v2float %125 1
%131 = OpFAdd %v2float %129 %130
%132 = OpCompositeConstruct %mat2v2float %128 %131
OpStore %_7_m6 %132
%135 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%136 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%137 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%138 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%134 = OpCompositeConstruct %mat4v4float %135 %136 %137 %138
OpStore %_10_m10 %134
%141 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%142 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%143 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%144 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%140 = OpCompositeConstruct %mat4v4float %141 %142 %143 %144
OpStore %_11_m11 %140
%145 = OpLoad %mat4v4float %_11_m11
%146 = OpLoad %mat4v4float %_10_m10
%147 = OpCompositeExtract %v4float %145 0
%148 = OpCompositeExtract %v4float %146 0
%149 = OpFSub %v4float %147 %148
%150 = OpCompositeExtract %v4float %145 1
%151 = OpCompositeExtract %v4float %146 1
%152 = OpFSub %v4float %150 %151
%153 = OpCompositeExtract %v4float %145 2
%154 = OpCompositeExtract %v4float %146 2
%155 = OpFSub %v4float %153 %154
%156 = OpCompositeExtract %v4float %145 3
%157 = OpCompositeExtract %v4float %146 3
%158 = OpFSub %v4float %156 %157
%159 = OpCompositeConstruct %mat4v4float %149 %152 %155 %158
OpStore %_11_m11 %159
OpSelectionMerge %162 None
OpBranchConditional %true %161 %162
%161 = OpLabel
%163 = OpFunctionCall %bool %test_half_b
OpBranch %162
%162 = OpLabel
%164 = OpPhi %bool %false %99 %163 %161
OpSelectionMerge %169 None
OpBranchConditional %164 %167 %168
%167 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%172 = OpLoad %v4float %170
OpStore %165 %172
OpBranch %169
%168 = OpLabel
%173 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%175 = OpLoad %v4float %173
OpStore %165 %175
OpBranch %169
%169 = OpLabel
%176 = OpLoad %v4float %165
OpReturnValue %176
OpFunctionEnd
