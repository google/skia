OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_blend_overlay_component_hh2h2 "_blend_overlay_component_hh2h2"
OpName %main "main"
OpName %_0_result "_0_result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
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
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %_0_result RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%58 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%_blend_overlay_component_hh2h2 = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%27 = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v2float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpFMul %float %float_2 %22
%24 = OpLoad %v2float %18
%25 = OpCompositeExtract %float %24 1
%26 = OpFOrdLessThanEqual %bool %23 %25
OpSelectionMerge %31 None
OpBranchConditional %26 %29 %30
%29 = OpLabel
%32 = OpLoad %v2float %17
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %float_2 %33
%35 = OpLoad %v2float %18
%36 = OpCompositeExtract %float %35 0
%37 = OpFMul %float %34 %36
OpStore %27 %37
OpBranch %31
%30 = OpLabel
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 1
%40 = OpLoad %v2float %18
%41 = OpCompositeExtract %float %40 1
%42 = OpFMul %float %39 %41
%43 = OpLoad %v2float %18
%44 = OpCompositeExtract %float %43 1
%45 = OpLoad %v2float %18
%46 = OpCompositeExtract %float %45 0
%47 = OpFSub %float %44 %46
%48 = OpFMul %float %float_2 %47
%49 = OpLoad %v2float %17
%50 = OpCompositeExtract %float %49 1
%51 = OpLoad %v2float %17
%52 = OpCompositeExtract %float %51 0
%53 = OpFSub %float %50 %52
%54 = OpFMul %float %48 %53
%55 = OpFSub %float %42 %54
OpStore %27 %55
OpBranch %31
%31 = OpLabel
%56 = OpLoad %float %27
OpReturnValue %56
OpFunctionEnd
%main = OpFunction %void None %58
%59 = OpLabel
%_0_result = OpVariable %_ptr_Function_v4float Function
%68 = OpVariable %_ptr_Function_v2float Function
%73 = OpVariable %_ptr_Function_v2float Function
%78 = OpVariable %_ptr_Function_v2float Function
%82 = OpVariable %_ptr_Function_v2float Function
%87 = OpVariable %_ptr_Function_v2float Function
%91 = OpVariable %_ptr_Function_v2float Function
%62 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%66 = OpLoad %v4float %62
%67 = OpVectorShuffle %v2float %66 %66 0 3
OpStore %68 %67
%69 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%71 = OpLoad %v4float %69
%72 = OpVectorShuffle %v2float %71 %71 0 3
OpStore %73 %72
%74 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %68 %73
%75 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%76 = OpLoad %v4float %75
%77 = OpVectorShuffle %v2float %76 %76 1 3
OpStore %78 %77
%79 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%80 = OpLoad %v4float %79
%81 = OpVectorShuffle %v2float %80 %80 1 3
OpStore %82 %81
%83 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %78 %82
%84 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%85 = OpLoad %v4float %84
%86 = OpVectorShuffle %v2float %85 %85 2 3
OpStore %87 %86
%88 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v2float %89 %89 2 3
OpStore %91 %90
%92 = OpFunctionCall %float %_blend_overlay_component_hh2h2 %87 %91
%93 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 3
%97 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%98 = OpLoad %v4float %97
%99 = OpCompositeExtract %float %98 3
%100 = OpFSub %float %float_1 %99
%101 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%102 = OpLoad %v4float %101
%103 = OpCompositeExtract %float %102 3
%104 = OpFMul %float %100 %103
%105 = OpFAdd %float %95 %104
%106 = OpCompositeConstruct %v4float %74 %83 %92 %105
OpStore %_0_result %106
%107 = OpLoad %v4float %_0_result
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%110 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%113 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%114 = OpLoad %v4float %113
%115 = OpCompositeExtract %float %114 3
%116 = OpFSub %float %float_1 %115
%117 = OpVectorTimesScalar %v3float %112 %116
%118 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%119 = OpLoad %v4float %118
%120 = OpVectorShuffle %v3float %119 %119 0 1 2
%121 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%122 = OpLoad %v4float %121
%123 = OpCompositeExtract %float %122 3
%124 = OpFSub %float %float_1 %123
%125 = OpVectorTimesScalar %v3float %120 %124
%126 = OpFAdd %v3float %117 %125
%127 = OpFAdd %v3float %108 %126
%128 = OpLoad %v4float %_0_result
%129 = OpVectorShuffle %v4float %128 %127 4 5 6 3
OpStore %_0_result %129
%130 = OpLoad %v4float %_0_result
OpStore %sk_FragColor %130
OpReturn
OpFunctionEnd
