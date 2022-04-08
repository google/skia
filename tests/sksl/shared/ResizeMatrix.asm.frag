OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%34 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%36 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%52 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%53 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%54 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%55 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%85 = OpConstantComposite %v2float %float_1 %float_0
%86 = OpConstantComposite %v2float %float_0 %float_1
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%result = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%143 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%32 = OpCompositeConstruct %mat3v3float %34 %35 %36
%39 = OpCompositeExtract %v3float %32 0
%40 = OpVectorShuffle %v2float %39 %39 0 1
%41 = OpCompositeExtract %v3float %32 1
%42 = OpVectorShuffle %v2float %41 %41 0 1
%38 = OpCompositeConstruct %mat2v2float %40 %42
OpStore %a %38
%43 = OpLoad %float %result
%46 = OpAccessChain %_ptr_Function_v2float %a %int_0
%47 = OpLoad %v2float %46
%48 = OpCompositeExtract %float %47 0
%49 = OpFAdd %float %43 %48
OpStore %result %49
%51 = OpCompositeConstruct %mat4v4float %52 %53 %54 %55
%58 = OpCompositeExtract %v4float %51 0
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpCompositeExtract %v4float %51 1
%61 = OpVectorShuffle %v2float %60 %60 0 1
%57 = OpCompositeConstruct %mat2v2float %59 %61
OpStore %b %57
%62 = OpLoad %float %result
%63 = OpAccessChain %_ptr_Function_v2float %b %int_0
%64 = OpLoad %v2float %63
%65 = OpCompositeExtract %float %64 0
%66 = OpFAdd %float %62 %65
OpStore %result %66
%69 = OpCompositeConstruct %mat4v4float %52 %53 %54 %55
%71 = OpCompositeExtract %v4float %69 0
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%73 = OpCompositeExtract %v4float %69 1
%74 = OpVectorShuffle %v3float %73 %73 0 1 2
%75 = OpCompositeExtract %v4float %69 2
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%70 = OpCompositeConstruct %mat3v3float %72 %74 %76
OpStore %c %70
%77 = OpLoad %float %result
%78 = OpAccessChain %_ptr_Function_v3float %c %int_0
%80 = OpLoad %v3float %78
%81 = OpCompositeExtract %float %80 0
%82 = OpFAdd %float %77 %81
OpStore %result %82
%84 = OpCompositeConstruct %mat2v2float %85 %86
%88 = OpCompositeExtract %v2float %84 0
%89 = OpCompositeConstruct %v3float %88 %float_0
%90 = OpCompositeExtract %v2float %84 1
%91 = OpCompositeConstruct %v3float %90 %float_0
%92 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%87 = OpCompositeConstruct %mat3v3float %89 %91 %92
OpStore %d %87
%93 = OpLoad %float %result
%94 = OpAccessChain %_ptr_Function_v3float %d %int_0
%95 = OpLoad %v3float %94
%96 = OpCompositeExtract %float %95 0
%97 = OpFAdd %float %93 %96
OpStore %result %97
%100 = OpCompositeConstruct %mat2v2float %85 %86
%102 = OpCompositeExtract %v2float %100 0
%103 = OpCompositeConstruct %v3float %102 %float_0
%104 = OpCompositeExtract %v2float %100 1
%105 = OpCompositeConstruct %v3float %104 %float_0
%106 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%101 = OpCompositeConstruct %mat3v3float %103 %105 %106
%108 = OpCompositeExtract %v3float %101 0
%109 = OpCompositeConstruct %v4float %108 %float_0
%110 = OpCompositeExtract %v3float %101 1
%111 = OpCompositeConstruct %v4float %110 %float_0
%112 = OpCompositeExtract %v3float %101 2
%113 = OpCompositeConstruct %v4float %112 %float_0
%114 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%107 = OpCompositeConstruct %mat4v4float %109 %111 %113 %114
OpStore %e %107
%115 = OpLoad %float %result
%116 = OpAccessChain %_ptr_Function_v4float %e %int_0
%118 = OpLoad %v4float %116
%119 = OpCompositeExtract %float %118 0
%120 = OpFAdd %float %115 %119
OpStore %result %120
%122 = OpCompositeConstruct %mat4v4float %52 %53 %54 %55
%124 = OpCompositeExtract %v4float %122 0
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%126 = OpCompositeExtract %v4float %122 1
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpCompositeExtract %v4float %122 2
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%123 = OpCompositeConstruct %mat3v3float %125 %127 %129
%131 = OpCompositeExtract %v3float %123 0
%132 = OpVectorShuffle %v2float %131 %131 0 1
%133 = OpCompositeExtract %v3float %123 1
%134 = OpVectorShuffle %v2float %133 %133 0 1
%130 = OpCompositeConstruct %mat2v2float %132 %134
OpStore %f %130
%135 = OpLoad %float %result
%136 = OpAccessChain %_ptr_Function_v2float %f %int_0
%137 = OpLoad %v2float %136
%138 = OpCompositeExtract %float %137 0
%139 = OpFAdd %float %135 %138
OpStore %result %139
%140 = OpLoad %float %result
%142 = OpFOrdEqual %bool %140 %float_6
OpSelectionMerge %146 None
OpBranchConditional %142 %144 %145
%144 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%149 = OpLoad %v4float %147
OpStore %143 %149
OpBranch %146
%145 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%152 = OpLoad %v4float %150
OpStore %143 %152
OpBranch %146
%146 = OpLabel
%153 = OpLoad %v4float %143
OpReturnValue %153
OpFunctionEnd
