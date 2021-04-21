OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %v RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%148 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
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
%v = OpVariable %_ptr_Function_v4float Function
%152 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %v %32
%33 = OpLoad %v4float %v
%34 = OpCompositeExtract %float %33 0
%36 = OpCompositeConstruct %v4float %34 %float_1 %float_1 %float_1
OpStore %v %36
%37 = OpLoad %v4float %v
%38 = OpVectorShuffle %v2float %37 %37 0 1
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeConstruct %v4float %39 %40 %float_1 %float_1
OpStore %v %41
%42 = OpLoad %v4float %v
%43 = OpCompositeExtract %float %42 0
%44 = OpCompositeConstruct %v4float %43 %float_1 %float_1 %float_1
OpStore %v %44
%45 = OpLoad %v4float %v
%46 = OpCompositeExtract %float %45 1
%47 = OpCompositeConstruct %v4float %float_0 %46 %float_1 %float_1
OpStore %v %47
%48 = OpLoad %v4float %v
%49 = OpVectorShuffle %v3float %48 %48 0 1 2
%51 = OpCompositeExtract %float %49 0
%52 = OpCompositeExtract %float %49 1
%53 = OpCompositeExtract %float %49 2
%54 = OpCompositeConstruct %v4float %51 %52 %53 %float_1
OpStore %v %54
%55 = OpLoad %v4float %v
%56 = OpVectorShuffle %v2float %55 %55 0 1
%57 = OpCompositeExtract %float %56 0
%58 = OpCompositeExtract %float %56 1
%59 = OpCompositeConstruct %v4float %57 %58 %float_1 %float_1
OpStore %v %59
%60 = OpLoad %v4float %v
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v4float %v
%63 = OpCompositeExtract %float %62 2
%64 = OpCompositeConstruct %v4float %61 %float_0 %63 %float_1
OpStore %v %64
%65 = OpLoad %v4float %v
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeConstruct %v4float %66 %float_1 %float_0 %float_1
OpStore %v %67
%68 = OpLoad %v4float %v
%69 = OpVectorShuffle %v2float %68 %68 1 2
%70 = OpCompositeExtract %float %69 0
%71 = OpCompositeExtract %float %69 1
%72 = OpCompositeConstruct %v4float %float_1 %70 %71 %float_1
OpStore %v %72
%73 = OpLoad %v4float %v
%74 = OpCompositeExtract %float %73 1
%75 = OpCompositeConstruct %v4float %float_0 %74 %float_1 %float_1
OpStore %v %75
%76 = OpLoad %v4float %v
%77 = OpCompositeExtract %float %76 2
%78 = OpCompositeConstruct %v4float %float_1 %float_1 %77 %float_1
OpStore %v %78
%79 = OpLoad %v4float %v
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%81 = OpCompositeExtract %float %80 0
%82 = OpCompositeExtract %float %80 1
%83 = OpCompositeExtract %float %80 2
%84 = OpCompositeConstruct %v4float %81 %82 %83 %float_1
OpStore %v %84
%85 = OpLoad %v4float %v
%86 = OpVectorShuffle %v2float %85 %85 0 1
%87 = OpCompositeExtract %float %86 0
%88 = OpCompositeExtract %float %86 1
%89 = OpLoad %v4float %v
%90 = OpCompositeExtract %float %89 3
%91 = OpCompositeConstruct %v4float %87 %88 %float_0 %90
OpStore %v %91
%92 = OpLoad %v4float %v
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpCompositeExtract %float %93 0
%95 = OpCompositeExtract %float %93 1
%96 = OpCompositeConstruct %v4float %94 %95 %float_1 %float_0
OpStore %v %96
%97 = OpLoad %v4float %v
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v4float %v
%100 = OpVectorShuffle %v2float %99 %99 2 3
%101 = OpCompositeExtract %float %100 0
%102 = OpCompositeExtract %float %100 1
%103 = OpCompositeConstruct %v4float %98 %float_1 %101 %102
OpStore %v %103
%104 = OpLoad %v4float %v
%105 = OpCompositeExtract %float %104 0
%106 = OpLoad %v4float %v
%107 = OpCompositeExtract %float %106 2
%108 = OpCompositeConstruct %v4float %105 %float_0 %107 %float_1
OpStore %v %108
%109 = OpLoad %v4float %v
%110 = OpCompositeExtract %float %109 0
%111 = OpLoad %v4float %v
%112 = OpCompositeExtract %float %111 3
%113 = OpCompositeConstruct %v4float %110 %float_1 %float_1 %112
OpStore %v %113
%114 = OpLoad %v4float %v
%115 = OpCompositeExtract %float %114 0
%116 = OpCompositeConstruct %v4float %115 %float_1 %float_0 %float_1
OpStore %v %116
%117 = OpLoad %v4float %v
%118 = OpVectorShuffle %v3float %117 %117 1 2 3
%119 = OpCompositeExtract %float %118 0
%120 = OpCompositeExtract %float %118 1
%121 = OpCompositeExtract %float %118 2
%122 = OpCompositeConstruct %v4float %float_1 %119 %120 %121
OpStore %v %122
%123 = OpLoad %v4float %v
%124 = OpVectorShuffle %v2float %123 %123 1 2
%125 = OpCompositeExtract %float %124 0
%126 = OpCompositeExtract %float %124 1
%127 = OpCompositeConstruct %v4float %float_0 %125 %126 %float_1
OpStore %v %127
%128 = OpLoad %v4float %v
%129 = OpCompositeExtract %float %128 1
%130 = OpLoad %v4float %v
%131 = OpCompositeExtract %float %130 3
%132 = OpCompositeConstruct %v4float %float_0 %129 %float_1 %131
OpStore %v %132
%133 = OpLoad %v4float %v
%134 = OpCompositeExtract %float %133 1
%135 = OpCompositeConstruct %v4float %float_1 %134 %float_1 %float_1
OpStore %v %135
%136 = OpLoad %v4float %v
%137 = OpVectorShuffle %v2float %136 %136 2 3
%138 = OpCompositeExtract %float %137 0
%139 = OpCompositeExtract %float %137 1
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %138 %139
OpStore %v %140
%141 = OpLoad %v4float %v
%142 = OpCompositeExtract %float %141 2
%143 = OpCompositeConstruct %v4float %float_0 %float_0 %142 %float_1
OpStore %v %143
%144 = OpLoad %v4float %v
%145 = OpCompositeExtract %float %144 3
%146 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %145
OpStore %v %146
%147 = OpLoad %v4float %v
%149 = OpFOrdEqual %v4bool %147 %148
%151 = OpAll %bool %149
OpSelectionMerge %155 None
OpBranchConditional %151 %153 %154
%153 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%158 = OpLoad %v4float %156
OpStore %152 %158
OpBranch %155
%154 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%161 = OpLoad %v4float %159
OpStore %152 %161
OpBranch %155
%155 = OpLabel
%162 = OpLoad %v4float %152
OpReturnValue %162
OpFunctionEnd
