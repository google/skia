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
OpMemberName %_UniformBuffer 2 "unknownInput"
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
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %74 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
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
%144 = OpVariable %_ptr_Function_v4float Function
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
%72 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%74 = OpLoad %float %72
%75 = OpFOrdGreaterThanEqual %bool %float_2 %74
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpLoad %float %y
%79 = OpLoad %float %x
%80 = OpFOrdLessThanEqual %bool %78 %79
OpBranch %77
%77 = OpLabel
%81 = OpPhi %bool %false %69 %80 %76
OpBranch %70
%70 = OpLabel
%82 = OpPhi %bool %true %25 %81 %77
OpStore %b %82
%84 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%85 = OpLoad %float %84
%86 = OpFOrdGreaterThan %bool %85 %float_2
OpStore %c %86
%88 = OpLoad %bool %b
%89 = OpLoad %bool %c
%90 = OpLogicalNotEqual %bool %88 %89
OpStore %d %90
%92 = OpLoad %bool %b
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %bool %c
OpBranch %94
%94 = OpLabel
%96 = OpPhi %bool %false %70 %95 %93
OpStore %e %96
%98 = OpLoad %bool %b
OpSelectionMerge %100 None
OpBranchConditional %98 %100 %99
%99 = OpLabel
%101 = OpLoad %bool %c
OpBranch %100
%100 = OpLabel
%102 = OpPhi %bool %true %94 %101 %99
OpStore %f %102
%103 = OpLoad %float %x
%105 = OpFAdd %float %103 %float_12
OpStore %x %105
%106 = OpLoad %float %x
%107 = OpFSub %float %106 %float_12
OpStore %x %107
%108 = OpLoad %float %x
%109 = OpLoad %float %y
%111 = OpFDiv %float %109 %float_10
OpStore %y %111
%112 = OpFMul %float %108 %111
OpStore %x %112
OpStore %x %float_6
%114 = OpLoad %bool %b
%115 = OpSelect %float %114 %float_1 %float_0
%116 = OpLoad %bool %c
%117 = OpSelect %float %116 %float_1 %float_0
%118 = OpFMul %float %115 %117
%119 = OpLoad %bool %d
%120 = OpSelect %float %119 %float_1 %float_0
%121 = OpFMul %float %118 %120
%122 = OpLoad %bool %e
%123 = OpSelect %float %122 %float_1 %float_0
%124 = OpFMul %float %121 %123
%125 = OpLoad %bool %f
%126 = OpSelect %float %125 %float_1 %float_0
%127 = OpFMul %float %124 %126
OpStore %y %127
OpStore %y %float_6
%128 = OpLoad %int %z
%130 = OpISub %int %128 %int_1
OpStore %z %130
OpStore %z %int_6
%132 = OpLoad %float %x
%133 = OpFOrdEqual %bool %132 %float_6
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %float %y
%137 = OpFOrdEqual %bool %136 %float_6
OpBranch %135
%135 = OpLabel
%138 = OpPhi %bool %false %100 %137 %134
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpLoad %int %z
%142 = OpIEqual %bool %141 %int_6
OpBranch %140
%140 = OpLabel
%143 = OpPhi %bool %false %135 %142 %139
OpSelectionMerge %148 None
OpBranchConditional %143 %146 %147
%146 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%152 = OpLoad %v4float %149
OpStore %144 %152
OpBranch %148
%147 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %153
OpStore %144 %154
OpBranch %148
%148 = OpLabel
%155 = OpLoad %v4float %144
OpReturnValue %155
OpFunctionEnd
