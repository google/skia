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
OpDecorate %46 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%float_1 = OpConstant %float 1
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%104 = OpConstantComposite %v2float %float_123 %float_456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%136 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_0 = OpConstant %int 0
%171 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%95 = OpVariable %_ptr_Function_v4float Function
%102 = OpVariable %_ptr_Function_v4float Function
%110 = OpVariable %_ptr_Function_v4float Function
%115 = OpVariable %_ptr_Function_v4float Function
%122 = OpVariable %_ptr_Function_v4float Function
%127 = OpVariable %_ptr_Function_v4float Function
%175 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%45 = OpLoad %v4float %v
OpStore %v %45
%46 = OpLoad %v4float %v
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%49 = OpCompositeExtract %float %47 0
%50 = OpCompositeExtract %float %47 1
%51 = OpCompositeExtract %float %47 2
%53 = OpCompositeConstruct %v4float %49 %50 %51 %float_0
%54 = OpVectorShuffle %v4float %53 %53 3 2 1 0
OpStore %v %54
%55 = OpLoad %v4float %v
%56 = OpVectorShuffle %v2float %55 %55 0 3
%58 = OpCompositeExtract %float %56 0
%59 = OpCompositeExtract %float %56 1
%60 = OpCompositeConstruct %v3float %58 %59 %float_0
%61 = OpVectorShuffle %v4float %60 %60 2 2 0 1
OpStore %v %61
%62 = OpLoad %v4float %v
%63 = OpVectorShuffle %v4float %62 %62 0 0 0 3
%64 = OpVectorShuffle %v2float %63 %63 0 3
%65 = OpCompositeExtract %float %64 0
%66 = OpCompositeExtract %float %64 1
%67 = OpCompositeConstruct %v3float %65 %66 %float_0
%68 = OpVectorShuffle %v4float %67 %67 2 2 0 1
%69 = OpVectorShuffle %v2float %68 %68 3 2
%70 = OpCompositeExtract %float %69 0
%71 = OpCompositeExtract %float %69 1
%73 = OpCompositeConstruct %v3float %70 %71 %float_1
%74 = OpVectorShuffle %v4float %73 %73 2 2 0 1
OpStore %v %74
%75 = OpLoad %v4float %v
%76 = OpVectorShuffle %v4float %75 %75 3 2 1 3
%77 = OpVectorShuffle %v2float %76 %76 1 2
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpCompositeConstruct %v3float %78 %79 %float_1
%81 = OpVectorShuffle %v4float %80 %80 0 1 2 2
OpStore %v %81
%82 = OpLoad %v4float %v
%83 = OpVectorShuffle %v4float %82 %82 3 2 1 0
%84 = OpVectorShuffle %v4float %83 %83 3 2 1 0
OpStore %v %84
%85 = OpLoad %v4float %v
%86 = OpVectorShuffle %v4float %85 %85 0 0 0 0
%87 = OpVectorShuffle %v2float %86 %86 2 2
%88 = OpCompositeExtract %float %87 0
%89 = OpCompositeExtract %float %87 1
%90 = OpCompositeConstruct %v4float %88 %89 %float_1 %float_1
OpStore %v %90
%91 = OpLoad %v4float %v
%92 = OpVectorShuffle %v2float %91 %91 2 3
%93 = OpVectorShuffle %v4float %92 %92 1 0 1 0
OpStore %v %93
%94 = OpLoad %v4float %v
OpStore %95 %94
%96 = OpFunctionCall %float %fn %95
%99 = OpCompositeConstruct %v3float %96 %float_123 %float_456
%100 = OpVectorShuffle %v4float %99 %99 1 1 2 2
OpStore %v %100
%101 = OpLoad %v4float %v
OpStore %102 %101
%103 = OpFunctionCall %float %fn %102
%105 = OpCompositeExtract %float %104 0
%106 = OpCompositeExtract %float %104 1
%107 = OpCompositeConstruct %v3float %103 %105 %106
%108 = OpVectorShuffle %v4float %107 %107 1 1 2 2
OpStore %v %108
%109 = OpLoad %v4float %v
OpStore %110 %109
%111 = OpFunctionCall %float %fn %110
%112 = OpCompositeConstruct %v3float %111 %float_123 %float_456
%113 = OpVectorShuffle %v4float %112 %112 1 2 2 0
OpStore %v %113
%114 = OpLoad %v4float %v
OpStore %115 %114
%116 = OpFunctionCall %float %fn %115
%117 = OpCompositeExtract %float %104 0
%118 = OpCompositeExtract %float %104 1
%119 = OpCompositeConstruct %v3float %116 %117 %118
%120 = OpVectorShuffle %v4float %119 %119 1 2 2 0
OpStore %v %120
%121 = OpLoad %v4float %v
OpStore %122 %121
%123 = OpFunctionCall %float %fn %122
%124 = OpCompositeConstruct %v3float %123 %float_123 %float_456
%125 = OpVectorShuffle %v4float %124 %124 1 0 0 2
OpStore %v %125
%126 = OpLoad %v4float %v
OpStore %127 %126
%128 = OpFunctionCall %float %fn %127
%129 = OpCompositeExtract %float %104 0
%130 = OpCompositeExtract %float %104 1
%131 = OpCompositeConstruct %v3float %128 %129 %130
%132 = OpVectorShuffle %v4float %131 %131 1 0 0 2
OpStore %v %132
%137 = OpVectorShuffle %v4float %136 %136 0 0 1 2
OpStore %v %137
%138 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%140 = OpLoad %v4float %138
%141 = OpVectorShuffle %v3float %140 %140 0 1 2
%142 = OpCompositeExtract %float %141 0
%143 = OpCompositeExtract %float %141 1
%144 = OpCompositeExtract %float %141 2
%145 = OpCompositeConstruct %v4float %float_1 %142 %143 %144
%146 = OpVectorShuffle %v4float %145 %145 1 2 3 0
OpStore %v %146
%147 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%148 = OpLoad %v4float %147
%149 = OpVectorShuffle %v3float %148 %148 0 1 2
%150 = OpCompositeExtract %float %149 0
%151 = OpCompositeExtract %float %149 1
%152 = OpCompositeExtract %float %149 2
%153 = OpCompositeConstruct %v4float %float_1 %150 %151 %152
%154 = OpVectorShuffle %v4float %153 %153 1 0 2 3
OpStore %v %154
%155 = OpLoad %v4float %v
OpStore %v %155
%156 = OpLoad %v4float %v
%157 = OpLoad %v4float %v
%158 = OpVectorShuffle %v4float %157 %156 7 6 5 4
OpStore %v %158
%159 = OpLoad %v4float %v
%160 = OpVectorShuffle %v2float %159 %159 1 2
%161 = OpLoad %v4float %v
%162 = OpVectorShuffle %v4float %161 %160 4 1 2 5
OpStore %v %162
%163 = OpLoad %v4float %v
%164 = OpVectorShuffle %v2float %163 %163 3 3
%165 = OpCompositeExtract %float %164 0
%166 = OpCompositeExtract %float %164 1
%167 = OpCompositeConstruct %v3float %165 %166 %float_1
%168 = OpLoad %v4float %v
%169 = OpVectorShuffle %v4float %168 %167 6 5 4 3
OpStore %v %169
%170 = OpLoad %v4float %v
%172 = OpFOrdEqual %v4bool %170 %171
%174 = OpAll %bool %172
OpSelectionMerge %178 None
OpBranchConditional %174 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%180 = OpLoad %v4float %179
OpStore %175 %180
OpBranch %178
%177 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%182 = OpLoad %v4float %181
OpStore %175 %182
OpBranch %178
%178 = OpLabel
%183 = OpLoad %v4float %175
OpReturnValue %183
OpFunctionEnd
