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
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
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
%163 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
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
%167 = OpVariable %_ptr_Function_v4float Function
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
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v4float %v
%62 = OpCompositeExtract %float %61 2
%63 = OpCompositeConstruct %v3float %60 %float_0 %62
%64 = OpCompositeExtract %float %63 0
%65 = OpCompositeExtract %float %63 1
%66 = OpCompositeExtract %float %63 2
%67 = OpCompositeConstruct %v4float %64 %65 %66 %float_1
OpStore %v %67
%68 = OpLoad %v4float %v
%69 = OpCompositeExtract %float %68 0
%70 = OpCompositeConstruct %v4float %69 %float_1 %float_0 %float_1
OpStore %v %70
%71 = OpLoad %v4float %v
%72 = OpVectorShuffle %v2float %71 %71 1 2
%73 = OpCompositeExtract %float %72 0
%74 = OpCompositeExtract %float %72 1
%75 = OpCompositeConstruct %v3float %float_1 %73 %74
%76 = OpCompositeExtract %float %75 0
%77 = OpCompositeExtract %float %75 1
%78 = OpCompositeExtract %float %75 2
%79 = OpCompositeConstruct %v4float %76 %77 %78 %float_1
OpStore %v %79
%80 = OpLoad %v4float %v
%81 = OpCompositeExtract %float %80 1
%82 = OpCompositeConstruct %v3float %float_0 %81 %float_1
%83 = OpCompositeExtract %float %82 0
%84 = OpCompositeExtract %float %82 1
%85 = OpCompositeExtract %float %82 2
%86 = OpCompositeConstruct %v4float %83 %84 %85 %float_1
OpStore %v %86
%87 = OpLoad %v4float %v
%88 = OpCompositeExtract %float %87 2
%89 = OpCompositeConstruct %v3float %float_1 %float_1 %88
%90 = OpCompositeExtract %float %89 0
%91 = OpCompositeExtract %float %89 1
%92 = OpCompositeExtract %float %89 2
%93 = OpCompositeConstruct %v4float %90 %91 %92 %float_1
OpStore %v %93
%94 = OpLoad %v4float %v
%95 = OpVectorShuffle %v3float %94 %94 0 1 2
%96 = OpCompositeExtract %float %95 0
%97 = OpCompositeExtract %float %95 1
%98 = OpCompositeExtract %float %95 2
%99 = OpCompositeConstruct %v4float %96 %97 %98 %float_1
OpStore %v %99
%100 = OpLoad %v4float %v
%101 = OpVectorShuffle %v2float %100 %100 0 1
%102 = OpCompositeExtract %float %101 0
%103 = OpCompositeExtract %float %101 1
%104 = OpLoad %v4float %v
%105 = OpCompositeExtract %float %104 3
%106 = OpCompositeConstruct %v4float %102 %103 %float_0 %105
OpStore %v %106
%107 = OpLoad %v4float %v
%108 = OpVectorShuffle %v2float %107 %107 0 1
%109 = OpCompositeExtract %float %108 0
%110 = OpCompositeExtract %float %108 1
%111 = OpCompositeConstruct %v4float %109 %110 %float_1 %float_0
OpStore %v %111
%112 = OpLoad %v4float %v
%113 = OpCompositeExtract %float %112 0
%114 = OpLoad %v4float %v
%115 = OpVectorShuffle %v2float %114 %114 2 3
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %115 1
%118 = OpCompositeConstruct %v4float %113 %float_1 %116 %117
OpStore %v %118
%119 = OpLoad %v4float %v
%120 = OpCompositeExtract %float %119 0
%121 = OpLoad %v4float %v
%122 = OpCompositeExtract %float %121 2
%123 = OpCompositeConstruct %v4float %120 %float_0 %122 %float_1
OpStore %v %123
%124 = OpLoad %v4float %v
%125 = OpCompositeExtract %float %124 0
%126 = OpLoad %v4float %v
%127 = OpCompositeExtract %float %126 3
%128 = OpCompositeConstruct %v4float %125 %float_1 %float_1 %127
OpStore %v %128
%129 = OpLoad %v4float %v
%130 = OpCompositeExtract %float %129 0
%131 = OpCompositeConstruct %v4float %130 %float_1 %float_0 %float_1
OpStore %v %131
%132 = OpLoad %v4float %v
%133 = OpVectorShuffle %v3float %132 %132 1 2 3
%134 = OpCompositeExtract %float %133 0
%135 = OpCompositeExtract %float %133 1
%136 = OpCompositeExtract %float %133 2
%137 = OpCompositeConstruct %v4float %float_1 %134 %135 %136
OpStore %v %137
%138 = OpLoad %v4float %v
%139 = OpVectorShuffle %v2float %138 %138 1 2
%140 = OpCompositeExtract %float %139 0
%141 = OpCompositeExtract %float %139 1
%142 = OpCompositeConstruct %v4float %float_0 %140 %141 %float_1
OpStore %v %142
%143 = OpLoad %v4float %v
%144 = OpCompositeExtract %float %143 1
%145 = OpLoad %v4float %v
%146 = OpCompositeExtract %float %145 3
%147 = OpCompositeConstruct %v4float %float_0 %144 %float_1 %146
OpStore %v %147
%148 = OpLoad %v4float %v
%149 = OpCompositeExtract %float %148 1
%150 = OpCompositeConstruct %v4float %float_1 %149 %float_1 %float_1
OpStore %v %150
%151 = OpLoad %v4float %v
%152 = OpVectorShuffle %v2float %151 %151 2 3
%153 = OpCompositeExtract %float %152 0
%154 = OpCompositeExtract %float %152 1
%155 = OpCompositeConstruct %v4float %float_0 %float_0 %153 %154
OpStore %v %155
%156 = OpLoad %v4float %v
%157 = OpCompositeExtract %float %156 2
%158 = OpCompositeConstruct %v4float %float_0 %float_0 %157 %float_1
OpStore %v %158
%159 = OpLoad %v4float %v
%160 = OpCompositeExtract %float %159 3
%161 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %160
OpStore %v %161
%162 = OpLoad %v4float %v
%164 = OpFOrdEqual %v4bool %162 %163
%166 = OpAll %bool %164
OpSelectionMerge %170 None
OpBranchConditional %166 %168 %169
%168 = OpLabel
%171 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%173 = OpLoad %v4float %171
OpStore %167 %173
OpBranch %170
%169 = OpLabel
%174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%176 = OpLoad %v4float %174
OpStore %167 %176
OpBranch %170
%170 = OpLabel
%177 = OpLoad %v4float %167
OpReturnValue %177
OpFunctionEnd
