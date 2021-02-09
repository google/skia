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
OpName %main "main"
OpName %_1_m3 "_1_m3"
OpName %_2_m5 "_2_m5"
OpName %_3_m6 "_3_m6"
OpName %_4_m11 "_4_m11"
OpName %_6_m3 "_6_m3"
OpName %_7_m5 "_7_m5"
OpName %_8_m6 "_8_m6"
OpName %_9_m11 "_9_m11"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_1_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m5 = OpVariable %_ptr_Function_mat2v2float Function
%40 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_6_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m5 = OpVariable %_ptr_Function_mat2v2float Function
%111 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m11 = OpVariable %_ptr_Function_mat4v4float Function
%29 = OpCompositeConstruct %v2float %float_1 %float_2
%30 = OpCompositeConstruct %v2float %float_3 %float_4
%28 = OpCompositeConstruct %mat2v2float %29 %30
OpStore %_1_m3 %28
%32 = OpCompositeConstruct %v2float %float_1 %float_2
%33 = OpCompositeConstruct %v2float %float_3 %float_4
%31 = OpCompositeConstruct %mat2v2float %32 %33
%36 = OpCompositeConstruct %v2float %float_1 %float_0
%37 = OpCompositeConstruct %v2float %float_0 %float_1
%34 = OpCompositeConstruct %mat2v2float %36 %37
%38 = OpMatrixTimesMatrix %mat2v2float %31 %34
OpStore %_1_m3 %38
%42 = OpCompositeConstruct %v2float %float_1 %float_2
%43 = OpCompositeConstruct %v2float %float_3 %float_4
%41 = OpCompositeConstruct %mat2v2float %42 %43
OpStore %40 %41
%46 = OpAccessChain %_ptr_Function_v2float %40 %int_0
%48 = OpLoad %v2float %46
%49 = OpCompositeExtract %float %48 0
%51 = OpCompositeConstruct %v2float %49 %float_0
%52 = OpCompositeConstruct %v2float %float_0 %49
%50 = OpCompositeConstruct %mat2v2float %51 %52
OpStore %_2_m5 %50
%55 = OpCompositeConstruct %v2float %float_1 %float_2
%56 = OpCompositeConstruct %v2float %float_3 %float_4
%54 = OpCompositeConstruct %mat2v2float %55 %56
OpStore %_3_m6 %54
%58 = OpCompositeConstruct %v2float %float_1 %float_2
%59 = OpCompositeConstruct %v2float %float_3 %float_4
%57 = OpCompositeConstruct %mat2v2float %58 %59
%60 = OpLoad %mat2v2float %_2_m5
%61 = OpCompositeExtract %v2float %57 0
%62 = OpCompositeExtract %v2float %60 0
%63 = OpFAdd %v2float %61 %62
%64 = OpCompositeExtract %v2float %57 1
%65 = OpCompositeExtract %v2float %60 1
%66 = OpFAdd %v2float %64 %65
%67 = OpCompositeConstruct %mat2v2float %63 %66
OpStore %_3_m6 %67
%72 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%74 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%75 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%71 = OpCompositeConstruct %mat4v4float %72 %73 %74 %75
OpStore %_4_m11 %71
%77 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%80 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%76 = OpCompositeConstruct %mat4v4float %77 %78 %79 %80
%82 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%83 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%84 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%85 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%81 = OpCompositeConstruct %mat4v4float %82 %83 %84 %85
%86 = OpCompositeExtract %v4float %76 0
%87 = OpCompositeExtract %v4float %81 0
%88 = OpFSub %v4float %86 %87
%89 = OpCompositeExtract %v4float %76 1
%90 = OpCompositeExtract %v4float %81 1
%91 = OpFSub %v4float %89 %90
%92 = OpCompositeExtract %v4float %76 2
%93 = OpCompositeExtract %v4float %81 2
%94 = OpFSub %v4float %92 %93
%95 = OpCompositeExtract %v4float %76 3
%96 = OpCompositeExtract %v4float %81 3
%97 = OpFSub %v4float %95 %96
%98 = OpCompositeConstruct %mat4v4float %88 %91 %94 %97
OpStore %_4_m11 %98
%101 = OpCompositeConstruct %v2float %float_1 %float_2
%102 = OpCompositeConstruct %v2float %float_3 %float_4
%100 = OpCompositeConstruct %mat2v2float %101 %102
OpStore %_6_m3 %100
%104 = OpCompositeConstruct %v2float %float_1 %float_2
%105 = OpCompositeConstruct %v2float %float_3 %float_4
%103 = OpCompositeConstruct %mat2v2float %104 %105
%107 = OpCompositeConstruct %v2float %float_1 %float_0
%108 = OpCompositeConstruct %v2float %float_0 %float_1
%106 = OpCompositeConstruct %mat2v2float %107 %108
%109 = OpMatrixTimesMatrix %mat2v2float %103 %106
OpStore %_6_m3 %109
%113 = OpCompositeConstruct %v2float %float_1 %float_2
%114 = OpCompositeConstruct %v2float %float_3 %float_4
%112 = OpCompositeConstruct %mat2v2float %113 %114
OpStore %111 %112
%115 = OpAccessChain %_ptr_Function_v2float %111 %int_0
%116 = OpLoad %v2float %115
%117 = OpCompositeExtract %float %116 0
%119 = OpCompositeConstruct %v2float %117 %float_0
%120 = OpCompositeConstruct %v2float %float_0 %117
%118 = OpCompositeConstruct %mat2v2float %119 %120
OpStore %_7_m5 %118
%123 = OpCompositeConstruct %v2float %float_1 %float_2
%124 = OpCompositeConstruct %v2float %float_3 %float_4
%122 = OpCompositeConstruct %mat2v2float %123 %124
OpStore %_8_m6 %122
%126 = OpCompositeConstruct %v2float %float_1 %float_2
%127 = OpCompositeConstruct %v2float %float_3 %float_4
%125 = OpCompositeConstruct %mat2v2float %126 %127
%128 = OpLoad %mat2v2float %_7_m5
%129 = OpCompositeExtract %v2float %125 0
%130 = OpCompositeExtract %v2float %128 0
%131 = OpFAdd %v2float %129 %130
%132 = OpCompositeExtract %v2float %125 1
%133 = OpCompositeExtract %v2float %128 1
%134 = OpFAdd %v2float %132 %133
%135 = OpCompositeConstruct %mat2v2float %131 %134
OpStore %_8_m6 %135
%138 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%139 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%141 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%137 = OpCompositeConstruct %mat4v4float %138 %139 %140 %141
OpStore %_9_m11 %137
%143 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%144 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%145 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%146 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%142 = OpCompositeConstruct %mat4v4float %143 %144 %145 %146
%148 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%149 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%150 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%151 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%147 = OpCompositeConstruct %mat4v4float %148 %149 %150 %151
%152 = OpCompositeExtract %v4float %142 0
%153 = OpCompositeExtract %v4float %147 0
%154 = OpFSub %v4float %152 %153
%155 = OpCompositeExtract %v4float %142 1
%156 = OpCompositeExtract %v4float %147 1
%157 = OpFSub %v4float %155 %156
%158 = OpCompositeExtract %v4float %142 2
%159 = OpCompositeExtract %v4float %147 2
%160 = OpFSub %v4float %158 %159
%161 = OpCompositeExtract %v4float %142 3
%162 = OpCompositeExtract %v4float %147 3
%163 = OpFSub %v4float %161 %162
%164 = OpCompositeConstruct %mat4v4float %154 %157 %160 %163
OpStore %_9_m11 %164
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%167 = OpLoad %v4float %165
OpReturnValue %167
OpFunctionEnd
