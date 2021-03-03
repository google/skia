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
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
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
%103 = OpConstantComposite %v2float %float_123 %float_456
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%135 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int_0 = OpConstant %int 0
%169 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
%94 = OpVariable %_ptr_Function_v4float Function
%101 = OpVariable %_ptr_Function_v4float Function
%109 = OpVariable %_ptr_Function_v4float Function
%114 = OpVariable %_ptr_Function_v4float Function
%121 = OpVariable %_ptr_Function_v4float Function
%126 = OpVariable %_ptr_Function_v4float Function
%173 = OpVariable %_ptr_Function_v4float Function
%42 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%44 = OpLoad %v4float %42
OpStore %v %44
%45 = OpLoad %v4float %v
%46 = OpVectorShuffle %v3float %45 %45 0 1 2
%48 = OpCompositeExtract %float %46 0
%49 = OpCompositeExtract %float %46 1
%50 = OpCompositeExtract %float %46 2
%52 = OpCompositeConstruct %v4float %48 %49 %50 %float_0
%53 = OpVectorShuffle %v4float %52 %52 3 2 1 0
OpStore %v %53
%54 = OpLoad %v4float %v
%55 = OpVectorShuffle %v2float %54 %54 0 3
%57 = OpCompositeExtract %float %55 0
%58 = OpCompositeExtract %float %55 1
%59 = OpCompositeConstruct %v3float %57 %58 %float_0
%60 = OpVectorShuffle %v4float %59 %59 2 2 0 1
OpStore %v %60
%61 = OpLoad %v4float %v
%62 = OpVectorShuffle %v4float %61 %61 0 0 0 3
%63 = OpVectorShuffle %v2float %62 %62 0 3
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeConstruct %v3float %64 %65 %float_0
%67 = OpVectorShuffle %v4float %66 %66 2 2 0 1
%68 = OpVectorShuffle %v2float %67 %67 3 2
%69 = OpCompositeExtract %float %68 0
%70 = OpCompositeExtract %float %68 1
%72 = OpCompositeConstruct %v3float %69 %70 %float_1
%73 = OpVectorShuffle %v4float %72 %72 2 2 0 1
OpStore %v %73
%74 = OpLoad %v4float %v
%75 = OpVectorShuffle %v4float %74 %74 3 2 1 3
%76 = OpVectorShuffle %v2float %75 %75 1 2
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeExtract %float %76 1
%79 = OpCompositeConstruct %v3float %77 %78 %float_1
%80 = OpVectorShuffle %v4float %79 %79 0 1 2 2
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpVectorShuffle %v4float %81 %81 3 2 1 0
%83 = OpVectorShuffle %v4float %82 %82 3 2 1 0
OpStore %v %83
%84 = OpLoad %v4float %v
%85 = OpVectorShuffle %v4float %84 %84 0 0 0 0
%86 = OpVectorShuffle %v2float %85 %85 2 2
%87 = OpCompositeExtract %float %86 0
%88 = OpCompositeExtract %float %86 1
%89 = OpCompositeConstruct %v4float %87 %88 %float_1 %float_1
OpStore %v %89
%90 = OpLoad %v4float %v
%91 = OpVectorShuffle %v2float %90 %90 2 3
%92 = OpVectorShuffle %v4float %91 %91 1 0 1 0
OpStore %v %92
%93 = OpLoad %v4float %v
OpStore %94 %93
%95 = OpFunctionCall %float %fn %94
%98 = OpCompositeConstruct %v3float %95 %float_123 %float_456
%99 = OpVectorShuffle %v4float %98 %98 1 1 2 2
OpStore %v %99
%100 = OpLoad %v4float %v
OpStore %101 %100
%102 = OpFunctionCall %float %fn %101
%104 = OpCompositeExtract %float %103 0
%105 = OpCompositeExtract %float %103 1
%106 = OpCompositeConstruct %v3float %102 %104 %105
%107 = OpVectorShuffle %v4float %106 %106 1 1 2 2
OpStore %v %107
%108 = OpLoad %v4float %v
OpStore %109 %108
%110 = OpFunctionCall %float %fn %109
%111 = OpCompositeConstruct %v3float %110 %float_123 %float_456
%112 = OpVectorShuffle %v4float %111 %111 1 2 2 0
OpStore %v %112
%113 = OpLoad %v4float %v
OpStore %114 %113
%115 = OpFunctionCall %float %fn %114
%116 = OpCompositeExtract %float %103 0
%117 = OpCompositeExtract %float %103 1
%118 = OpCompositeConstruct %v3float %115 %116 %117
%119 = OpVectorShuffle %v4float %118 %118 1 2 2 0
OpStore %v %119
%120 = OpLoad %v4float %v
OpStore %121 %120
%122 = OpFunctionCall %float %fn %121
%123 = OpCompositeConstruct %v3float %122 %float_123 %float_456
%124 = OpVectorShuffle %v4float %123 %123 1 0 0 2
OpStore %v %124
%125 = OpLoad %v4float %v
OpStore %126 %125
%127 = OpFunctionCall %float %fn %126
%128 = OpCompositeExtract %float %103 0
%129 = OpCompositeExtract %float %103 1
%130 = OpCompositeConstruct %v3float %127 %128 %129
%131 = OpVectorShuffle %v4float %130 %130 1 0 0 2
OpStore %v %131
%136 = OpVectorShuffle %v4float %135 %135 0 0 1 2
OpStore %v %136
%137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%139 = OpLoad %v4float %137
%140 = OpVectorShuffle %v3float %139 %139 0 1 2
%141 = OpCompositeExtract %float %140 0
%142 = OpCompositeExtract %float %140 1
%143 = OpCompositeExtract %float %140 2
%144 = OpCompositeConstruct %v4float %float_1 %141 %142 %143
%145 = OpVectorShuffle %v4float %144 %144 1 2 3 0
OpStore %v %145
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%147 = OpLoad %v4float %146
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeExtract %float %148 1
%151 = OpCompositeExtract %float %148 2
%152 = OpCompositeConstruct %v4float %float_1 %149 %150 %151
%153 = OpVectorShuffle %v4float %152 %152 1 0 2 3
OpStore %v %153
%154 = OpLoad %v4float %v
%155 = OpLoad %v4float %v
%156 = OpVectorShuffle %v4float %155 %154 7 6 5 4
OpStore %v %156
%157 = OpLoad %v4float %v
%158 = OpVectorShuffle %v2float %157 %157 1 2
%159 = OpLoad %v4float %v
%160 = OpVectorShuffle %v4float %159 %158 4 1 2 5
OpStore %v %160
%161 = OpLoad %v4float %v
%162 = OpVectorShuffle %v2float %161 %161 3 3
%163 = OpCompositeExtract %float %162 0
%164 = OpCompositeExtract %float %162 1
%165 = OpCompositeConstruct %v3float %163 %164 %float_1
%166 = OpLoad %v4float %v
%167 = OpVectorShuffle %v4float %166 %165 6 5 4 3
OpStore %v %167
%168 = OpLoad %v4float %v
%170 = OpFOrdEqual %v4bool %168 %169
%172 = OpAll %bool %170
OpSelectionMerge %176 None
OpBranchConditional %172 %174 %175
%174 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%178 = OpLoad %v4float %177
OpStore %173 %178
OpBranch %176
%175 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%180 = OpLoad %v4float %179
OpStore %173 %180
OpBranch %176
%176 = OpLabel
%181 = OpLoad %v4float %173
OpReturnValue %181
OpFunctionEnd
