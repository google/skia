OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %scratchVar "scratchVar"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_flat_b "test_flat_b"
OpName %test_if_b "test_if_b"
OpName %test_else_b "test_else_b"
OpName %test_loop_return_b "test_loop_return_b"
OpName %x "x"
OpName %test_loop_break_b "test_loop_break_b"
OpName %x_0 "x"
OpName %test_loop_if_b "test_loop_if_b"
OpName %x_1 "x"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %20 Binding 0
OpDecorate %20 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%scratchVar = OpVariable %_ptr_Private_int Private
%int_0 = OpConstant %int 0
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%25 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
%106 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %25
%26 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_flat_b = OpFunction %bool None %33
%34 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_b = OpFunction %bool None %33
%36 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%39 = OpLoad %v4float %37
%40 = OpCompositeExtract %float %39 1
%41 = OpFOrdGreaterThan %bool %40 %float_0
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
OpReturnValue %true
%43 = OpLabel
%46 = OpLoad %int %scratchVar
%47 = OpIAdd %int %46 %int_1
OpStore %scratchVar %47
OpBranch %44
%44 = OpLabel
%48 = OpLoad %int %scratchVar
%49 = OpIAdd %int %48 %int_1
OpStore %scratchVar %49
OpReturnValue %false
OpFunctionEnd
%test_else_b = OpFunction %bool None %33
%51 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%53 = OpLoad %v4float %52
%54 = OpCompositeExtract %float %53 1
%55 = OpFOrdEqual %bool %54 %float_0
OpSelectionMerge %58 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
OpReturnValue %false
%57 = OpLabel
OpReturnValue %true
%58 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %33
%59 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %62
%62 = OpLabel
OpLoopMerge %66 %65 None
OpBranch %63
%63 = OpLabel
%67 = OpLoad %int %x
%68 = OpSLessThan %bool %67 %int_0
OpBranchConditional %68 %64 %66
%64 = OpLabel
OpReturnValue %false
%65 = OpLabel
%69 = OpLoad %int %x
%70 = OpIAdd %int %69 %int_1
OpStore %x %70
OpBranch %62
%66 = OpLabel
%71 = OpLoad %int %scratchVar
%72 = OpIAdd %int %71 %int_1
OpStore %scratchVar %72
OpReturnValue %true
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %33
%73 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %75
%75 = OpLabel
OpLoopMerge %79 %78 None
OpBranch %76
%76 = OpLabel
%80 = OpLoad %int %x_0
%81 = OpSLessThanEqual %bool %80 %int_1
OpBranchConditional %81 %77 %79
%77 = OpLabel
OpBranch %79
%78 = OpLabel
%82 = OpLoad %int %x_0
%83 = OpIAdd %int %82 %int_1
OpStore %x_0 %83
OpBranch %75
%79 = OpLabel
%84 = OpLoad %int %scratchVar
%85 = OpIAdd %int %84 %int_1
OpStore %scratchVar %85
OpReturnValue %true
OpFunctionEnd
%test_loop_if_b = OpFunction %bool None %33
%86 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %88
%88 = OpLabel
OpLoopMerge %92 %91 None
OpBranch %89
%89 = OpLabel
%93 = OpLoad %int %x_1
%94 = OpSLessThanEqual %bool %93 %int_1
OpBranchConditional %94 %90 %92
%90 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%96 = OpLoad %v4float %95
%97 = OpCompositeExtract %float %96 1
%98 = OpFOrdEqual %bool %97 %float_0
OpSelectionMerge %101 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
OpReturnValue %false
%100 = OpLabel
OpReturnValue %true
%101 = OpLabel
OpBranch %91
%91 = OpLabel
%102 = OpLoad %int %x_1
%103 = OpIAdd %int %102 %int_1
OpStore %x_1 %103
OpBranch %88
%92 = OpLabel
%104 = OpLoad %int %scratchVar
%105 = OpIAdd %int %104 %int_1
OpStore %scratchVar %105
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %106
%107 = OpFunctionParameter %_ptr_Function_v2float
%108 = OpLabel
%130 = OpVariable %_ptr_Function_v4float Function
OpStore %scratchVar %int_0
%109 = OpFunctionCall %bool %test_flat_b
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpFunctionCall %bool %test_if_b
OpBranch %111
%111 = OpLabel
%113 = OpPhi %bool %false %108 %112 %110
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpFunctionCall %bool %test_else_b
OpBranch %115
%115 = OpLabel
%117 = OpPhi %bool %false %111 %116 %114
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpFunctionCall %bool %test_loop_return_b
OpBranch %119
%119 = OpLabel
%121 = OpPhi %bool %false %115 %120 %118
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpFunctionCall %bool %test_loop_break_b
OpBranch %123
%123 = OpLabel
%125 = OpPhi %bool %false %119 %124 %122
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpFunctionCall %bool %test_loop_if_b
OpBranch %127
%127 = OpLabel
%129 = OpPhi %bool %false %123 %128 %126
OpSelectionMerge %134 None
OpBranchConditional %129 %132 %133
%132 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%136 = OpLoad %v4float %135
OpStore %130 %136
OpBranch %134
%133 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%138 = OpLoad %v4float %137
OpStore %130 %138
OpBranch %134
%134 = OpLabel
%139 = OpLoad %v4float %130
OpReturnValue %139
OpFunctionEnd
