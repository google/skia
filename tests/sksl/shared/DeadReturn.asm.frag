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
OpName %test_loop_if_b "test_loop_if_b"
OpName %x "x"
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
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
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
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%23 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%31 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
%78 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %23
%24 = OpLabel
%28 = OpVariable %_ptr_Function_v2float Function
OpStore %28 %27
%30 = OpFunctionCall %v4float %main %28
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
%test_flat_b = OpFunction %bool None %31
%32 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_b = OpFunction %bool None %31
%34 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%37 = OpLoad %v4float %35
%38 = OpCompositeExtract %float %37 1
%39 = OpFOrdGreaterThan %bool %38 %float_0
OpSelectionMerge %42 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
OpReturnValue %true
%41 = OpLabel
%44 = OpLoad %int %scratchVar
%45 = OpIAdd %int %44 %int_1
OpStore %scratchVar %45
OpBranch %42
%42 = OpLabel
%46 = OpLoad %int %scratchVar
%47 = OpIAdd %int %46 %int_1
OpStore %scratchVar %47
OpReturnValue %false
OpFunctionEnd
%test_else_b = OpFunction %bool None %31
%49 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 1
%53 = OpFOrdEqual %bool %52 %float_0
OpSelectionMerge %56 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
OpReturnValue %false
%55 = OpLabel
OpReturnValue %true
%56 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_if_b = OpFunction %bool None %31
%57 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %60
%60 = OpLabel
OpLoopMerge %64 %63 None
OpBranch %61
%61 = OpLabel
%65 = OpLoad %int %x
%66 = OpSLessThanEqual %bool %65 %int_1
OpBranchConditional %66 %62 %64
%62 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 1
%70 = OpFOrdEqual %bool %69 %float_0
OpSelectionMerge %73 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
OpReturnValue %false
%72 = OpLabel
OpReturnValue %true
%73 = OpLabel
OpBranch %63
%63 = OpLabel
%74 = OpLoad %int %x
%75 = OpIAdd %int %74 %int_1
OpStore %x %75
OpBranch %60
%64 = OpLabel
%76 = OpLoad %int %scratchVar
%77 = OpIAdd %int %76 %int_1
OpStore %scratchVar %77
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %78
%79 = OpFunctionParameter %_ptr_Function_v2float
%80 = OpLabel
%94 = OpVariable %_ptr_Function_v4float Function
OpStore %scratchVar %int_0
%81 = OpFunctionCall %bool %test_flat_b
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpFunctionCall %bool %test_if_b
OpBranch %83
%83 = OpLabel
%85 = OpPhi %bool %false %80 %84 %82
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpFunctionCall %bool %test_else_b
OpBranch %87
%87 = OpLabel
%89 = OpPhi %bool %false %83 %88 %86
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpFunctionCall %bool %test_loop_if_b
OpBranch %91
%91 = OpLabel
%93 = OpPhi %bool %false %87 %92 %90
OpSelectionMerge %98 None
OpBranchConditional %93 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%100 = OpLoad %v4float %99
OpStore %94 %100
OpBranch %98
%97 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%102 = OpLoad %v4float %101
OpStore %94 %102
OpBranch %98
%98 = OpLabel
%103 = OpLoad %v4float %94
OpReturnValue %103
OpFunctionEnd
