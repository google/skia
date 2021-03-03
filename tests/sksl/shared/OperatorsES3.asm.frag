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
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_1 = OpConstant %int 1
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_5 = OpConstant %int 5
%float_6 = OpConstant %float 6
%int_6 = OpConstant %int 6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
%135 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%29 = OpLoad %float %x
%30 = OpLoad %float %x
%31 = OpFSub %float %29 %30
%32 = OpLoad %float %y
%33 = OpLoad %float %x
%34 = OpFMul %float %32 %33
%35 = OpLoad %float %x
%36 = OpFMul %float %34 %35
%37 = OpLoad %float %y
%38 = OpLoad %float %x
%39 = OpFSub %float %37 %38
%40 = OpFMul %float %36 %39
%41 = OpFAdd %float %31 %40
OpStore %x %41
%42 = OpLoad %float %x
%43 = OpLoad %float %y
%44 = OpFDiv %float %42 %43
%45 = OpLoad %float %x
%46 = OpFDiv %float %44 %45
OpStore %y %46
%47 = OpLoad %int %z
%49 = OpSDiv %int %47 %int_2
%50 = OpSMod %int %49 %int_3
%52 = OpShiftLeftLogical %int %50 %int_4
%53 = OpShiftRightArithmetic %int %52 %int_2
%55 = OpShiftLeftLogical %int %53 %int_1
OpStore %z %55
%59 = OpLoad %float %x
%61 = OpFOrdGreaterThan %bool %59 %float_4
%62 = OpLoad %float %x
%63 = OpFOrdLessThan %bool %62 %float_2
%64 = OpLogicalEqual %bool %61 %63
OpSelectionMerge %66 None
OpBranchConditional %64 %66 %65
%65 = OpLabel
%68 = OpExtInst %float %1 Sqrt %float_2
%69 = OpFOrdGreaterThanEqual %bool %float_2 %68
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %float %y
%73 = OpLoad %float %x
%74 = OpFOrdLessThanEqual %bool %72 %73
OpBranch %71
%71 = OpLabel
%75 = OpPhi %bool %false %65 %74 %70
OpBranch %66
%66 = OpLabel
%76 = OpPhi %bool %true %19 %75 %71
OpStore %b %76
%78 = OpExtInst %float %1 Sqrt %float_2
%79 = OpFOrdGreaterThan %bool %78 %float_2
OpStore %c %79
%81 = OpLoad %bool %b
%82 = OpLoad %bool %c
%83 = OpLogicalNotEqual %bool %81 %82
OpStore %d %83
%85 = OpLoad %bool %b
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %bool %c
OpBranch %87
%87 = OpLabel
%89 = OpPhi %bool %false %66 %88 %86
OpStore %e %89
%91 = OpLoad %bool %b
OpSelectionMerge %93 None
OpBranchConditional %91 %93 %92
%92 = OpLabel
%94 = OpLoad %bool %c
OpBranch %93
%93 = OpLabel
%95 = OpPhi %bool %true %87 %94 %92
OpStore %f %95
%96 = OpLoad %float %x
%98 = OpFAdd %float %96 %float_12
OpStore %x %98
%99 = OpLoad %float %x
%100 = OpFSub %float %99 %float_12
OpStore %x %100
%101 = OpLoad %float %x
%102 = OpLoad %float %y
%104 = OpFDiv %float %102 %float_10
OpStore %y %104
%105 = OpFMul %float %101 %104
OpStore %x %105
%106 = OpLoad %int %z
%108 = OpBitwiseOr %int %106 %int_0
OpStore %z %108
%109 = OpLoad %int %z
%111 = OpBitwiseAnd %int %109 %int_n1
OpStore %z %111
%112 = OpLoad %int %z
%113 = OpBitwiseXor %int %112 %int_0
OpStore %z %113
%114 = OpLoad %int %z
%115 = OpShiftRightArithmetic %int %114 %int_2
OpStore %z %115
%116 = OpLoad %int %z
%117 = OpShiftLeftLogical %int %116 %int_4
OpStore %z %117
%118 = OpLoad %int %z
%120 = OpSMod %int %118 %int_5
OpStore %z %120
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%123 = OpLoad %float %x
%124 = OpFOrdEqual %bool %123 %float_6
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpLoad %float %y
%128 = OpFOrdEqual %bool %127 %float_6
OpBranch %126
%126 = OpLabel
%129 = OpPhi %bool %false %93 %128 %125
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpLoad %int %z
%133 = OpIEqual %bool %132 %int_6
OpBranch %131
%131 = OpLabel
%134 = OpPhi %bool %false %126 %133 %130
OpSelectionMerge %139 None
OpBranchConditional %134 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%142 = OpLoad %v4float %140
OpStore %135 %142
OpBranch %139
%138 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
OpStore %135 %144
OpBranch %139
%139 = OpLabel
%145 = OpLoad %v4float %135
OpReturnValue %145
OpFunctionEnd
