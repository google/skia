OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colRGB"
OpName %fn "fn"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 DescriptorSet 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v3float = OpTypeVector %float 3
%_UniformBuffer = OpTypeStruct %v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%15 = OpTypeFunction %float %_ptr_Function_float
%int = OpTypeInt 32 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%void = OpTypeVoid
%28 = OpTypeFunction %void
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%float_123 = OpConstant %float 123
%float_456 = OpConstant %float 456
%97 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%int_0 = OpConstant %int 0
%fn = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpLabel
%19 = OpLoad %float %17
%20 = OpConvertFToS %int %19
OpSelectionMerge %22 None
OpSwitch %20 %24 1 %23
%23 = OpLabel
OpReturnValue %float_2
%24 = OpLabel
OpReturnValue %float_3
%22 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %28
%29 = OpLabel
%v = OpVariable %_ptr_Function_float Function
%68 = OpVariable %_ptr_Function_float Function
%75 = OpVariable %_ptr_Function_float Function
%80 = OpVariable %_ptr_Function_float Function
%84 = OpVariable %_ptr_Function_float Function
%88 = OpVariable %_ptr_Function_float Function
%93 = OpVariable %_ptr_Function_float Function
%31 = OpExtInst %float %1 Sqrt %float_1
OpStore %v %31
%33 = OpLoad %float %v
%34 = OpCompositeConstruct %v4float %33 %33 %33 %33
OpStore %sk_FragColor %34
%36 = OpLoad %float %v
%37 = OpCompositeConstruct %v3float %36 %36 %36
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpCompositeConstruct %v4float %float_0 %38 %39 %40
OpStore %sk_FragColor %41
%42 = OpLoad %float %v
%43 = OpCompositeConstruct %v2float %42 %42
%45 = OpCompositeExtract %float %43 0
%46 = OpCompositeExtract %float %43 1
%47 = OpCompositeConstruct %v4float %float_0 %float_0 %45 %46
OpStore %sk_FragColor %47
%48 = OpLoad %float %v
%49 = OpCompositeConstruct %v2float %48 %48
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpCompositeConstruct %v4float %float_1 %float_1 %50 %51
OpStore %sk_FragColor %52
%53 = OpLoad %float %v
%54 = OpCompositeConstruct %v2float %53 %53
%55 = OpCompositeExtract %float %54 0
%56 = OpCompositeExtract %float %54 1
%57 = OpCompositeConstruct %v4float %55 %56 %float_1 %float_1
OpStore %sk_FragColor %57
%58 = OpLoad %float %v
%59 = OpCompositeConstruct %v4float %58 %58 %58 %58
OpStore %sk_FragColor %59
%60 = OpLoad %float %v
%61 = OpCompositeConstruct %v2float %60 %60
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeConstruct %v4float %62 %63 %float_1 %float_1
OpStore %sk_FragColor %64
%65 = OpLoad %float %v
%66 = OpCompositeConstruct %v4float %65 %65 %65 %65
OpStore %sk_FragColor %66
%67 = OpLoad %float %v
OpStore %68 %67
%69 = OpFunctionCall %float %fn %68
%72 = OpCompositeConstruct %v3float %69 %float_123 %float_456
%73 = OpVectorShuffle %v4float %72 %72 1 1 2 2
OpStore %sk_FragColor %73
%74 = OpLoad %float %v
OpStore %75 %74
%76 = OpFunctionCall %float %fn %75
%77 = OpCompositeConstruct %v3float %76 %float_123 %float_456
%78 = OpVectorShuffle %v4float %77 %77 1 1 2 2
OpStore %sk_FragColor %78
%79 = OpLoad %float %v
OpStore %80 %79
%81 = OpFunctionCall %float %fn %80
%82 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %81
OpStore %sk_FragColor %82
%83 = OpLoad %float %v
OpStore %84 %83
%85 = OpFunctionCall %float %fn %84
%86 = OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %85
OpStore %sk_FragColor %86
%87 = OpLoad %float %v
OpStore %88 %87
%89 = OpFunctionCall %float %fn %88
%90 = OpCompositeConstruct %v3float %89 %float_123 %float_456
%91 = OpVectorShuffle %v4float %90 %90 1 0 0 2
OpStore %sk_FragColor %91
%92 = OpLoad %float %v
OpStore %93 %92
%94 = OpFunctionCall %float %fn %93
%95 = OpCompositeConstruct %v3float %94 %float_123 %float_456
%96 = OpVectorShuffle %v4float %95 %95 1 0 0 2
OpStore %sk_FragColor %96
OpStore %sk_FragColor %97
%98 = OpAccessChain %_ptr_Uniform_v3float %11 %int_0
%101 = OpLoad %v3float %98
%102 = OpCompositeExtract %float %101 0
%103 = OpCompositeExtract %float %101 1
%104 = OpCompositeExtract %float %101 2
%105 = OpCompositeConstruct %v4float %102 %103 %104 %float_1
OpStore %sk_FragColor %105
%106 = OpAccessChain %_ptr_Uniform_v3float %11 %int_0
%107 = OpLoad %v3float %106
%108 = OpCompositeExtract %float %107 0
%109 = OpAccessChain %_ptr_Uniform_v3float %11 %int_0
%110 = OpLoad %v3float %109
%111 = OpVectorShuffle %v2float %110 %110 1 2
%112 = OpCompositeExtract %float %111 0
%113 = OpCompositeExtract %float %111 1
%114 = OpCompositeConstruct %v4float %108 %float_1 %112 %113
OpStore %sk_FragColor %114
%115 = OpLoad %v4float %sk_FragColor
%116 = OpLoad %v4float %sk_FragColor
%117 = OpVectorShuffle %v4float %116 %115 4 5 6 7
OpStore %sk_FragColor %117
%118 = OpLoad %v4float %sk_FragColor
%119 = OpLoad %v4float %sk_FragColor
%120 = OpVectorShuffle %v4float %119 %118 7 6 5 4
OpStore %sk_FragColor %120
%121 = OpLoad %v4float %sk_FragColor
%122 = OpVectorShuffle %v2float %121 %121 1 2
%123 = OpLoad %v4float %sk_FragColor
%124 = OpVectorShuffle %v4float %123 %122 4 1 2 5
OpStore %sk_FragColor %124
%125 = OpLoad %v4float %sk_FragColor
%126 = OpVectorShuffle %v2float %125 %125 3 3
%127 = OpCompositeExtract %float %126 0
%128 = OpCompositeExtract %float %126 1
%129 = OpCompositeConstruct %v3float %127 %128 %float_1
%130 = OpLoad %v4float %sk_FragColor
%131 = OpVectorShuffle %v4float %130 %129 6 5 4 3
OpStore %sk_FragColor %131
OpReturn
OpFunctionEnd
