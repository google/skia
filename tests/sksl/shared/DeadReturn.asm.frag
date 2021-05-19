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
OpDecorate %19 Binding 0
OpDecorate %19 DescriptorSet 0
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
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
%19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%24 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%32 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
%83 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %24
%25 = OpLabel
%29 = OpVariable %_ptr_Function_v2float Function
OpStore %29 %28
%31 = OpFunctionCall %v4float %main %29
OpStore %sk_FragColor %31
OpReturn
OpFunctionEnd
%test_flat_b = OpFunction %bool None %32
%33 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_b = OpFunction %bool None %32
%35 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 1
%40 = OpFOrdGreaterThan %bool %39 %float_0
OpSelectionMerge %43 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
OpReturnValue %true
%42 = OpLabel
%45 = OpLoad %int %scratchVar
%46 = OpIAdd %int %45 %int_1
OpStore %scratchVar %46
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %scratchVar
%48 = OpIAdd %int %47 %int_1
OpStore %scratchVar %48
OpReturnValue %false
OpFunctionEnd
%test_else_b = OpFunction %bool None %32
%50 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 1
%54 = OpFOrdEqual %bool %53 %float_0
OpSelectionMerge %57 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
OpReturnValue %false
%56 = OpLabel
OpReturnValue %true
%57 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %32
%58 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %61
%61 = OpLabel
OpLoopMerge %65 %64 None
OpBranch %62
%62 = OpLabel
%66 = OpLoad %int %x
%67 = OpSLessThanEqual %bool %66 %int_1
OpBranchConditional %67 %63 %65
%63 = OpLabel
OpReturnValue %true
%64 = OpLabel
%68 = OpLoad %int %x
%69 = OpIAdd %int %68 %int_1
OpStore %x %69
OpBranch %61
%65 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %32
%70 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %72
%72 = OpLabel
OpLoopMerge %76 %75 None
OpBranch %73
%73 = OpLabel
%77 = OpLoad %int %x_0
%78 = OpSLessThanEqual %bool %77 %int_1
OpBranchConditional %78 %74 %76
%74 = OpLabel
OpBranch %76
%75 = OpLabel
%79 = OpLoad %int %x_0
%80 = OpIAdd %int %79 %int_1
OpStore %x_0 %80
OpBranch %72
%76 = OpLabel
%81 = OpLoad %int %scratchVar
%82 = OpIAdd %int %81 %int_1
OpStore %scratchVar %82
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %83
%84 = OpFunctionParameter %_ptr_Function_v2float
%85 = OpLabel
%103 = OpVariable %_ptr_Function_v4float Function
OpStore %scratchVar %int_0
%86 = OpFunctionCall %bool %test_flat_b
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpFunctionCall %bool %test_if_b
OpBranch %88
%88 = OpLabel
%90 = OpPhi %bool %false %85 %89 %87
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpFunctionCall %bool %test_else_b
OpBranch %92
%92 = OpLabel
%94 = OpPhi %bool %false %88 %93 %91
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpFunctionCall %bool %test_loop_return_b
OpBranch %96
%96 = OpLabel
%98 = OpPhi %bool %false %92 %97 %95
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpFunctionCall %bool %test_loop_break_b
OpBranch %100
%100 = OpLabel
%102 = OpPhi %bool %false %96 %101 %99
OpSelectionMerge %107 None
OpBranchConditional %102 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%109 = OpLoad %v4float %108
OpStore %103 %109
OpBranch %107
%106 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%111 = OpLoad %v4float %110
OpStore %103 %111
OpBranch %107
%107 = OpLabel
%112 = OpLoad %v4float %103
OpReturnValue %112
OpFunctionEnd
