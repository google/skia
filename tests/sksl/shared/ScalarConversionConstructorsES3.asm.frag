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
OpName %f "f"
OpName %i "i"
OpName %u "u"
OpName %b "b"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %u1 "u1"
OpName %u2 "u2"
OpName %u3 "u3"
OpName %u4 "u4"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
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
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_bool = OpTypePointer Function %bool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%u = OpVariable %_ptr_Function_uint Function
%b = OpVariable %_ptr_Function_bool Function
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_int Function
%u1 = OpVariable %_ptr_Function_uint Function
%u2 = OpVariable %_ptr_Function_uint Function
%u3 = OpVariable %_ptr_Function_uint Function
%u4 = OpVariable %_ptr_Function_uint Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
%b4 = OpVariable %_ptr_Function_bool Function
%146 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpCompositeExtract %float %32 1
OpStore %f %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %36
%38 = OpCompositeExtract %float %37 1
%39 = OpConvertFToS %int %38
OpStore %i %39
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpCompositeExtract %float %44 1
%46 = OpConvertFToU %uint %45
OpStore %u %46
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 1
%52 = OpFUnordNotEqual %bool %51 %float_0
OpStore %b %52
%54 = OpLoad %float %f
OpStore %f1 %54
%56 = OpLoad %int %i
%57 = OpConvertSToF %float %56
OpStore %f2 %57
%59 = OpLoad %uint %u
%60 = OpConvertUToF %float %59
OpStore %f3 %60
%62 = OpLoad %bool %b
%63 = OpSelect %float %62 %float_1 %float_0
OpStore %f4 %63
%66 = OpLoad %float %f
%67 = OpConvertFToS %int %66
OpStore %i1 %67
%69 = OpLoad %int %i
OpStore %i2 %69
%71 = OpLoad %uint %u
%72 = OpBitcast %int %71
OpStore %i3 %72
%74 = OpLoad %bool %b
%75 = OpSelect %int %74 %int_1 %int_0
OpStore %i4 %75
%78 = OpLoad %float %f
%79 = OpConvertFToU %uint %78
OpStore %u1 %79
%81 = OpLoad %int %i
%82 = OpBitcast %uint %81
OpStore %u2 %82
%84 = OpLoad %uint %u
OpStore %u3 %84
%86 = OpLoad %bool %b
%87 = OpSelect %uint %86 %uint_1 %uint_0
OpStore %u4 %87
%91 = OpLoad %float %f
%92 = OpFUnordNotEqual %bool %91 %float_0
OpStore %b1 %92
%94 = OpLoad %int %i
%95 = OpINotEqual %bool %94 %int_0
OpStore %b2 %95
%97 = OpLoad %uint %u
%98 = OpINotEqual %bool %97 %uint_0
OpStore %b3 %98
%100 = OpLoad %bool %b
OpStore %b4 %100
%101 = OpLoad %float %f1
%102 = OpLoad %float %f2
%103 = OpFAdd %float %101 %102
%104 = OpLoad %float %f3
%105 = OpFAdd %float %103 %104
%106 = OpLoad %float %f4
%107 = OpFAdd %float %105 %106
%108 = OpLoad %int %i1
%109 = OpConvertSToF %float %108
%110 = OpFAdd %float %107 %109
%111 = OpLoad %int %i2
%112 = OpConvertSToF %float %111
%113 = OpFAdd %float %110 %112
%114 = OpLoad %int %i3
%115 = OpConvertSToF %float %114
%116 = OpFAdd %float %113 %115
%117 = OpLoad %int %i4
%118 = OpConvertSToF %float %117
%119 = OpFAdd %float %116 %118
%120 = OpLoad %uint %u1
%121 = OpConvertUToF %float %120
%122 = OpFAdd %float %119 %121
%123 = OpLoad %uint %u2
%124 = OpConvertUToF %float %123
%125 = OpFAdd %float %122 %124
%126 = OpLoad %uint %u3
%127 = OpConvertUToF %float %126
%128 = OpFAdd %float %125 %127
%129 = OpLoad %uint %u4
%130 = OpConvertUToF %float %129
%131 = OpFAdd %float %128 %130
%132 = OpLoad %bool %b1
%133 = OpSelect %float %132 %float_1 %float_0
%134 = OpFAdd %float %131 %133
%135 = OpLoad %bool %b2
%136 = OpSelect %float %135 %float_1 %float_0
%137 = OpFAdd %float %134 %136
%138 = OpLoad %bool %b3
%139 = OpSelect %float %138 %float_1 %float_0
%140 = OpFAdd %float %137 %139
%141 = OpLoad %bool %b4
%142 = OpSelect %float %141 %float_1 %float_0
%143 = OpFAdd %float %140 %142
%145 = OpFOrdEqual %bool %143 %float_16
OpSelectionMerge %150 None
OpBranchConditional %145 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%152 = OpLoad %v4float %151
OpStore %146 %152
OpBranch %150
%149 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %153
OpStore %146 %154
OpBranch %150
%150 = OpLabel
%155 = OpLoad %v4float %146
OpReturnValue %155
OpFunctionEnd
