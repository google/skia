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
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%33 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%34 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
%37 = OpConstantComposite %mat3v3float %33 %34 %35
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%49 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%50 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%51 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
%52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
%54 = OpConstantComposite %mat4v4float %49 %50 %51 %52
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%76 = OpConstantComposite %v2float %float_1 %float_0
%77 = OpConstantComposite %v2float %float_0 %float_1
%78 = OpConstantComposite %mat2v2float %76 %77
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
%126 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%39 = OpVectorShuffle %v2float %33 %33 0 1
%40 = OpVectorShuffle %v2float %34 %34 0 1
%38 = OpCompositeConstruct %mat2v2float %39 %40
OpStore %a %38
%41 = OpLoad %float %result
%44 = OpAccessChain %_ptr_Function_v2float %a %int_0
%45 = OpLoad %v2float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFAdd %float %41 %46
OpStore %result %47
%56 = OpVectorShuffle %v2float %49 %49 0 1
%57 = OpVectorShuffle %v2float %50 %50 0 1
%55 = OpCompositeConstruct %mat2v2float %56 %57
OpStore %b %55
%58 = OpLoad %float %result
%59 = OpAccessChain %_ptr_Function_v2float %b %int_0
%60 = OpLoad %v2float %59
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %58 %61
OpStore %result %62
%66 = OpVectorShuffle %v3float %49 %49 0 1 2
%67 = OpVectorShuffle %v3float %50 %50 0 1 2
%68 = OpVectorShuffle %v3float %51 %51 0 1 2
%65 = OpCompositeConstruct %mat3v3float %66 %67 %68
OpStore %c %65
%69 = OpLoad %float %result
%70 = OpAccessChain %_ptr_Function_v3float %c %int_0
%72 = OpLoad %v3float %70
%73 = OpCompositeExtract %float %72 0
%74 = OpFAdd %float %69 %73
OpStore %result %74
%80 = OpCompositeConstruct %v3float %76 %float_0
%81 = OpCompositeConstruct %v3float %77 %float_0
%82 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%79 = OpCompositeConstruct %mat3v3float %80 %81 %82
OpStore %d %79
%83 = OpLoad %float %result
%84 = OpAccessChain %_ptr_Function_v3float %d %int_0
%85 = OpLoad %v3float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFAdd %float %83 %86
OpStore %result %87
%91 = OpCompositeConstruct %v3float %76 %float_0
%92 = OpCompositeConstruct %v3float %77 %float_0
%93 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%90 = OpCompositeConstruct %mat3v3float %91 %92 %93
%95 = OpCompositeExtract %v3float %90 0
%96 = OpCompositeConstruct %v4float %95 %float_0
%97 = OpCompositeExtract %v3float %90 1
%98 = OpCompositeConstruct %v4float %97 %float_0
%99 = OpCompositeExtract %v3float %90 2
%100 = OpCompositeConstruct %v4float %99 %float_0
%101 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%94 = OpCompositeConstruct %mat4v4float %96 %98 %100 %101
OpStore %e %94
%102 = OpLoad %float %result
%103 = OpAccessChain %_ptr_Function_v4float %e %int_0
%105 = OpLoad %v4float %103
%106 = OpCompositeExtract %float %105 0
%107 = OpFAdd %float %102 %106
OpStore %result %107
%110 = OpVectorShuffle %v3float %49 %49 0 1 2
%111 = OpVectorShuffle %v3float %50 %50 0 1 2
%112 = OpVectorShuffle %v3float %51 %51 0 1 2
%109 = OpCompositeConstruct %mat3v3float %110 %111 %112
%114 = OpCompositeExtract %v3float %109 0
%115 = OpVectorShuffle %v2float %114 %114 0 1
%116 = OpCompositeExtract %v3float %109 1
%117 = OpVectorShuffle %v2float %116 %116 0 1
%113 = OpCompositeConstruct %mat2v2float %115 %117
OpStore %f %113
%118 = OpLoad %float %result
%119 = OpAccessChain %_ptr_Function_v2float %f %int_0
%120 = OpLoad %v2float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpFAdd %float %118 %121
OpStore %result %122
%123 = OpLoad %float %result
%125 = OpFOrdEqual %bool %123 %float_6
OpSelectionMerge %129 None
OpBranchConditional %125 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%132 = OpLoad %v4float %130
OpStore %126 %132
OpBranch %129
%128 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%135 = OpLoad %v4float %133
OpStore %126 %135
OpBranch %129
%129 = OpLabel
%136 = OpLoad %v4float %126
OpReturnValue %136
OpFunctionEnd
