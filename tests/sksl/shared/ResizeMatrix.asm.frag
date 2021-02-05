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
OpName %result "result"
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
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%result = OpVariable %_ptr_Function_float Function
%23 = OpVariable %_ptr_Function_mat2v2float Function
%46 = OpVariable %_ptr_Function_mat2v2float Function
%63 = OpVariable %_ptr_Function_mat3v3float Function
%83 = OpVariable %_ptr_Function_mat3v3float Function
%98 = OpVariable %_ptr_Function_mat4v4float Function
%123 = OpVariable %_ptr_Function_mat2v2float Function
%148 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%30 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%31 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%32 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%28 = OpCompositeConstruct %mat3v3float %30 %31 %32
%35 = OpCompositeExtract %v3float %28 0
%36 = OpVectorShuffle %v2float %35 %35 0 1
%37 = OpCompositeExtract %v3float %28 1
%38 = OpVectorShuffle %v2float %37 %37 0 1
%34 = OpCompositeConstruct %mat2v2float %36 %38
OpStore %23 %34
%41 = OpAccessChain %_ptr_Function_v2float %23 %int_0
%43 = OpLoad %v2float %41
%44 = OpCompositeExtract %float %43 0
OpStore %result %44
%45 = OpLoad %float %result
%48 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%49 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%50 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%47 = OpCompositeConstruct %mat4v4float %48 %49 %50 %51
%54 = OpCompositeExtract %v4float %47 0
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpCompositeExtract %v4float %47 1
%57 = OpVectorShuffle %v2float %56 %56 0 1
%53 = OpCompositeConstruct %mat2v2float %55 %57
OpStore %46 %53
%58 = OpAccessChain %_ptr_Function_v2float %46 %int_0
%59 = OpLoad %v2float %58
%60 = OpCompositeExtract %float %59 0
%61 = OpFAdd %float %45 %60
OpStore %result %61
%62 = OpLoad %float %result
%66 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%67 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%68 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%65 = OpCompositeConstruct %mat4v4float %66 %67 %68 %69
%71 = OpCompositeExtract %v4float %65 0
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%73 = OpCompositeExtract %v4float %65 1
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%75 = OpCompositeExtract %v4float %65 2
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%70 = OpCompositeConstruct %mat3v3float %72 %74 %76
OpStore %63 %70
%77 = OpAccessChain %_ptr_Function_v3float %63 %int_0
%79 = OpLoad %v3float %77
%80 = OpCompositeExtract %float %79 0
%81 = OpFAdd %float %62 %80
OpStore %result %81
%82 = OpLoad %float %result
%85 = OpCompositeConstruct %v2float %float_1 %float_0
%86 = OpCompositeConstruct %v2float %float_0 %float_1
%84 = OpCompositeConstruct %mat2v2float %85 %86
%88 = OpCompositeExtract %v2float %84 0
%89 = OpCompositeConstruct %v3float %88 %float_0
%90 = OpCompositeExtract %v2float %84 1
%91 = OpCompositeConstruct %v3float %90 %float_0
%92 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%87 = OpCompositeConstruct %mat3v3float %89 %91 %92
OpStore %83 %87
%93 = OpAccessChain %_ptr_Function_v3float %83 %int_0
%94 = OpLoad %v3float %93
%95 = OpCompositeExtract %float %94 0
%96 = OpFAdd %float %82 %95
OpStore %result %96
%97 = OpLoad %float %result
%101 = OpCompositeConstruct %v2float %float_1 %float_0
%102 = OpCompositeConstruct %v2float %float_0 %float_1
%100 = OpCompositeConstruct %mat2v2float %101 %102
%104 = OpCompositeExtract %v2float %100 0
%105 = OpCompositeConstruct %v3float %104 %float_0
%106 = OpCompositeExtract %v2float %100 1
%107 = OpCompositeConstruct %v3float %106 %float_0
%108 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%103 = OpCompositeConstruct %mat3v3float %105 %107 %108
%110 = OpCompositeExtract %v3float %103 0
%111 = OpCompositeConstruct %v4float %110 %float_0
%112 = OpCompositeExtract %v3float %103 1
%113 = OpCompositeConstruct %v4float %112 %float_0
%114 = OpCompositeExtract %v3float %103 2
%115 = OpCompositeConstruct %v4float %114 %float_0
%116 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%109 = OpCompositeConstruct %mat4v4float %111 %113 %115 %116
OpStore %98 %109
%117 = OpAccessChain %_ptr_Function_v4float %98 %int_0
%119 = OpLoad %v4float %117
%120 = OpCompositeExtract %float %119 0
%121 = OpFAdd %float %97 %120
OpStore %result %121
%122 = OpLoad %float %result
%125 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%126 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%127 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%124 = OpCompositeConstruct %mat4v4float %125 %126 %127 %128
%130 = OpCompositeExtract %v4float %124 0
%131 = OpVectorShuffle %v3float %130 %130 0 1 2
%132 = OpCompositeExtract %v4float %124 1
%133 = OpVectorShuffle %v3float %132 %132 0 1 2
%134 = OpCompositeExtract %v4float %124 2
%135 = OpVectorShuffle %v3float %134 %134 0 1 2
%129 = OpCompositeConstruct %mat3v3float %131 %133 %135
%137 = OpCompositeExtract %v3float %129 0
%138 = OpVectorShuffle %v2float %137 %137 0 1
%139 = OpCompositeExtract %v3float %129 1
%140 = OpVectorShuffle %v2float %139 %139 0 1
%136 = OpCompositeConstruct %mat2v2float %138 %140
OpStore %123 %136
%141 = OpAccessChain %_ptr_Function_v2float %123 %int_0
%142 = OpLoad %v2float %141
%143 = OpCompositeExtract %float %142 0
%144 = OpFAdd %float %122 %143
OpStore %result %144
%145 = OpLoad %float %result
%147 = OpFOrdEqual %bool %145 %float_6
OpSelectionMerge %151 None
OpBranchConditional %147 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%154 = OpLoad %v4float %152
OpStore %148 %154
OpBranch %151
%150 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%157 = OpLoad %v4float %155
OpStore %148 %157
OpBranch %151
%151 = OpLabel
%158 = OpLoad %v4float %148
OpReturnValue %158
OpFunctionEnd
