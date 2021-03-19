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
OpDecorate %26 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_bool = OpTypePointer Function %bool
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
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
%141 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 1
OpStore %f %27
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%31 = OpLoad %v4float %30
%32 = OpCompositeExtract %float %31 1
%33 = OpConvertFToS %int %32
OpStore %i %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %37
%39 = OpCompositeExtract %float %38 1
%40 = OpConvertFToU %uint %39
OpStore %u %40
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpCompositeExtract %float %44 1
%46 = OpFUnordNotEqual %bool %45 %float_0
OpStore %b %46
%49 = OpLoad %float %f
OpStore %f1 %49
%51 = OpLoad %int %i
%52 = OpConvertSToF %float %51
OpStore %f2 %52
%54 = OpLoad %uint %u
%55 = OpConvertUToF %float %54
OpStore %f3 %55
%57 = OpLoad %bool %b
%58 = OpSelect %float %57 %float_1 %float_0
OpStore %f4 %58
%61 = OpLoad %float %f
%62 = OpConvertFToS %int %61
OpStore %i1 %62
%64 = OpLoad %int %i
OpStore %i2 %64
%66 = OpLoad %uint %u
%67 = OpBitcast %int %66
OpStore %i3 %67
%69 = OpLoad %bool %b
%70 = OpSelect %int %69 %int_1 %int_0
OpStore %i4 %70
%73 = OpLoad %float %f
%74 = OpConvertFToU %uint %73
OpStore %u1 %74
%76 = OpLoad %int %i
%77 = OpBitcast %uint %76
OpStore %u2 %77
%79 = OpLoad %uint %u
OpStore %u3 %79
%81 = OpLoad %bool %b
%82 = OpSelect %uint %81 %uint_1 %uint_0
OpStore %u4 %82
%86 = OpLoad %float %f
%87 = OpFUnordNotEqual %bool %86 %float_0
OpStore %b1 %87
%89 = OpLoad %int %i
%90 = OpINotEqual %bool %89 %int_0
OpStore %b2 %90
%92 = OpLoad %uint %u
%93 = OpINotEqual %bool %92 %uint_0
OpStore %b3 %93
%95 = OpLoad %bool %b
OpStore %b4 %95
%96 = OpLoad %float %f1
%97 = OpLoad %float %f2
%98 = OpFAdd %float %96 %97
%99 = OpLoad %float %f3
%100 = OpFAdd %float %98 %99
%101 = OpLoad %float %f4
%102 = OpFAdd %float %100 %101
%103 = OpLoad %int %i1
%104 = OpConvertSToF %float %103
%105 = OpFAdd %float %102 %104
%106 = OpLoad %int %i2
%107 = OpConvertSToF %float %106
%108 = OpFAdd %float %105 %107
%109 = OpLoad %int %i3
%110 = OpConvertSToF %float %109
%111 = OpFAdd %float %108 %110
%112 = OpLoad %int %i4
%113 = OpConvertSToF %float %112
%114 = OpFAdd %float %111 %113
%115 = OpLoad %uint %u1
%116 = OpConvertUToF %float %115
%117 = OpFAdd %float %114 %116
%118 = OpLoad %uint %u2
%119 = OpConvertUToF %float %118
%120 = OpFAdd %float %117 %119
%121 = OpLoad %uint %u3
%122 = OpConvertUToF %float %121
%123 = OpFAdd %float %120 %122
%124 = OpLoad %uint %u4
%125 = OpConvertUToF %float %124
%126 = OpFAdd %float %123 %125
%127 = OpLoad %bool %b1
%128 = OpSelect %float %127 %float_1 %float_0
%129 = OpFAdd %float %126 %128
%130 = OpLoad %bool %b2
%131 = OpSelect %float %130 %float_1 %float_0
%132 = OpFAdd %float %129 %131
%133 = OpLoad %bool %b3
%134 = OpSelect %float %133 %float_1 %float_0
%135 = OpFAdd %float %132 %134
%136 = OpLoad %bool %b4
%137 = OpSelect %float %136 %float_1 %float_0
%138 = OpFAdd %float %135 %137
%140 = OpFOrdEqual %bool %138 %float_16
OpSelectionMerge %145 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%147 = OpLoad %v4float %146
OpStore %141 %147
OpBranch %145
%144 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%149 = OpLoad %v4float %148
OpStore %141 %149
OpBranch %145
%145 = OpLabel
%150 = OpLoad %v4float %141
OpReturnValue %150
OpFunctionEnd
