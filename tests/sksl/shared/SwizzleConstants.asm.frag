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
OpDecorate %111 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
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
%190 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
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
%194 = OpVariable %_ptr_Function_v4float Function
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
OpStore %v %110
%111 = OpLoad %v4float %v
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%113 = OpCompositeExtract %float %112 0
%114 = OpCompositeExtract %float %112 1
%115 = OpCompositeExtract %float %112 2
%116 = OpCompositeConstruct %v4float %113 %114 %115 %float_1
OpStore %v %116
%117 = OpLoad %v4float %v
%118 = OpVectorShuffle %v3float %117 %117 0 1 3
%119 = OpCompositeExtract %float %118 0
%120 = OpCompositeExtract %float %118 1
%121 = OpCompositeExtract %float %118 2
%122 = OpCompositeConstruct %v4float %119 %120 %121 %float_0
%123 = OpVectorShuffle %v4float %122 %122 0 1 3 2
OpStore %v %123
%124 = OpLoad %v4float %v
%125 = OpVectorShuffle %v2float %124 %124 0 1
%126 = OpCompositeExtract %float %125 0
%127 = OpCompositeExtract %float %125 1
%128 = OpCompositeConstruct %v4float %126 %127 %float_1 %float_0
OpStore %v %128
%129 = OpLoad %v4float %v
%130 = OpVectorShuffle %v3float %129 %129 0 2 3
%131 = OpCompositeExtract %float %130 0
%132 = OpCompositeExtract %float %130 1
%133 = OpCompositeExtract %float %130 2
%134 = OpCompositeConstruct %v4float %131 %132 %133 %float_1
%135 = OpVectorShuffle %v4float %134 %134 0 3 1 2
OpStore %v %135
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v2float %136 %136 0 2
%138 = OpCompositeExtract %float %137 0
%139 = OpCompositeExtract %float %137 1
%140 = OpCompositeConstruct %v4float %138 %139 %float_0 %float_1
%141 = OpVectorShuffle %v4float %140 %140 0 2 1 3
OpStore %v %141
%142 = OpLoad %v4float %v
%143 = OpVectorShuffle %v2float %142 %142 0 3
%144 = OpCompositeExtract %float %143 0
%145 = OpCompositeExtract %float %143 1
%146 = OpCompositeConstruct %v3float %144 %145 %float_1
%147 = OpVectorShuffle %v4float %146 %146 0 2 2 1
OpStore %v %147
%148 = OpLoad %v4float %v
%149 = OpCompositeExtract %float %148 0
%150 = OpCompositeConstruct %v3float %149 %float_1 %float_0
%151 = OpVectorShuffle %v4float %150 %150 0 1 2 1
OpStore %v %151
%152 = OpLoad %v4float %v
%153 = OpVectorShuffle %v3float %152 %152 1 2 3
%154 = OpCompositeExtract %float %153 0
%155 = OpCompositeExtract %float %153 1
%156 = OpCompositeExtract %float %153 2
%157 = OpCompositeConstruct %v4float %154 %155 %156 %float_1
%158 = OpVectorShuffle %v4float %157 %157 3 0 1 2
OpStore %v %158
%159 = OpLoad %v4float %v
%160 = OpVectorShuffle %v2float %159 %159 1 2
%161 = OpCompositeExtract %float %160 0
%162 = OpCompositeExtract %float %160 1
%163 = OpCompositeConstruct %v4float %161 %162 %float_0 %float_1
%164 = OpVectorShuffle %v4float %163 %163 2 0 1 3
OpStore %v %164
%165 = OpLoad %v4float %v
%166 = OpVectorShuffle %v2float %165 %165 1 3
%167 = OpCompositeExtract %float %166 0
%168 = OpCompositeExtract %float %166 1
%169 = OpCompositeConstruct %v4float %167 %168 %float_0 %float_1
%170 = OpVectorShuffle %v4float %169 %169 2 0 3 1
OpStore %v %170
%171 = OpLoad %v4float %v
%172 = OpCompositeExtract %float %171 1
%173 = OpCompositeConstruct %v2float %172 %float_1
%174 = OpVectorShuffle %v4float %173 %173 1 0 1 1
OpStore %v %174
%175 = OpLoad %v4float %v
%176 = OpVectorShuffle %v2float %175 %175 2 3
%177 = OpCompositeExtract %float %176 0
%178 = OpCompositeExtract %float %176 1
%179 = OpCompositeConstruct %v3float %177 %178 %float_0
%180 = OpVectorShuffle %v4float %179 %179 2 2 0 1
OpStore %v %180
%181 = OpLoad %v4float %v
%182 = OpCompositeExtract %float %181 2
%183 = OpCompositeConstruct %v3float %182 %float_0 %float_1
%184 = OpVectorShuffle %v4float %183 %183 1 1 0 2
OpStore %v %184
%185 = OpLoad %v4float %v
%186 = OpCompositeExtract %float %185 3
%187 = OpCompositeConstruct %v3float %186 %float_0 %float_1
%188 = OpVectorShuffle %v4float %187 %187 1 2 2 0
OpStore %v %188
%189 = OpLoad %v4float %v
%191 = OpFOrdEqual %v4bool %189 %190
%193 = OpAll %bool %191
OpSelectionMerge %197 None
OpBranchConditional %193 %195 %196
%195 = OpLabel
%198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%200 = OpLoad %v4float %198
OpStore %194 %200
OpBranch %197
%196 = OpLabel
%201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%203 = OpLoad %v4float %201
OpStore %194 %203
OpBranch %197
%197 = OpLabel
%204 = OpLoad %v4float %194
OpReturnValue %204
OpFunctionEnd
