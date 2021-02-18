OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
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
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%179 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%183 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %v %26
%27 = OpLoad %v4float %v
%28 = OpCompositeExtract %float %27 0
%30 = OpCompositeConstruct %v4float %28 %float_1 %float_1 %float_1
OpStore %v %30
%31 = OpLoad %v4float %v
%32 = OpVectorShuffle %v2float %31 %31 0 1
%34 = OpCompositeExtract %float %32 0
%35 = OpCompositeExtract %float %32 1
%36 = OpCompositeConstruct %v4float %34 %35 %float_1 %float_1
OpStore %v %36
%37 = OpLoad %v4float %v
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeConstruct %v4float %38 %float_1 %float_1 %float_1
OpStore %v %39
%41 = OpLoad %v4float %v
%42 = OpCompositeExtract %float %41 1
%43 = OpCompositeConstruct %v2float %float_0 %42
%44 = OpCompositeExtract %float %43 0
%45 = OpCompositeExtract %float %43 1
%46 = OpCompositeConstruct %v4float %44 %45 %float_1 %float_1
OpStore %v %46
%47 = OpLoad %v4float %v
%48 = OpVectorShuffle %v3float %47 %47 0 1 2
%50 = OpCompositeExtract %float %48 0
%51 = OpCompositeExtract %float %48 1
%52 = OpCompositeExtract %float %48 2
%53 = OpCompositeConstruct %v4float %50 %51 %52 %float_1
OpStore %v %53
%54 = OpLoad %v4float %v
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpCompositeExtract %float %55 0
%57 = OpCompositeExtract %float %55 1
%58 = OpCompositeConstruct %v4float %56 %57 %float_1 %float_1
OpStore %v %58
%59 = OpLoad %v4float %v
%60 = OpVectorShuffle %v2float %59 %59 0 2
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v4float %v
%63 = OpVectorShuffle %v2float %62 %62 0 2
%64 = OpCompositeExtract %float %63 1
%65 = OpCompositeConstruct %v3float %61 %float_0 %64
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeExtract %float %65 2
%69 = OpCompositeConstruct %v4float %66 %67 %68 %float_1
OpStore %v %69
%70 = OpLoad %v4float %v
%71 = OpCompositeExtract %float %70 0
%72 = OpCompositeConstruct %v4float %71 %float_1 %float_0 %float_1
OpStore %v %72
%73 = OpLoad %v4float %v
%74 = OpVectorShuffle %v2float %73 %73 1 2
%75 = OpVectorShuffle %v2float %74 %74 0 1
%76 = OpCompositeExtract %float %75 0
%77 = OpCompositeExtract %float %75 1
%78 = OpCompositeConstruct %v3float %float_1 %76 %77
%79 = OpCompositeExtract %float %78 0
%80 = OpCompositeExtract %float %78 1
%81 = OpCompositeExtract %float %78 2
%82 = OpCompositeConstruct %v4float %79 %80 %81 %float_1
OpStore %v %82
%83 = OpLoad %v4float %v
%84 = OpCompositeExtract %float %83 1
%85 = OpCompositeConstruct %v3float %float_0 %84 %float_1
%86 = OpCompositeExtract %float %85 0
%87 = OpCompositeExtract %float %85 1
%88 = OpCompositeExtract %float %85 2
%89 = OpCompositeConstruct %v4float %86 %87 %88 %float_1
OpStore %v %89
%90 = OpLoad %v4float %v
%91 = OpCompositeExtract %float %90 2
%92 = OpCompositeConstruct %v3float %float_1 %float_1 %91
%93 = OpCompositeExtract %float %92 0
%94 = OpCompositeExtract %float %92 1
%95 = OpCompositeExtract %float %92 2
%96 = OpCompositeConstruct %v4float %93 %94 %95 %float_1
OpStore %v %96
%97 = OpLoad %v4float %v
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%99 = OpCompositeExtract %float %98 0
%100 = OpCompositeExtract %float %98 1
%101 = OpCompositeExtract %float %98 2
%102 = OpCompositeConstruct %v4float %99 %100 %101 %float_1
OpStore %v %102
%103 = OpLoad %v4float %v
%104 = OpVectorShuffle %v3float %103 %103 0 1 3
%105 = OpVectorShuffle %v2float %104 %104 0 1
%106 = OpCompositeExtract %float %105 0
%107 = OpCompositeExtract %float %105 1
%108 = OpLoad %v4float %v
%109 = OpVectorShuffle %v3float %108 %108 0 1 3
%110 = OpCompositeExtract %float %109 2
%111 = OpCompositeConstruct %v4float %106 %107 %float_0 %110
OpStore %v %111
%112 = OpLoad %v4float %v
%113 = OpVectorShuffle %v2float %112 %112 0 1
%114 = OpCompositeExtract %float %113 0
%115 = OpCompositeExtract %float %113 1
%116 = OpCompositeConstruct %v4float %114 %115 %float_1 %float_0
OpStore %v %116
%117 = OpLoad %v4float %v
%118 = OpVectorShuffle %v3float %117 %117 0 2 3
%119 = OpCompositeExtract %float %118 0
%120 = OpLoad %v4float %v
%121 = OpVectorShuffle %v3float %120 %120 0 2 3
%122 = OpVectorShuffle %v2float %121 %121 1 2
%123 = OpCompositeExtract %float %122 0
%124 = OpCompositeExtract %float %122 1
%125 = OpCompositeConstruct %v4float %119 %float_1 %123 %124
OpStore %v %125
%126 = OpLoad %v4float %v
%127 = OpVectorShuffle %v2float %126 %126 0 2
%128 = OpCompositeExtract %float %127 0
%129 = OpLoad %v4float %v
%130 = OpVectorShuffle %v2float %129 %129 0 2
%131 = OpCompositeExtract %float %130 1
%132 = OpCompositeConstruct %v4float %128 %float_0 %131 %float_1
OpStore %v %132
%133 = OpLoad %v4float %v
%134 = OpVectorShuffle %v2float %133 %133 0 3
%135 = OpCompositeExtract %float %134 0
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v2float %136 %136 0 3
%138 = OpCompositeExtract %float %137 1
%139 = OpCompositeConstruct %v4float %135 %float_1 %float_1 %138
OpStore %v %139
%140 = OpLoad %v4float %v
%141 = OpCompositeExtract %float %140 0
%142 = OpCompositeConstruct %v4float %141 %float_1 %float_0 %float_1
OpStore %v %142
%143 = OpLoad %v4float %v
%144 = OpVectorShuffle %v3float %143 %143 1 2 3
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%146 = OpCompositeExtract %float %145 0
%147 = OpCompositeExtract %float %145 1
%148 = OpCompositeExtract %float %145 2
%149 = OpCompositeConstruct %v4float %float_1 %146 %147 %148
OpStore %v %149
%150 = OpLoad %v4float %v
%151 = OpVectorShuffle %v2float %150 %150 1 2
%152 = OpVectorShuffle %v2float %151 %151 0 1
%153 = OpCompositeExtract %float %152 0
%154 = OpCompositeExtract %float %152 1
%155 = OpCompositeConstruct %v4float %float_0 %153 %154 %float_1
OpStore %v %155
%156 = OpLoad %v4float %v
%157 = OpVectorShuffle %v2float %156 %156 1 3
%158 = OpCompositeExtract %float %157 0
%159 = OpLoad %v4float %v
%160 = OpVectorShuffle %v2float %159 %159 1 3
%161 = OpCompositeExtract %float %160 1
%162 = OpCompositeConstruct %v4float %float_0 %158 %float_1 %161
OpStore %v %162
%163 = OpLoad %v4float %v
%164 = OpCompositeExtract %float %163 1
%165 = OpCompositeConstruct %v4float %float_1 %164 %float_1 %float_1
OpStore %v %165
%166 = OpLoad %v4float %v
%167 = OpVectorShuffle %v2float %166 %166 2 3
%168 = OpVectorShuffle %v2float %167 %167 0 1
%169 = OpCompositeExtract %float %168 0
%170 = OpCompositeExtract %float %168 1
%171 = OpCompositeConstruct %v4float %float_0 %float_0 %169 %170
OpStore %v %171
%172 = OpLoad %v4float %v
%173 = OpCompositeExtract %float %172 2
%174 = OpCompositeConstruct %v4float %float_0 %float_0 %173 %float_1
OpStore %v %174
%175 = OpLoad %v4float %v
%176 = OpCompositeExtract %float %175 3
%177 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %176
OpStore %v %177
%178 = OpLoad %v4float %v
%180 = OpFOrdEqual %v4bool %178 %179
%182 = OpAll %bool %180
OpSelectionMerge %186 None
OpBranchConditional %182 %184 %185
%184 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%189 = OpLoad %v4float %187
OpStore %183 %189
OpBranch %186
%185 = OpLabel
%190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%192 = OpLoad %v4float %190
OpStore %183 %192
OpBranch %186
%186 = OpLabel
%193 = OpLoad %v4float %183
OpReturnValue %193
OpFunctionEnd
