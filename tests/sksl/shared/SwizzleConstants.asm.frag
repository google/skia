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
OpDecorate %43 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
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
%189 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
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
%193 = OpVariable %_ptr_Function_v4float Function
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
%39 = OpCompositeConstruct %v2float %38 %float_1
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeConstruct %v4float %40 %41 %float_1 %float_1
OpStore %v %42
%43 = OpLoad %v4float %v
%44 = OpCompositeExtract %float %43 1
%46 = OpCompositeConstruct %v2float %44 %float_0
%47 = OpVectorShuffle %v2float %46 %46 1 0
%48 = OpCompositeExtract %float %47 0
%49 = OpCompositeExtract %float %47 1
%50 = OpCompositeConstruct %v4float %48 %49 %float_1 %float_1
OpStore %v %50
%51 = OpLoad %v4float %v
%52 = OpVectorShuffle %v3float %51 %51 0 1 2
%54 = OpCompositeExtract %float %52 0
%55 = OpCompositeExtract %float %52 1
%56 = OpCompositeExtract %float %52 2
%57 = OpCompositeConstruct %v4float %54 %55 %56 %float_1
OpStore %v %57
%58 = OpLoad %v4float %v
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpCompositeExtract %float %59 0
%61 = OpCompositeExtract %float %59 1
%62 = OpCompositeConstruct %v3float %60 %61 %float_1
%63 = OpCompositeExtract %float %62 0
%64 = OpCompositeExtract %float %62 1
%65 = OpCompositeExtract %float %62 2
%66 = OpCompositeConstruct %v4float %63 %64 %65 %float_1
OpStore %v %66
%67 = OpLoad %v4float %v
%68 = OpVectorShuffle %v2float %67 %67 0 2
%69 = OpCompositeExtract %float %68 0
%70 = OpCompositeExtract %float %68 1
%71 = OpCompositeConstruct %v3float %69 %70 %float_0
%72 = OpVectorShuffle %v3float %71 %71 0 2 1
%73 = OpCompositeExtract %float %72 0
%74 = OpCompositeExtract %float %72 1
%75 = OpCompositeExtract %float %72 2
%76 = OpCompositeConstruct %v4float %73 %74 %75 %float_1
OpStore %v %76
%77 = OpLoad %v4float %v
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeConstruct %v3float %78 %float_1 %float_0
%80 = OpCompositeExtract %float %79 0
%81 = OpCompositeExtract %float %79 1
%82 = OpCompositeExtract %float %79 2
%83 = OpCompositeConstruct %v4float %80 %81 %82 %float_1
OpStore %v %83
%84 = OpLoad %v4float %v
%85 = OpVectorShuffle %v2float %84 %84 1 2
%86 = OpCompositeExtract %float %85 0
%87 = OpCompositeExtract %float %85 1
%88 = OpCompositeConstruct %v3float %86 %87 %float_1
%89 = OpVectorShuffle %v3float %88 %88 2 0 1
%90 = OpCompositeExtract %float %89 0
%91 = OpCompositeExtract %float %89 1
%92 = OpCompositeExtract %float %89 2
%93 = OpCompositeConstruct %v4float %90 %91 %92 %float_1
OpStore %v %93
%94 = OpLoad %v4float %v
%95 = OpCompositeExtract %float %94 1
%96 = OpCompositeConstruct %v3float %95 %float_0 %float_1
%97 = OpVectorShuffle %v3float %96 %96 1 0 2
%98 = OpCompositeExtract %float %97 0
%99 = OpCompositeExtract %float %97 1
%100 = OpCompositeExtract %float %97 2
%101 = OpCompositeConstruct %v4float %98 %99 %100 %float_1
OpStore %v %101
%102 = OpLoad %v4float %v
%103 = OpCompositeExtract %float %102 2
%104 = OpCompositeConstruct %v2float %103 %float_1
%105 = OpVectorShuffle %v3float %104 %104 1 1 0
%106 = OpCompositeExtract %float %105 0
%107 = OpCompositeExtract %float %105 1
%108 = OpCompositeExtract %float %105 2
%109 = OpCompositeConstruct %v4float %106 %107 %108 %float_1
OpStore %v %109
%110 = OpLoad %v4float %v
%111 = OpVectorShuffle %v3float %110 %110 0 1 2
%112 = OpCompositeExtract %float %111 0
%113 = OpCompositeExtract %float %111 1
%114 = OpCompositeExtract %float %111 2
%115 = OpCompositeConstruct %v4float %112 %113 %114 %float_1
OpStore %v %115
%116 = OpLoad %v4float %v
%117 = OpVectorShuffle %v3float %116 %116 0 1 3
%118 = OpCompositeExtract %float %117 0
%119 = OpCompositeExtract %float %117 1
%120 = OpCompositeExtract %float %117 2
%121 = OpCompositeConstruct %v4float %118 %119 %120 %float_0
%122 = OpVectorShuffle %v4float %121 %121 0 1 3 2
OpStore %v %122
%123 = OpLoad %v4float %v
%124 = OpVectorShuffle %v2float %123 %123 0 1
%125 = OpCompositeExtract %float %124 0
%126 = OpCompositeExtract %float %124 1
%127 = OpCompositeConstruct %v4float %125 %126 %float_1 %float_0
OpStore %v %127
%128 = OpLoad %v4float %v
%129 = OpVectorShuffle %v3float %128 %128 0 2 3
%130 = OpCompositeExtract %float %129 0
%131 = OpCompositeExtract %float %129 1
%132 = OpCompositeExtract %float %129 2
%133 = OpCompositeConstruct %v4float %130 %131 %132 %float_1
%134 = OpVectorShuffle %v4float %133 %133 0 3 1 2
OpStore %v %134
%135 = OpLoad %v4float %v
%136 = OpVectorShuffle %v2float %135 %135 0 2
%137 = OpCompositeExtract %float %136 0
%138 = OpCompositeExtract %float %136 1
%139 = OpCompositeConstruct %v4float %137 %138 %float_0 %float_1
%140 = OpVectorShuffle %v4float %139 %139 0 2 1 3
OpStore %v %140
%141 = OpLoad %v4float %v
%142 = OpVectorShuffle %v2float %141 %141 0 3
%143 = OpCompositeExtract %float %142 0
%144 = OpCompositeExtract %float %142 1
%145 = OpCompositeConstruct %v3float %143 %144 %float_1
%146 = OpVectorShuffle %v4float %145 %145 0 2 2 1
OpStore %v %146
%147 = OpLoad %v4float %v
%148 = OpCompositeExtract %float %147 0
%149 = OpCompositeConstruct %v3float %148 %float_1 %float_0
%150 = OpVectorShuffle %v4float %149 %149 0 1 2 1
OpStore %v %150
%151 = OpLoad %v4float %v
%152 = OpVectorShuffle %v3float %151 %151 1 2 3
%153 = OpCompositeExtract %float %152 0
%154 = OpCompositeExtract %float %152 1
%155 = OpCompositeExtract %float %152 2
%156 = OpCompositeConstruct %v4float %153 %154 %155 %float_1
%157 = OpVectorShuffle %v4float %156 %156 3 0 1 2
OpStore %v %157
%158 = OpLoad %v4float %v
%159 = OpVectorShuffle %v2float %158 %158 1 2
%160 = OpCompositeExtract %float %159 0
%161 = OpCompositeExtract %float %159 1
%162 = OpCompositeConstruct %v4float %160 %161 %float_0 %float_1
%163 = OpVectorShuffle %v4float %162 %162 2 0 1 3
OpStore %v %163
%164 = OpLoad %v4float %v
%165 = OpVectorShuffle %v2float %164 %164 1 3
%166 = OpCompositeExtract %float %165 0
%167 = OpCompositeExtract %float %165 1
%168 = OpCompositeConstruct %v4float %166 %167 %float_0 %float_1
%169 = OpVectorShuffle %v4float %168 %168 2 0 3 1
OpStore %v %169
%170 = OpLoad %v4float %v
%171 = OpCompositeExtract %float %170 1
%172 = OpCompositeConstruct %v2float %171 %float_1
%173 = OpVectorShuffle %v4float %172 %172 1 0 1 1
OpStore %v %173
%174 = OpLoad %v4float %v
%175 = OpVectorShuffle %v2float %174 %174 2 3
%176 = OpCompositeExtract %float %175 0
%177 = OpCompositeExtract %float %175 1
%178 = OpCompositeConstruct %v3float %176 %177 %float_0
%179 = OpVectorShuffle %v4float %178 %178 2 2 0 1
OpStore %v %179
%180 = OpLoad %v4float %v
%181 = OpCompositeExtract %float %180 2
%182 = OpCompositeConstruct %v3float %181 %float_0 %float_1
%183 = OpVectorShuffle %v4float %182 %182 1 1 0 2
OpStore %v %183
%184 = OpLoad %v4float %v
%185 = OpCompositeExtract %float %184 3
%186 = OpCompositeConstruct %v3float %185 %float_0 %float_1
%187 = OpVectorShuffle %v4float %186 %186 1 2 2 0
OpStore %v %187
%188 = OpLoad %v4float %v
%190 = OpFOrdEqual %v4bool %188 %189
%192 = OpAll %bool %190
OpSelectionMerge %196 None
OpBranchConditional %192 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%199 = OpLoad %v4float %197
OpStore %193 %199
OpBranch %196
%195 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%202 = OpLoad %v4float %200
OpStore %193 %202
OpBranch %196
%196 = OpLabel
%203 = OpLoad %v4float %193
OpReturnValue %203
OpFunctionEnd
