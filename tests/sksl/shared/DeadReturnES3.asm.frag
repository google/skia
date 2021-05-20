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
OpName %test_return_b "test_return_b"
OpName %test_break_b "test_break_b"
OpName %test_continue_b "test_continue_b"
OpName %test_if_return_b "test_if_return_b"
OpName %test_if_break_b "test_if_break_b"
OpName %test_else_b "test_else_b"
OpName %test_loop_return_b "test_loop_return_b"
OpName %x "x"
OpName %test_loop_break_b "test_loop_break_b"
OpName %x_0 "x"
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
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%23 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%31 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%118 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %23
%24 = OpLabel
%28 = OpVariable %_ptr_Function_v2float Function
OpStore %28 %27
%30 = OpFunctionCall %v4float %main %28
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
%test_return_b = OpFunction %bool None %31
%32 = OpLabel
OpBranch %33
%33 = OpLabel
OpLoopMerge %37 %36 None
OpBranch %34
%34 = OpLabel
OpReturnValue %true
%35 = OpLabel
OpBranch %36
%36 = OpLabel
OpBranchConditional %false %33 %37
%37 = OpLabel
OpUnreachable
OpFunctionEnd
%test_break_b = OpFunction %bool None %31
%40 = OpLabel
OpBranch %41
%41 = OpLabel
OpLoopMerge %45 %44 None
OpBranch %42
%42 = OpLabel
OpBranch %45
%43 = OpLabel
OpBranch %44
%44 = OpLabel
OpBranchConditional %false %41 %45
%45 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_continue_b = OpFunction %bool None %31
%46 = OpLabel
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
OpBranch %50
%49 = OpLabel
OpBranch %50
%50 = OpLabel
OpBranchConditional %false %47 %51
%51 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_return_b = OpFunction %bool None %31
%52 = OpLabel
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%62 = OpLoad %v4float %58
%63 = OpCompositeExtract %float %62 1
%64 = OpFOrdGreaterThan %bool %63 %float_0
OpSelectionMerge %67 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
OpReturnValue %true
%66 = OpLabel
OpBranch %57
%67 = OpLabel
OpBranch %56
%55 = OpLabel
OpBranch %56
%56 = OpLabel
OpBranchConditional %false %53 %57
%57 = OpLabel
OpReturnValue %false
OpFunctionEnd
%test_if_break_b = OpFunction %bool None %31
%68 = OpLabel
OpBranch %69
%69 = OpLabel
OpLoopMerge %73 %72 None
OpBranch %70
%70 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%75 = OpLoad %v4float %74
%76 = OpCompositeExtract %float %75 1
%77 = OpFOrdGreaterThan %bool %76 %float_0
OpSelectionMerge %80 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
OpBranch %73
%79 = OpLabel
OpBranch %72
%80 = OpLabel
OpBranch %71
%71 = OpLabel
OpBranch %72
%72 = OpLabel
OpBranchConditional %false %69 %73
%73 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_else_b = OpFunction %bool None %31
%81 = OpLabel
OpBranch %82
%82 = OpLabel
OpLoopMerge %86 %85 None
OpBranch %83
%83 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%88 = OpLoad %v4float %87
%89 = OpCompositeExtract %float %88 1
%90 = OpFOrdEqual %bool %89 %float_0
OpSelectionMerge %93 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
OpReturnValue %false
%92 = OpLabel
OpReturnValue %true
%93 = OpLabel
OpBranch %84
%84 = OpLabel
OpBranch %85
%85 = OpLabel
OpBranchConditional %false %82 %86
%86 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %31
%94 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %97
%97 = OpLabel
OpLoopMerge %101 %100 None
OpBranch %98
%98 = OpLabel
%102 = OpLoad %int %x
%103 = OpSLessThan %bool %102 %int_0
OpBranchConditional %103 %99 %101
%99 = OpLabel
OpReturnValue %false
%100 = OpLabel
%105 = OpLoad %int %x
%106 = OpIAdd %int %105 %int_1
OpStore %x %106
OpBranch %97
%101 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %31
%107 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %109
%109 = OpLabel
OpLoopMerge %113 %112 None
OpBranch %110
%110 = OpLabel
%114 = OpLoad %int %x_0
%115 = OpSLessThanEqual %bool %114 %int_1
OpBranchConditional %115 %111 %113
%111 = OpLabel
OpBranch %113
%112 = OpLabel
%116 = OpLoad %int %x_0
%117 = OpIAdd %int %116 %int_1
OpStore %x_0 %117
OpBranch %109
%113 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %118
%119 = OpFunctionParameter %_ptr_Function_v2float
%120 = OpLabel
%150 = OpVariable %_ptr_Function_v4float Function
%121 = OpFunctionCall %bool %test_return_b
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpFunctionCall %bool %test_break_b
OpBranch %123
%123 = OpLabel
%125 = OpPhi %bool %false %120 %124 %122
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpFunctionCall %bool %test_continue_b
OpBranch %127
%127 = OpLabel
%129 = OpPhi %bool %false %123 %128 %126
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpFunctionCall %bool %test_if_return_b
OpBranch %131
%131 = OpLabel
%133 = OpPhi %bool %false %127 %132 %130
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpFunctionCall %bool %test_if_break_b
OpBranch %135
%135 = OpLabel
%137 = OpPhi %bool %false %131 %136 %134
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpFunctionCall %bool %test_else_b
OpBranch %139
%139 = OpLabel
%141 = OpPhi %bool %false %135 %140 %138
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpFunctionCall %bool %test_loop_return_b
OpBranch %143
%143 = OpLabel
%145 = OpPhi %bool %false %139 %144 %142
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpFunctionCall %bool %test_loop_break_b
OpBranch %147
%147 = OpLabel
%149 = OpPhi %bool %false %143 %148 %146
OpSelectionMerge %154 None
OpBranchConditional %149 %152 %153
%152 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%156 = OpLoad %v4float %155
OpStore %150 %156
OpBranch %154
%153 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%158 = OpLoad %v4float %157
OpStore %150 %158
OpBranch %154
%154 = OpLabel
%159 = OpLoad %v4float %150
OpReturnValue %159
OpFunctionEnd
