OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint "_entrypoint"
OpName %fn "fn"
OpName %x "x"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%19 = OpTypeFunction %float %_ptr_Function_v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%39 = OpTypeFunction %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%v3float = OpTypeVector %float 3
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%112 = OpConstantComposite %v2float %float_123 %float_456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%144 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%181 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%fn = OpFunction %float None %19
%21 = OpFunctionParameter %_ptr_Function_v4float
%22 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_1
OpBranch %27
%27 = OpLabel
OpLoopMerge %31 %30 None
OpBranch %28
%28 = OpLabel
%32 = OpLoad %int %x
%34 = OpSLessThanEqual %bool %32 %int_2
OpBranchConditional %34 %29 %31
%29 = OpLabel
%35 = OpLoad %v4float %21
%36 = OpCompositeExtract %float %35 0
OpReturnValue %36
%30 = OpLabel
%37 = OpLoad %int %x
%38 = OpIAdd %int %37 %int_1
OpStore %x %38
OpBranch %27
%31 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %39
%40 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v4float Function
%110 = OpVariable %_ptr_Function_v4float Function
%118 = OpVariable %_ptr_Function_v4float Function
%123 = OpVariable %_ptr_Function_v4float Function
%130 = OpVariable %_ptr_Function_v4float Function
%135 = OpVariable %_ptr_Function_v4float Function
%185 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%45 = OpLoad %v4float %v
%46 = OpVectorShuffle %v4float %45 %45 0 1 2 3
OpStore %v %46
%47 = OpLoad %v4float %v
%48 = OpVectorShuffle %v3float %47 %47 0 1 2
%50 = OpCompositeExtract %float %48 0
%51 = OpCompositeExtract %float %48 1
%52 = OpCompositeExtract %float %48 2
%54 = OpConvertSToF %float %int_0
%55 = OpCompositeConstruct %v4float %50 %51 %52 %54
%56 = OpVectorShuffle %v4float %55 %55 3 2 1 0
OpStore %v %56
%57 = OpLoad %v4float %v
%58 = OpVectorShuffle %v4float %57 %57 0 1 2 3
%59 = OpVectorShuffle %v2float %58 %58 0 3
%61 = OpCompositeExtract %float %59 0
%62 = OpCompositeExtract %float %59 1
%63 = OpConvertSToF %float %int_0
%64 = OpCompositeConstruct %v3float %61 %62 %63
%65 = OpVectorShuffle %v4float %64 %64 2 2 0 1
OpStore %v %65
%66 = OpLoad %v4float %v
%67 = OpVectorShuffle %v4float %66 %66 0 1 2 3
%68 = OpVectorShuffle %v4float %67 %67 0 0 0 3
%69 = OpVectorShuffle %v2float %68 %68 0 3
%70 = OpCompositeExtract %float %69 0
%71 = OpCompositeExtract %float %69 1
%72 = OpConvertSToF %float %int_0
%73 = OpCompositeConstruct %v3float %70 %71 %72
%74 = OpVectorShuffle %v4float %73 %73 2 2 0 1
%75 = OpVectorShuffle %v2float %74 %74 3 2
%76 = OpCompositeExtract %float %75 0
%77 = OpCompositeExtract %float %75 1
%78 = OpConvertSToF %float %int_1
%79 = OpCompositeConstruct %v3float %76 %77 %78
%80 = OpVectorShuffle %v4float %79 %79 2 2 0 1
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpVectorShuffle %v4float %81 %81 3 2 1 3
%83 = OpVectorShuffle %v2float %82 %82 1 2
%84 = OpCompositeExtract %float %83 0
%85 = OpCompositeExtract %float %83 1
%86 = OpConvertSToF %float %int_1
%87 = OpCompositeConstruct %v3float %84 %85 %86
%88 = OpVectorShuffle %v4float %87 %87 0 1 2 2
OpStore %v %88
%89 = OpLoad %v4float %v
%90 = OpVectorShuffle %v4float %89 %89 3 2 1 0
%91 = OpVectorShuffle %v4float %90 %90 3 2 1 0
OpStore %v %91
%92 = OpLoad %v4float %v
%93 = OpVectorShuffle %v4float %92 %92 0 0 0 0
%94 = OpVectorShuffle %v2float %93 %93 2 2
%95 = OpCompositeExtract %float %94 0
%96 = OpCompositeExtract %float %94 1
%98 = OpCompositeConstruct %v4float %95 %96 %float_1 %float_1
OpStore %v %98
%99 = OpLoad %v4float %v
%100 = OpVectorShuffle %v2float %99 %99 2 3
%101 = OpVectorShuffle %v4float %100 %100 1 0 1 0
OpStore %v %101
%102 = OpLoad %v4float %v
OpStore %103 %102
%104 = OpFunctionCall %float %fn %103
%107 = OpCompositeConstruct %v3float %104 %float_123 %float_456
%108 = OpVectorShuffle %v4float %107 %107 1 1 2 2
OpStore %v %108
%109 = OpLoad %v4float %v
OpStore %110 %109
%111 = OpFunctionCall %float %fn %110
%113 = OpCompositeExtract %float %112 0
%114 = OpCompositeExtract %float %112 1
%115 = OpCompositeConstruct %v3float %111 %113 %114
%116 = OpVectorShuffle %v4float %115 %115 1 1 2 2
OpStore %v %116
%117 = OpLoad %v4float %v
OpStore %118 %117
%119 = OpFunctionCall %float %fn %118
%120 = OpCompositeConstruct %v3float %119 %float_123 %float_456
%121 = OpVectorShuffle %v4float %120 %120 1 2 2 0
OpStore %v %121
%122 = OpLoad %v4float %v
OpStore %123 %122
%124 = OpFunctionCall %float %fn %123
%125 = OpCompositeExtract %float %112 0
%126 = OpCompositeExtract %float %112 1
%127 = OpCompositeConstruct %v3float %124 %125 %126
%128 = OpVectorShuffle %v4float %127 %127 1 2 2 0
OpStore %v %128
%129 = OpLoad %v4float %v
OpStore %130 %129
%131 = OpFunctionCall %float %fn %130
%132 = OpCompositeConstruct %v3float %131 %float_123 %float_456
%133 = OpVectorShuffle %v4float %132 %132 1 0 0 2
OpStore %v %133
%134 = OpLoad %v4float %v
OpStore %135 %134
%136 = OpFunctionCall %float %fn %135
%137 = OpCompositeExtract %float %112 0
%138 = OpCompositeExtract %float %112 1
%139 = OpCompositeConstruct %v3float %136 %137 %138
%140 = OpVectorShuffle %v4float %139 %139 1 0 0 2
OpStore %v %140
%145 = OpVectorShuffle %v4float %144 %144 0 0 1 2
OpStore %v %145
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%147 = OpLoad %v4float %146
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeExtract %float %148 1
%151 = OpCompositeExtract %float %148 2
%152 = OpCompositeConstruct %v4float %float_1 %149 %150 %151
%153 = OpVectorShuffle %v4float %152 %152 1 2 3 0
OpStore %v %153
%154 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%155 = OpLoad %v4float %154
%156 = OpVectorShuffle %v3float %155 %155 0 1 2
%157 = OpCompositeExtract %float %156 0
%158 = OpCompositeExtract %float %156 1
%159 = OpCompositeExtract %float %156 2
%160 = OpCompositeConstruct %v4float %float_1 %157 %158 %159
%161 = OpVectorShuffle %v4float %160 %160 1 0 2 3
OpStore %v %161
%162 = OpLoad %v4float %v
%163 = OpLoad %v4float %v
%164 = OpVectorShuffle %v4float %163 %162 4 5 6 7
OpStore %v %164
%165 = OpLoad %v4float %v
%166 = OpLoad %v4float %v
%167 = OpVectorShuffle %v4float %166 %165 7 6 5 4
OpStore %v %167
%168 = OpLoad %v4float %v
%169 = OpVectorShuffle %v2float %168 %168 1 2
%170 = OpLoad %v4float %v
%171 = OpVectorShuffle %v4float %170 %169 4 1 2 5
OpStore %v %171
%172 = OpLoad %v4float %v
%173 = OpVectorShuffle %v2float %172 %172 3 3
%174 = OpCompositeExtract %float %173 0
%175 = OpCompositeExtract %float %173 1
%176 = OpConvertSToF %float %int_1
%177 = OpCompositeConstruct %v3float %174 %175 %176
%178 = OpLoad %v4float %v
%179 = OpVectorShuffle %v4float %178 %177 6 5 4 3
OpStore %v %179
%180 = OpLoad %v4float %v
%182 = OpFOrdEqual %v4bool %180 %181
%184 = OpAll %bool %182
OpSelectionMerge %188 None
OpBranchConditional %184 %186 %187
%186 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%190 = OpLoad %v4float %189
OpStore %185 %190
OpBranch %188
%187 = OpLabel
%191 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%192 = OpLoad %v4float %191
OpStore %185 %192
OpBranch %188
%188 = OpLabel
%193 = OpLoad %v4float %185
OpReturnValue %193
OpFunctionEnd
