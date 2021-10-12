OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %glob "glob"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %block_variable_hides_global_variable_b "block_variable_hides_global_variable_b"
OpName %local_variable_hides_struct_b "local_variable_hides_struct_b"
OpName %S "S"
OpName %local_struct_variable_hides_struct_type_b "local_struct_variable_hides_struct_type_b"
OpName %S_0 "S"
OpMemberName %S_0 0 "i"
OpName %S_1 "S"
OpName %local_variable_hides_global_variable_b "local_variable_hides_global_variable_b"
OpName %glob_0 "glob"
OpName %main "main"
OpName %_0_var "_0_var"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %17 Binding 0
OpDecorate %17 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpMemberDecorate %S_0 0 Offset 0
OpDecorate %60 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%glob = OpVariable %_ptr_Private_int Private
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%22 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%30 = OpTypeFunction %bool
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%S_0 = OpTypeStruct %int
%_ptr_Function_S_0 = OpTypePointer Function %S_0
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%55 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%27 = OpVariable %_ptr_Function_v2float Function
OpStore %27 %26
%29 = OpFunctionCall %v4float %main %27
OpStore %sk_FragColor %29
OpReturn
OpFunctionEnd
%block_variable_hides_global_variable_b = OpFunction %bool None %30
%31 = OpLabel
%32 = OpLoad %int %glob
%34 = OpIEqual %bool %32 %int_2
OpReturnValue %34
OpFunctionEnd
%local_variable_hides_struct_b = OpFunction %bool None %30
%35 = OpLabel
%S = OpVariable %_ptr_Function_bool Function
OpStore %S %true
%39 = OpLoad %bool %S
OpReturnValue %39
OpFunctionEnd
%local_struct_variable_hides_struct_type_b = OpFunction %bool None %30
%40 = OpLabel
%S_1 = OpVariable %_ptr_Function_S_0 Function
%45 = OpCompositeConstruct %S_0 %int_1
OpStore %S_1 %45
%47 = OpAccessChain %_ptr_Function_int %S_1 %int_0
%49 = OpLoad %int %47
%50 = OpIEqual %bool %49 %int_1
OpReturnValue %50
OpFunctionEnd
%local_variable_hides_global_variable_b = OpFunction %bool None %30
%51 = OpLabel
%glob_0 = OpVariable %_ptr_Function_int Function
OpStore %glob_0 %int_1
%53 = OpLoad %int %glob_0
%54 = OpIEqual %bool %53 %int_1
OpReturnValue %54
OpFunctionEnd
%main = OpFunction %v4float None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
%_0_var = OpVariable %_ptr_Function_bool Function
%77 = OpVariable %_ptr_Function_v4float Function
OpStore %glob %int_2
OpStore %_0_var %true
%60 = OpLoad %bool %_0_var
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%63 = OpFunctionCall %bool %block_variable_hides_global_variable_b
OpBranch %62
%62 = OpLabel
%64 = OpPhi %bool %false %57 %63 %61
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpFunctionCall %bool %local_variable_hides_struct_b
OpBranch %66
%66 = OpLabel
%68 = OpPhi %bool %false %62 %67 %65
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpFunctionCall %bool %local_struct_variable_hides_struct_type_b
OpBranch %70
%70 = OpLabel
%72 = OpPhi %bool %false %66 %71 %69
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpFunctionCall %bool %local_variable_hides_global_variable_b
OpBranch %74
%74 = OpLabel
%76 = OpPhi %bool %false %70 %75 %73
OpSelectionMerge %81 None
OpBranchConditional %76 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%84 = OpLoad %v4float %82
OpStore %77 %84
OpBranch %81
%80 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
%86 = OpLoad %v4float %85
OpStore %77 %86
OpBranch %81
%81 = OpLabel
%87 = OpLoad %v4float %77
OpReturnValue %87
OpFunctionEnd
