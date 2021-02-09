OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %test_float "test_float"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %test_int "test_int"
OpName %one_0 "one"
OpName %two_0 "two"
OpName %result_0 "result"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%20 = OpTypeFunction %bool
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%30 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%v2float = OpTypeVector %float 2
%34 = OpConstantComposite %v2float %float_n1 %float_n1
%37 = OpConstantComposite %v2float %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%50 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%float_0 = OpConstant %float 0
%_ptr_Function_int = OpTypePointer Function %int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n1 = OpConstant %int -1
%117 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n1 %int_n1
%v2int = OpTypeVector %int 2
%121 = OpConstantComposite %v2int %int_n1 %int_n1
%124 = OpConstantComposite %v2int %int_1 %int_1
%132 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%v3int = OpTypeVector %int 3
%184 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%test_float = OpFunction %bool None %20
%21 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%two = OpVariable %_ptr_Function_float Function
%result = OpVariable %_ptr_Function_v4float Function
OpStore %one %float_1
OpStore %two %float_2
%32 = OpFNegate %v2float %34
%35 = OpCompositeExtract %float %32 0
%36 = OpCompositeExtract %float %32 1
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeConstruct %v4float %35 %36 %38 %39
%31 = OpFNegate %v4float %40
%41 = OpFOrdEqual %v4bool %30 %31
%43 = OpAll %bool %41
%44 = OpSelect %int %43 %int_1 %int_0
%48 = OpConvertSToF %float %44
%49 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %49 %48
%51 = OpFNegate %v4float %50
%52 = OpFOrdNotEqual %v4bool %50 %51
%53 = OpAny %bool %52
%54 = OpSelect %int %53 %int_1 %int_0
%55 = OpConvertSToF %float %54
%56 = OpAccessChain %_ptr_Function_float %result %int_1
OpStore %56 %55
%58 = OpLoad %float %two
%59 = OpCompositeConstruct %v4float %58 %58 %58 %58
%57 = OpFNegate %v4float %59
%61 = OpLoad %float %two
%60 = OpFNegate %float %61
%63 = OpLoad %float %two
%62 = OpFNegate %float %63
%64 = OpCompositeConstruct %v3float %62 %62 %62
%66 = OpCompositeExtract %float %64 0
%67 = OpCompositeExtract %float %64 1
%68 = OpCompositeExtract %float %64 2
%69 = OpCompositeConstruct %v4float %60 %66 %67 %68
%70 = OpFOrdEqual %v4bool %57 %69
%71 = OpAll %bool %70
%72 = OpSelect %int %71 %int_1 %int_0
%73 = OpConvertSToF %float %72
%74 = OpAccessChain %_ptr_Function_float %result %int_2
OpStore %74 %73
%78 = OpLoad %float %one
%77 = OpFNegate %float %78
%79 = OpLoad %float %one
%80 = OpLoad %float %one
%81 = OpFAdd %float %79 %80
%82 = OpCompositeConstruct %v2float %77 %81
%76 = OpFNegate %v2float %82
%84 = OpLoad %float %one
%85 = OpLoad %float %two
%86 = OpFSub %float %84 %85
%87 = OpLoad %float %two
%88 = OpCompositeConstruct %v2float %86 %87
%83 = OpFNegate %v2float %88
%89 = OpFOrdEqual %v2bool %76 %83
%91 = OpAll %bool %89
%92 = OpSelect %int %91 %int_1 %int_0
%93 = OpConvertSToF %float %92
%94 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %94 %93
%96 = OpLoad %v4float %result
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v4float %result
%99 = OpCompositeExtract %float %98 1
%100 = OpFMul %float %97 %99
%101 = OpLoad %v4float %result
%102 = OpCompositeExtract %float %101 2
%103 = OpFMul %float %100 %102
%104 = OpLoad %v4float %result
%105 = OpCompositeExtract %float %104 3
%106 = OpFMul %float %103 %105
%107 = OpFUnordNotEqual %bool %106 %float_0
OpReturnValue %107
OpFunctionEnd
%test_int = OpFunction %bool None %20
%109 = OpLabel
%one_0 = OpVariable %_ptr_Function_int Function
%two_0 = OpVariable %_ptr_Function_int Function
%result_0 = OpVariable %_ptr_Function_v4int Function
OpStore %one_0 %int_1
OpStore %two_0 %int_2
%119 = OpSNegate %v2int %121
%122 = OpCompositeExtract %int %119 0
%123 = OpCompositeExtract %int %119 1
%125 = OpCompositeExtract %int %124 0
%126 = OpCompositeExtract %int %124 1
%127 = OpCompositeConstruct %v4int %122 %123 %125 %126
%118 = OpSNegate %v4int %127
%128 = OpIEqual %v4bool %117 %118
%129 = OpAll %bool %128
%130 = OpSelect %int %129 %int_1 %int_0
%131 = OpAccessChain %_ptr_Function_int %result_0 %int_0
OpStore %131 %130
%133 = OpSNegate %v4int %132
%134 = OpINotEqual %v4bool %132 %133
%135 = OpAny %bool %134
%136 = OpSelect %int %135 %int_1 %int_0
%137 = OpAccessChain %_ptr_Function_int %result_0 %int_1
OpStore %137 %136
%139 = OpLoad %int %two_0
%140 = OpCompositeConstruct %v4int %139 %139 %139 %139
%138 = OpSNegate %v4int %140
%142 = OpLoad %int %two_0
%141 = OpSNegate %int %142
%144 = OpLoad %int %two_0
%143 = OpSNegate %int %144
%145 = OpCompositeConstruct %v3int %143 %143 %143
%147 = OpCompositeExtract %int %145 0
%148 = OpCompositeExtract %int %145 1
%149 = OpCompositeExtract %int %145 2
%150 = OpCompositeConstruct %v4int %141 %147 %148 %149
%151 = OpIEqual %v4bool %138 %150
%152 = OpAll %bool %151
%153 = OpSelect %int %152 %int_1 %int_0
%154 = OpAccessChain %_ptr_Function_int %result_0 %int_2
OpStore %154 %153
%157 = OpLoad %int %one_0
%156 = OpSNegate %int %157
%158 = OpLoad %int %one_0
%159 = OpLoad %int %one_0
%160 = OpIAdd %int %158 %159
%161 = OpCompositeConstruct %v2int %156 %160
%155 = OpSNegate %v2int %161
%163 = OpLoad %int %one_0
%164 = OpLoad %int %two_0
%165 = OpISub %int %163 %164
%166 = OpLoad %int %two_0
%167 = OpCompositeConstruct %v2int %165 %166
%162 = OpSNegate %v2int %167
%168 = OpIEqual %v2bool %155 %162
%169 = OpAll %bool %168
%170 = OpSelect %int %169 %int_1 %int_0
%171 = OpAccessChain %_ptr_Function_int %result_0 %int_3
OpStore %171 %170
%172 = OpLoad %v4int %result_0
%173 = OpCompositeExtract %int %172 0
%174 = OpLoad %v4int %result_0
%175 = OpCompositeExtract %int %174 1
%176 = OpIMul %int %173 %175
%177 = OpLoad %v4int %result_0
%178 = OpCompositeExtract %int %177 2
%179 = OpIMul %int %176 %178
%180 = OpLoad %v4int %result_0
%181 = OpCompositeExtract %int %180 3
%182 = OpIMul %int %179 %181
%183 = OpINotEqual %bool %182 %int_0
OpReturnValue %183
OpFunctionEnd
%main = OpFunction %v4float None %184
%185 = OpLabel
%192 = OpVariable %_ptr_Function_v4float Function
%187 = OpFunctionCall %bool %test_float
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpFunctionCall %bool %test_int
OpBranch %189
%189 = OpLabel
%191 = OpPhi %bool %false %185 %190 %188
OpSelectionMerge %195 None
OpBranchConditional %191 %193 %194
%193 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%198 = OpLoad %v4float %196
OpStore %192 %198
OpBranch %195
%194 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%200 = OpLoad %v4float %199
OpStore %192 %200
OpBranch %195
%195 = OpLabel
%201 = OpLoad %v4float %192
OpReturnValue %201
OpFunctionEnd
