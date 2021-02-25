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
OpName %test_int "test_int"
OpName %one "one"
OpName %two "two"
OpName %result "result"
OpName %main "main"
OpName %_0_one "_0_one"
OpName %_1_two "_1_two"
OpName %_2_result "_2_result"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %114 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
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
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_n1 = OpConstant %int -1
%31 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n1 %int_n1
%v2int = OpTypeVector %int 2
%35 = OpConstantComposite %v2int %int_n1 %int_n1
%38 = OpConstantComposite %v2int %int_1 %int_1
%v4bool = OpTypeVector %bool 4
%int_0 = OpConstant %int 0
%48 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%v3int = OpTypeVector %int 3
%v2bool = OpTypeVector %bool 2
%int_3 = OpConstant %int 3
%102 = OpTypeFunction %v4float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%112 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%v2float = OpTypeVector %float 2
%116 = OpConstantComposite %v2float %float_n1 %float_n1
%119 = OpConstantComposite %v2float %float_1 %float_1
%128 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v3float = OpTypeVector %float 3
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int = OpFunction %bool None %19
%20 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%result = OpVariable %_ptr_Function_v4int Function
OpStore %one %int_1
OpStore %two %int_2
%33 = OpSNegate %v2int %35
%36 = OpCompositeExtract %int %33 0
%37 = OpCompositeExtract %int %33 1
%39 = OpCompositeExtract %int %38 0
%40 = OpCompositeExtract %int %38 1
%41 = OpCompositeConstruct %v4int %36 %37 %39 %40
%32 = OpSNegate %v4int %41
%42 = OpIEqual %v4bool %31 %32
%44 = OpAll %bool %42
%45 = OpSelect %int %44 %int_1 %int_0
%47 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %47 %45
%49 = OpSNegate %v4int %48
%50 = OpINotEqual %v4bool %48 %49
%51 = OpAny %bool %50
%52 = OpSelect %int %51 %int_1 %int_0
%53 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %53 %52
%55 = OpLoad %int %two
%56 = OpCompositeConstruct %v4int %55 %55 %55 %55
%54 = OpSNegate %v4int %56
%58 = OpLoad %int %two
%57 = OpSNegate %int %58
%60 = OpLoad %int %two
%59 = OpSNegate %int %60
%61 = OpCompositeConstruct %v3int %59 %59 %59
%63 = OpCompositeExtract %int %61 0
%64 = OpCompositeExtract %int %61 1
%65 = OpCompositeExtract %int %61 2
%66 = OpCompositeConstruct %v4int %57 %63 %64 %65
%67 = OpIEqual %v4bool %54 %66
%68 = OpAll %bool %67
%69 = OpSelect %int %68 %int_1 %int_0
%70 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %70 %69
%73 = OpLoad %int %one
%72 = OpSNegate %int %73
%74 = OpLoad %int %one
%75 = OpLoad %int %one
%76 = OpIAdd %int %74 %75
%77 = OpCompositeConstruct %v2int %72 %76
%71 = OpSNegate %v2int %77
%79 = OpLoad %int %one
%80 = OpLoad %int %two
%81 = OpISub %int %79 %80
%82 = OpLoad %int %two
%83 = OpCompositeConstruct %v2int %81 %82
%78 = OpSNegate %v2int %83
%84 = OpIEqual %v2bool %71 %78
%86 = OpAll %bool %84
%87 = OpSelect %int %86 %int_1 %int_0
%88 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %88 %87
%90 = OpLoad %v4int %result
%91 = OpCompositeExtract %int %90 0
%92 = OpLoad %v4int %result
%93 = OpCompositeExtract %int %92 1
%94 = OpIMul %int %91 %93
%95 = OpLoad %v4int %result
%96 = OpCompositeExtract %int %95 2
%97 = OpIMul %int %94 %96
%98 = OpLoad %v4int %result
%99 = OpCompositeExtract %int %98 3
%100 = OpIMul %int %97 %99
%101 = OpINotEqual %bool %100 %int_0
OpReturnValue %101
OpFunctionEnd
%main = OpFunction %v4float None %102
%103 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_result = OpVariable %_ptr_Function_v4float Function
%189 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
%114 = OpFNegate %v2float %116
%117 = OpCompositeExtract %float %114 0
%118 = OpCompositeExtract %float %114 1
%120 = OpCompositeExtract %float %119 0
%121 = OpCompositeExtract %float %119 1
%122 = OpCompositeConstruct %v4float %117 %118 %120 %121
%113 = OpFNegate %v4float %122
%123 = OpFOrdEqual %v4bool %112 %113
%124 = OpAll %bool %123
%125 = OpSelect %int %124 %int_1 %int_0
%126 = OpConvertSToF %float %125
%127 = OpAccessChain %_ptr_Function_float %_2_result %int_0
OpStore %127 %126
%129 = OpFNegate %v4float %128
%130 = OpFOrdNotEqual %v4bool %128 %129
%131 = OpAny %bool %130
%132 = OpSelect %int %131 %int_1 %int_0
%133 = OpConvertSToF %float %132
%134 = OpAccessChain %_ptr_Function_float %_2_result %int_1
OpStore %134 %133
%136 = OpLoad %float %_1_two
%137 = OpCompositeConstruct %v4float %136 %136 %136 %136
%135 = OpFNegate %v4float %137
%139 = OpLoad %float %_1_two
%138 = OpFNegate %float %139
%141 = OpLoad %float %_1_two
%140 = OpFNegate %float %141
%142 = OpCompositeConstruct %v3float %140 %140 %140
%144 = OpCompositeExtract %float %142 0
%145 = OpCompositeExtract %float %142 1
%146 = OpCompositeExtract %float %142 2
%147 = OpCompositeConstruct %v4float %138 %144 %145 %146
%148 = OpFOrdEqual %v4bool %135 %147
%149 = OpAll %bool %148
%150 = OpSelect %int %149 %int_1 %int_0
%151 = OpConvertSToF %float %150
%152 = OpAccessChain %_ptr_Function_float %_2_result %int_2
OpStore %152 %151
%155 = OpLoad %float %_0_one
%154 = OpFNegate %float %155
%156 = OpLoad %float %_0_one
%157 = OpLoad %float %_0_one
%158 = OpFAdd %float %156 %157
%159 = OpCompositeConstruct %v2float %154 %158
%153 = OpFNegate %v2float %159
%161 = OpLoad %float %_0_one
%162 = OpLoad %float %_1_two
%163 = OpFSub %float %161 %162
%164 = OpLoad %float %_1_two
%165 = OpCompositeConstruct %v2float %163 %164
%160 = OpFNegate %v2float %165
%166 = OpFOrdEqual %v2bool %153 %160
%167 = OpAll %bool %166
%168 = OpSelect %int %167 %int_1 %int_0
%169 = OpConvertSToF %float %168
%170 = OpAccessChain %_ptr_Function_float %_2_result %int_3
OpStore %170 %169
%172 = OpLoad %v4float %_2_result
%173 = OpCompositeExtract %float %172 0
%174 = OpLoad %v4float %_2_result
%175 = OpCompositeExtract %float %174 1
%176 = OpFMul %float %173 %175
%177 = OpLoad %v4float %_2_result
%178 = OpCompositeExtract %float %177 2
%179 = OpFMul %float %176 %178
%180 = OpLoad %v4float %_2_result
%181 = OpCompositeExtract %float %180 3
%182 = OpFMul %float %179 %181
%183 = OpFUnordNotEqual %bool %182 %float_0
OpSelectionMerge %186 None
OpBranchConditional %183 %185 %186
%185 = OpLabel
%187 = OpFunctionCall %bool %test_int
OpBranch %186
%186 = OpLabel
%188 = OpPhi %bool %false %103 %187 %185
OpSelectionMerge %192 None
OpBranchConditional %188 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%195 = OpLoad %v4float %193
OpStore %189 %195
OpBranch %192
%191 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%197 = OpLoad %v4float %196
OpStore %189 %197
OpBranch %192
%192 = OpLabel
%198 = OpLoad %v4float %189
OpReturnValue %198
OpFunctionEnd
