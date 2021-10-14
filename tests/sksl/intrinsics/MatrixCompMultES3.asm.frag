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
OpName %h24 "h24"
OpName %h42 "h42"
OpName %f43 "f43"
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
OpDecorate %h24 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %h42 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
OpDecorate %96 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
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
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_9 = OpConstant %float 9
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_12 = OpConstant %float 12
%float_22 = OpConstant %float 22
%float_30 = OpConstant %float 30
%float_36 = OpConstant %float 36
%float_40 = OpConstant %float 40
%float_42 = OpConstant %float 42
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%h24 = OpVariable %_ptr_Function_mat2v4float Function
%h42 = OpVariable %_ptr_Function_mat4v2float Function
%f43 = OpVariable %_ptr_Function_mat4v3float Function
%185 = OpVariable %_ptr_Function_v4float Function
%31 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_9
%32 = OpCompositeConstruct %v4float %float_9 %float_9 %float_9 %float_9
%33 = OpCompositeConstruct %mat2v4float %31 %32
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%38 = OpLoad %v4float %34
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %39
%42 = OpCompositeConstruct %mat2v4float %38 %41
%43 = OpCompositeExtract %v4float %33 0
%44 = OpCompositeExtract %v4float %42 0
%45 = OpFMul %v4float %43 %44
%46 = OpCompositeExtract %v4float %33 1
%47 = OpCompositeExtract %v4float %42 1
%48 = OpFMul %v4float %46 %47
%49 = OpCompositeConstruct %mat2v4float %45 %48
OpStore %h24 %49
%62 = OpCompositeConstruct %v2float %float_1 %float_2
%63 = OpCompositeConstruct %v2float %float_3 %float_4
%64 = OpCompositeConstruct %v2float %float_5 %float_6
%65 = OpCompositeConstruct %v2float %float_7 %float_8
%66 = OpCompositeConstruct %mat4v2float %62 %63 %64 %65
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %67
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %68 0
%72 = OpCompositeExtract %float %68 1
%73 = OpCompositeConstruct %v2float %71 %72
%74 = OpCompositeExtract %float %68 2
%75 = OpCompositeExtract %float %68 3
%76 = OpCompositeConstruct %v2float %74 %75
%77 = OpCompositeExtract %float %70 0
%78 = OpCompositeExtract %float %70 1
%79 = OpCompositeConstruct %v2float %77 %78
%80 = OpCompositeExtract %float %70 2
%81 = OpCompositeExtract %float %70 3
%82 = OpCompositeConstruct %v2float %80 %81
%83 = OpCompositeConstruct %mat4v2float %73 %76 %79 %82
%84 = OpCompositeExtract %v2float %66 0
%85 = OpCompositeExtract %v2float %83 0
%86 = OpFMul %v2float %84 %85
%87 = OpCompositeExtract %v2float %66 1
%88 = OpCompositeExtract %v2float %83 1
%89 = OpFMul %v2float %87 %88
%90 = OpCompositeExtract %v2float %66 2
%91 = OpCompositeExtract %v2float %83 2
%92 = OpFMul %v2float %90 %91
%93 = OpCompositeExtract %v2float %66 3
%94 = OpCompositeExtract %v2float %83 3
%95 = OpFMul %v2float %93 %94
%96 = OpCompositeConstruct %mat4v2float %86 %89 %92 %95
OpStore %h42 %96
%107 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%108 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%109 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%110 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%111 = OpCompositeConstruct %mat4v3float %107 %108 %109 %110
OpStore %f43 %111
%113 = OpLoad %mat2v4float %h24
%114 = OpCompositeConstruct %v4float %float_9 %float_0 %float_0 %float_9
%115 = OpCompositeConstruct %v4float %float_0 %float_9 %float_0 %float_9
%116 = OpCompositeConstruct %mat2v4float %114 %115
%118 = OpCompositeExtract %v4float %113 0
%119 = OpCompositeExtract %v4float %116 0
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
%122 = OpCompositeExtract %v4float %113 1
%123 = OpCompositeExtract %v4float %116 1
%124 = OpFOrdEqual %v4bool %122 %123
%125 = OpAll %bool %124
%126 = OpLogicalAnd %bool %121 %125
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%129 = OpLoad %mat4v2float %h42
%130 = OpCompositeConstruct %v2float %float_1 %float_0
%131 = OpCompositeConstruct %v2float %float_0 %float_4
%132 = OpCompositeConstruct %v2float %float_0 %float_6
%133 = OpCompositeConstruct %v2float %float_0 %float_8
%134 = OpCompositeConstruct %mat4v2float %130 %131 %132 %133
%136 = OpCompositeExtract %v2float %129 0
%137 = OpCompositeExtract %v2float %134 0
%138 = OpFOrdEqual %v2bool %136 %137
%139 = OpAll %bool %138
%140 = OpCompositeExtract %v2float %129 1
%141 = OpCompositeExtract %v2float %134 1
%142 = OpFOrdEqual %v2bool %140 %141
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %139 %143
%145 = OpCompositeExtract %v2float %129 2
%146 = OpCompositeExtract %v2float %134 2
%147 = OpFOrdEqual %v2bool %145 %146
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %144 %148
%150 = OpCompositeExtract %v2float %129 3
%151 = OpCompositeExtract %v2float %134 3
%152 = OpFOrdEqual %v2bool %150 %151
%153 = OpAll %bool %152
%154 = OpLogicalAnd %bool %149 %153
OpBranch %128
%128 = OpLabel
%155 = OpPhi %bool %false %25 %154 %127
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %mat4v3float %f43
%159 = OpCompositeConstruct %v3float %float_12 %float_22 %float_30
%160 = OpCompositeConstruct %v3float %float_36 %float_40 %float_42
%161 = OpCompositeConstruct %v3float %float_42 %float_40 %float_36
%162 = OpCompositeConstruct %v3float %float_30 %float_22 %float_12
%163 = OpCompositeConstruct %mat4v3float %159 %160 %161 %162
%165 = OpCompositeExtract %v3float %158 0
%166 = OpCompositeExtract %v3float %163 0
%167 = OpFOrdEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpCompositeExtract %v3float %158 1
%170 = OpCompositeExtract %v3float %163 1
%171 = OpFOrdEqual %v3bool %169 %170
%172 = OpAll %bool %171
%173 = OpLogicalAnd %bool %168 %172
%174 = OpCompositeExtract %v3float %158 2
%175 = OpCompositeExtract %v3float %163 2
%176 = OpFOrdEqual %v3bool %174 %175
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %173 %177
%179 = OpCompositeExtract %v3float %158 3
%180 = OpCompositeExtract %v3float %163 3
%181 = OpFOrdEqual %v3bool %179 %180
%182 = OpAll %bool %181
%183 = OpLogicalAnd %bool %178 %182
OpBranch %157
%157 = OpLabel
%184 = OpPhi %bool %false %128 %183 %156
OpSelectionMerge %189 None
OpBranchConditional %184 %187 %188
%187 = OpLabel
%190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%191 = OpLoad %v4float %190
OpStore %185 %191
OpBranch %189
%188 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%193 = OpLoad %v4float %192
OpStore %185 %193
OpBranch %189
%189 = OpLabel
%194 = OpLoad %v4float %185
OpReturnValue %194
OpFunctionEnd
