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
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %inside_infinite_do_loop_b "inside_infinite_do_loop_b"
OpName %inside_infinite_while_loop_b "inside_infinite_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %main "main"
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
OpDecorate %15 Binding 0
OpDecorate %15 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_123 = OpConstant %float 123
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%68 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%25 = OpVariable %_ptr_Function_v2float Function
OpStore %25 %24
%27 = OpFunctionCall %v4float %main %25
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %28
%29 = OpLabel
OpBranch %30
%30 = OpLabel
OpLoopMerge %34 %33 None
OpBranch %31
%31 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_float %15 %int_2
%39 = OpLoad %float %35
%41 = OpFOrdEqual %bool %39 %float_123
OpBranchConditional %41 %32 %34
%32 = OpLabel
OpReturnValue %false
%33 = OpLabel
OpBranch %30
%34 = OpLabel
OpReturnValue %true
OpFunctionEnd
%inside_infinite_do_loop_b = OpFunction %bool None %28
%44 = OpLabel
OpBranch %45
%45 = OpLabel
OpLoopMerge %49 %48 None
OpBranch %46
%46 = OpLabel
OpReturnValue %true
%47 = OpLabel
OpBranch %48
%48 = OpLabel
OpBranchConditional %true %45 %49
%49 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_infinite_while_loop_b = OpFunction %bool None %28
%50 = OpLabel
OpBranch %51
%51 = OpLabel
OpLoopMerge %55 %54 None
OpBranch %52
%52 = OpLabel
OpBranchConditional %true %53 %55
%53 = OpLabel
OpReturnValue %true
%54 = OpLabel
OpBranch %51
%55 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %28
%56 = OpLabel
OpBranch %57
%57 = OpLabel
OpLoopMerge %61 %60 None
OpBranch %58
%58 = OpLabel
OpBranch %61
%59 = OpLabel
OpBranch %60
%60 = OpLabel
OpBranchConditional %true %57 %61
%61 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %28
%62 = OpLabel
OpBranch %63
%63 = OpLabel
OpLoopMerge %67 %66 None
OpBranch %64
%64 = OpLabel
OpBranchConditional %true %65 %67
%65 = OpLabel
OpBranch %67
%66 = OpLabel
OpBranch %63
%67 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %68
%69 = OpFunctionParameter %_ptr_Function_v2float
%70 = OpLabel
%88 = OpVariable %_ptr_Function_v4float Function
%71 = OpFunctionCall %bool %inside_while_loop_b
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpFunctionCall %bool %inside_infinite_do_loop_b
OpBranch %73
%73 = OpLabel
%75 = OpPhi %bool %false %70 %74 %72
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpFunctionCall %bool %inside_infinite_while_loop_b
OpBranch %77
%77 = OpLabel
%79 = OpPhi %bool %false %73 %78 %76
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpFunctionCall %bool %after_do_loop_b
OpBranch %81
%81 = OpLabel
%83 = OpPhi %bool %false %77 %82 %80
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpFunctionCall %bool %after_while_loop_b
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %81 %86 %84
OpSelectionMerge %92 None
OpBranchConditional %87 %90 %91
%90 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%96 = OpLoad %v4float %93
OpStore %88 %96
OpBranch %92
%91 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
%99 = OpLoad %v4float %97
OpStore %88 %99
OpBranch %92
%92 = OpLabel
%100 = OpLoad %v4float %88
OpReturnValue %100
OpFunctionEnd
