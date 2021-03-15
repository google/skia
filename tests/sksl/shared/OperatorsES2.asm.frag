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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%float_6 = OpConstant %float 6
%float_0 = OpConstant %float 0
%int_1 = OpConstant %int 1
%int_6 = OpConstant %int 6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%136 = OpVariable %_ptr_Function_v4float Function
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
%50 = OpIMul %int %49 %int_3
%52 = OpIAdd %int %50 %int_4
%53 = OpISub %int %52 %int_2
OpStore %z %53
%57 = OpLoad %float %x
%59 = OpFOrdGreaterThan %bool %57 %float_4
%60 = OpLoad %float %x
%61 = OpFOrdLessThan %bool %60 %float_2
%62 = OpLogicalEqual %bool %59 %61
OpSelectionMerge %64 None
OpBranchConditional %62 %64 %63
%63 = OpLabel
%66 = OpExtInst %float %1 Sqrt %float_2
%67 = OpFOrdGreaterThanEqual %bool %float_2 %66
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %float %y
%71 = OpLoad %float %x
%72 = OpFOrdLessThanEqual %bool %70 %71
OpBranch %69
%69 = OpLabel
%73 = OpPhi %bool %false %63 %72 %68
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %true %19 %73 %69
OpStore %b %74
%76 = OpExtInst %float %1 Sqrt %float_2
%77 = OpFOrdGreaterThan %bool %76 %float_2
OpStore %c %77
%79 = OpLoad %bool %b
%80 = OpLoad %bool %c
%81 = OpLogicalNotEqual %bool %79 %80
OpStore %d %81
%83 = OpLoad %bool %b
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %bool %c
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %64 %86 %84
OpStore %e %87
%89 = OpLoad %bool %b
OpSelectionMerge %91 None
OpBranchConditional %89 %91 %90
%90 = OpLabel
%92 = OpLoad %bool %c
OpBranch %91
%91 = OpLabel
%93 = OpPhi %bool %true %85 %92 %90
OpStore %f %93
%94 = OpLoad %float %x
%96 = OpFAdd %float %94 %float_12
OpStore %x %96
%97 = OpLoad %float %x
%98 = OpFSub %float %97 %float_12
OpStore %x %98
%99 = OpLoad %float %x
%100 = OpLoad %float %y
%102 = OpFDiv %float %100 %float_10
OpStore %y %102
%103 = OpFMul %float %99 %102
OpStore %x %103
OpStore %x %float_6
%105 = OpLoad %bool %b
%106 = OpSelect %float %105 %float_1 %float_0
%108 = OpLoad %bool %c
%109 = OpSelect %float %108 %float_1 %float_0
%110 = OpFMul %float %106 %109
%111 = OpLoad %bool %d
%112 = OpSelect %float %111 %float_1 %float_0
%113 = OpFMul %float %110 %112
%114 = OpLoad %bool %e
%115 = OpSelect %float %114 %float_1 %float_0
%116 = OpFMul %float %113 %115
%117 = OpLoad %bool %f
%118 = OpSelect %float %117 %float_1 %float_0
%119 = OpFMul %float %116 %118
OpStore %y %119
OpStore %y %float_6
%120 = OpLoad %int %z
%122 = OpISub %int %120 %int_1
OpStore %z %122
OpStore %z %int_6
%124 = OpLoad %float %x
%125 = OpFOrdEqual %bool %124 %float_6
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %float %y
%129 = OpFOrdEqual %bool %128 %float_6
OpBranch %127
%127 = OpLabel
%130 = OpPhi %bool %false %91 %129 %126
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %int %z
%134 = OpIEqual %bool %133 %int_6
OpBranch %132
%132 = OpLabel
%135 = OpPhi %bool %false %127 %134 %131
OpSelectionMerge %140 None
OpBranchConditional %135 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%144 = OpLoad %v4float %141
OpStore %136 %144
OpBranch %140
%139 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %145
OpStore %136 %146
OpBranch %140
%140 = OpLabel
%147 = OpLoad %v4float %136
OpReturnValue %147
OpFunctionEnd
