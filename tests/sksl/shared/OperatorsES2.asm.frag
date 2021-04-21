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
OpName %x "x"
OpName %y "y"
OpName %z "z"
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
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%float_6 = OpConstant %float 6
%int_1 = OpConstant %int 1
%int_6 = OpConstant %int 6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
%141 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpLoad %float %x
%36 = OpLoad %float %x
%37 = OpFSub %float %35 %36
%38 = OpLoad %float %y
%39 = OpLoad %float %x
%40 = OpFMul %float %38 %39
%41 = OpLoad %float %x
%42 = OpFMul %float %40 %41
%43 = OpLoad %float %y
%44 = OpLoad %float %x
%45 = OpFSub %float %43 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %37 %46
OpStore %x %47
%48 = OpLoad %float %x
%49 = OpLoad %float %y
%50 = OpFDiv %float %48 %49
%51 = OpLoad %float %x
%52 = OpFDiv %float %50 %51
OpStore %y %52
%53 = OpLoad %int %z
%55 = OpSDiv %int %53 %int_2
%56 = OpIMul %int %55 %int_3
%58 = OpIAdd %int %56 %int_4
%59 = OpISub %int %58 %int_2
OpStore %z %59
%63 = OpLoad %float %x
%65 = OpFOrdGreaterThan %bool %63 %float_4
%66 = OpLoad %float %x
%67 = OpFOrdLessThan %bool %66 %float_2
%68 = OpLogicalEqual %bool %65 %67
OpSelectionMerge %70 None
OpBranchConditional %68 %70 %69
%69 = OpLabel
%72 = OpExtInst %float %1 Sqrt %float_2
%73 = OpFOrdGreaterThanEqual %bool %float_2 %72
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %float %y
%77 = OpLoad %float %x
%78 = OpFOrdLessThanEqual %bool %76 %77
OpBranch %75
%75 = OpLabel
%79 = OpPhi %bool %false %69 %78 %74
OpBranch %70
%70 = OpLabel
%80 = OpPhi %bool %true %25 %79 %75
OpStore %b %80
%82 = OpExtInst %float %1 Sqrt %float_2
%83 = OpFOrdGreaterThan %bool %82 %float_2
OpStore %c %83
%85 = OpLoad %bool %b
%86 = OpLoad %bool %c
%87 = OpLogicalNotEqual %bool %85 %86
OpStore %d %87
%89 = OpLoad %bool %b
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %bool %c
OpBranch %91
%91 = OpLabel
%93 = OpPhi %bool %false %70 %92 %90
OpStore %e %93
%95 = OpLoad %bool %b
OpSelectionMerge %97 None
OpBranchConditional %95 %97 %96
%96 = OpLabel
%98 = OpLoad %bool %c
OpBranch %97
%97 = OpLabel
%99 = OpPhi %bool %true %91 %98 %96
OpStore %f %99
%100 = OpLoad %float %x
%102 = OpFAdd %float %100 %float_12
OpStore %x %102
%103 = OpLoad %float %x
%104 = OpFSub %float %103 %float_12
OpStore %x %104
%105 = OpLoad %float %x
%106 = OpLoad %float %y
%108 = OpFDiv %float %106 %float_10
OpStore %y %108
%109 = OpFMul %float %105 %108
OpStore %x %109
OpStore %x %float_6
%111 = OpLoad %bool %b
%112 = OpSelect %float %111 %float_1 %float_0
%113 = OpLoad %bool %c
%114 = OpSelect %float %113 %float_1 %float_0
%115 = OpFMul %float %112 %114
%116 = OpLoad %bool %d
%117 = OpSelect %float %116 %float_1 %float_0
%118 = OpFMul %float %115 %117
%119 = OpLoad %bool %e
%120 = OpSelect %float %119 %float_1 %float_0
%121 = OpFMul %float %118 %120
%122 = OpLoad %bool %f
%123 = OpSelect %float %122 %float_1 %float_0
%124 = OpFMul %float %121 %123
OpStore %y %124
OpStore %y %float_6
%125 = OpLoad %int %z
%127 = OpISub %int %125 %int_1
OpStore %z %127
OpStore %z %int_6
%129 = OpLoad %float %x
%130 = OpFOrdEqual %bool %129 %float_6
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %float %y
%134 = OpFOrdEqual %bool %133 %float_6
OpBranch %132
%132 = OpLabel
%135 = OpPhi %bool %false %97 %134 %131
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpLoad %int %z
%139 = OpIEqual %bool %138 %int_6
OpBranch %137
%137 = OpLabel
%140 = OpPhi %bool %false %132 %139 %136
OpSelectionMerge %145 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%149 = OpLoad %v4float %146
OpStore %141 %149
OpBranch %145
%144 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%151 = OpLoad %v4float %150
OpStore %141 %151
OpBranch %145
%145 = OpLabel
%152 = OpLoad %v4float %141
OpReturnValue %152
OpFunctionEnd
