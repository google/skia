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
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
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
%85 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%67 = OpSLessThan %bool %66 %int_0
OpBranchConditional %67 %63 %65
%63 = OpLabel
OpReturnValue %false
%64 = OpLabel
%68 = OpLoad %int %x
%69 = OpIAdd %int %68 %int_1
OpStore %x %69
OpBranch %61
%65 = OpLabel
%70 = OpLoad %int %scratchVar
%71 = OpIAdd %int %70 %int_1
OpStore %scratchVar %71
OpReturnValue %true
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %32
%72 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %74
%74 = OpLabel
OpLoopMerge %78 %77 None
OpBranch %75
%75 = OpLabel
%79 = OpLoad %int %x_0
%80 = OpSLessThanEqual %bool %79 %int_1
OpBranchConditional %80 %76 %78
%76 = OpLabel
OpBranch %78
%77 = OpLabel
%81 = OpLoad %int %x_0
%82 = OpIAdd %int %81 %int_1
OpStore %x_0 %82
OpBranch %74
%78 = OpLabel
%83 = OpLoad %int %scratchVar
%84 = OpIAdd %int %83 %int_1
OpStore %scratchVar %84
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %85
%86 = OpFunctionParameter %_ptr_Function_v2float
%87 = OpLabel
%105 = OpVariable %_ptr_Function_v4float Function
OpStore %scratchVar %int_0
%88 = OpFunctionCall %bool %test_flat_b
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpFunctionCall %bool %test_if_b
OpBranch %90
%90 = OpLabel
%92 = OpPhi %bool %false %87 %91 %89
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpFunctionCall %bool %test_else_b
OpBranch %94
%94 = OpLabel
%96 = OpPhi %bool %false %90 %95 %93
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpFunctionCall %bool %test_loop_return_b
OpBranch %98
%98 = OpLabel
%100 = OpPhi %bool %false %94 %99 %97
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpFunctionCall %bool %test_loop_break_b
OpBranch %102
%102 = OpLabel
%104 = OpPhi %bool %false %98 %103 %101
OpSelectionMerge %109 None
OpBranchConditional %104 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %19 %int_0
%111 = OpLoad %v4float %110
OpStore %105 %111
OpBranch %109
%108 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %19 %int_1
%113 = OpLoad %v4float %112
OpStore %105 %113
OpBranch %109
%109 = OpLabel
%114 = OpLoad %v4float %105
OpReturnValue %114
OpFunctionEnd
