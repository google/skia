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
OpName %ok "ok"
OpName %m1 "m1"
OpName %m5 "m5"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m5 "_2_m5"
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
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
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
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%113 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %ok %true
%37 = OpCompositeConstruct %v2float %float_1 %float_2
%38 = OpCompositeConstruct %v2float %float_3 %float_4
%36 = OpCompositeConstruct %mat2v2float %37 %38
OpStore %m1 %36
%40 = OpLoad %bool %ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m1
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
%44 = OpCompositeConstruct %mat2v2float %45 %46
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %44 0
%50 = OpFOrdEqual %v2bool %48 %49
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %43 1
%53 = OpCompositeExtract %v2float %44 1
%54 = OpFOrdEqual %v2bool %52 %53
%55 = OpAll %bool %54
%56 = OpLogicalAnd %bool %51 %55
OpBranch %42
%42 = OpLabel
%57 = OpPhi %bool %false %25 %56 %41
OpStore %ok %57
%61 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%62 = OpLoad %v2float %61
%63 = OpCompositeExtract %float %62 1
%65 = OpCompositeConstruct %v2float %63 %float_0
%66 = OpCompositeConstruct %v2float %float_0 %63
%64 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %m5 %64
%67 = OpLoad %bool %ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %mat2v2float %m5
%72 = OpCompositeConstruct %v2float %float_4 %float_0
%73 = OpCompositeConstruct %v2float %float_0 %float_4
%71 = OpCompositeConstruct %mat2v2float %72 %73
%74 = OpCompositeExtract %v2float %70 0
%75 = OpCompositeExtract %v2float %71 0
%76 = OpFOrdEqual %v2bool %74 %75
%77 = OpAll %bool %76
%78 = OpCompositeExtract %v2float %70 1
%79 = OpCompositeExtract %v2float %71 1
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %77 %81
OpBranch %69
%69 = OpLabel
%83 = OpPhi %bool %false %42 %82 %68
OpStore %ok %83
%84 = OpLoad %mat2v2float %m1
%85 = OpLoad %mat2v2float %m5
%86 = OpCompositeExtract %v2float %84 0
%87 = OpCompositeExtract %v2float %85 0
%88 = OpFAdd %v2float %86 %87
%89 = OpCompositeExtract %v2float %84 1
%90 = OpCompositeExtract %v2float %85 1
%91 = OpFAdd %v2float %89 %90
%92 = OpCompositeConstruct %mat2v2float %88 %91
OpStore %m1 %92
%93 = OpLoad %bool %ok
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %mat2v2float %m1
%100 = OpCompositeConstruct %v2float %float_5 %float_2
%101 = OpCompositeConstruct %v2float %float_3 %float_8
%99 = OpCompositeConstruct %mat2v2float %100 %101
%102 = OpCompositeExtract %v2float %96 0
%103 = OpCompositeExtract %v2float %99 0
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpCompositeExtract %v2float %96 1
%107 = OpCompositeExtract %v2float %99 1
%108 = OpFOrdEqual %v2bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %105 %109
OpBranch %95
%95 = OpLabel
%111 = OpPhi %bool %false %69 %110 %94
OpStore %ok %111
%112 = OpLoad %bool %ok
OpReturnValue %112
OpFunctionEnd
%main = OpFunction %v4float None %113
%114 = OpFunctionParameter %_ptr_Function_v2float
%115 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m5 = OpVariable %_ptr_Function_mat2v2float Function
%193 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%119 = OpCompositeConstruct %v2float %float_1 %float_2
%120 = OpCompositeConstruct %v2float %float_3 %float_4
%118 = OpCompositeConstruct %mat2v2float %119 %120
OpStore %_1_m1 %118
%121 = OpLoad %bool %_0_ok
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %mat2v2float %_1_m1
%126 = OpCompositeConstruct %v2float %float_1 %float_2
%127 = OpCompositeConstruct %v2float %float_3 %float_4
%125 = OpCompositeConstruct %mat2v2float %126 %127
%128 = OpCompositeExtract %v2float %124 0
%129 = OpCompositeExtract %v2float %125 0
%130 = OpFOrdEqual %v2bool %128 %129
%131 = OpAll %bool %130
%132 = OpCompositeExtract %v2float %124 1
%133 = OpCompositeExtract %v2float %125 1
%134 = OpFOrdEqual %v2bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %131 %135
OpBranch %123
%123 = OpLabel
%137 = OpPhi %bool %false %115 %136 %122
OpStore %_0_ok %137
%139 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%140 = OpLoad %v2float %139
%141 = OpCompositeExtract %float %140 1
%143 = OpCompositeConstruct %v2float %141 %float_0
%144 = OpCompositeConstruct %v2float %float_0 %141
%142 = OpCompositeConstruct %mat2v2float %143 %144
OpStore %_2_m5 %142
%145 = OpLoad %bool %_0_ok
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpLoad %mat2v2float %_2_m5
%150 = OpCompositeConstruct %v2float %float_4 %float_0
%151 = OpCompositeConstruct %v2float %float_0 %float_4
%149 = OpCompositeConstruct %mat2v2float %150 %151
%152 = OpCompositeExtract %v2float %148 0
%153 = OpCompositeExtract %v2float %149 0
%154 = OpFOrdEqual %v2bool %152 %153
%155 = OpAll %bool %154
%156 = OpCompositeExtract %v2float %148 1
%157 = OpCompositeExtract %v2float %149 1
%158 = OpFOrdEqual %v2bool %156 %157
%159 = OpAll %bool %158
%160 = OpLogicalAnd %bool %155 %159
OpBranch %147
%147 = OpLabel
%161 = OpPhi %bool %false %123 %160 %146
OpStore %_0_ok %161
%162 = OpLoad %mat2v2float %_1_m1
%163 = OpLoad %mat2v2float %_2_m5
%164 = OpCompositeExtract %v2float %162 0
%165 = OpCompositeExtract %v2float %163 0
%166 = OpFAdd %v2float %164 %165
%167 = OpCompositeExtract %v2float %162 1
%168 = OpCompositeExtract %v2float %163 1
%169 = OpFAdd %v2float %167 %168
%170 = OpCompositeConstruct %mat2v2float %166 %169
OpStore %_1_m1 %170
%171 = OpLoad %bool %_0_ok
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %mat2v2float %_1_m1
%176 = OpCompositeConstruct %v2float %float_5 %float_2
%177 = OpCompositeConstruct %v2float %float_3 %float_8
%175 = OpCompositeConstruct %mat2v2float %176 %177
%178 = OpCompositeExtract %v2float %174 0
%179 = OpCompositeExtract %v2float %175 0
%180 = OpFOrdEqual %v2bool %178 %179
%181 = OpAll %bool %180
%182 = OpCompositeExtract %v2float %174 1
%183 = OpCompositeExtract %v2float %175 1
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %181 %185
OpBranch %173
%173 = OpLabel
%187 = OpPhi %bool %false %147 %186 %172
OpStore %_0_ok %187
%188 = OpLoad %bool %_0_ok
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpFunctionCall %bool %test_half_b
OpBranch %190
%190 = OpLabel
%192 = OpPhi %bool %false %173 %191 %189
OpSelectionMerge %197 None
OpBranchConditional %192 %195 %196
%195 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%201 = OpLoad %v4float %198
OpStore %193 %201
OpBranch %197
%196 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%203 = OpLoad %v4float %202
OpStore %193 %203
OpBranch %197
%197 = OpLabel
%204 = OpLoad %v4float %193
OpReturnValue %204
OpFunctionEnd
