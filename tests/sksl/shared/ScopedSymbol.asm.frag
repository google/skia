OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %glob "glob"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %block_variable_hides_local_variable_b "block_variable_hides_local_variable_b"
OpName %var "var"
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
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpMemberDecorate %S_0 0 Offset 0
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%glob = OpVariable %_ptr_Private_int Private
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%23 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%31 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int_2 = OpConstant %int 2
%S_0 = OpTypeStruct %int
%_ptr_Function_S_0 = OpTypePointer Function %S_0
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%55 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %23
%24 = OpLabel
%28 = OpVariable %_ptr_Function_v2float Function
OpStore %28 %27
%30 = OpFunctionCall %v4float %main %28
OpStore %sk_FragColor %30
OpReturn
OpFunctionEnd
%block_variable_hides_local_variable_b = OpFunction %bool None %31
%32 = OpLabel
%var = OpVariable %_ptr_Function_bool Function
OpStore %var %true
OpReturnValue %true
OpFunctionEnd
%block_variable_hides_global_variable_b = OpFunction %bool None %31
%36 = OpLabel
%37 = OpLoad %int %glob
%39 = OpIEqual %bool %37 %int_2
OpReturnValue %39
OpFunctionEnd
%local_variable_hides_struct_b = OpFunction %bool None %31
%40 = OpLabel
%S = OpVariable %_ptr_Function_bool Function
OpStore %S %true
OpReturnValue %true
OpFunctionEnd
%local_struct_variable_hides_struct_type_b = OpFunction %bool None %31
%42 = OpLabel
%S_1 = OpVariable %_ptr_Function_S_0 Function
%47 = OpCompositeConstruct %S_0 %int_1
OpStore %S_1 %47
%49 = OpAccessChain %_ptr_Function_int %S_1 %int_0
%51 = OpLoad %int %49
%52 = OpIEqual %bool %51 %int_1
OpReturnValue %52
OpFunctionEnd
%local_variable_hides_global_variable_b = OpFunction %bool None %31
%53 = OpLabel
%glob_0 = OpVariable %_ptr_Function_int Function
OpStore %glob_0 %int_1
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
%76 = OpVariable %_ptr_Function_v4float Function
OpStore %glob %int_2
%59 = OpFunctionCall %bool %block_variable_hides_local_variable_b
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpFunctionCall %bool %block_variable_hides_global_variable_b
OpBranch %61
%61 = OpLabel
%63 = OpPhi %bool %false %57 %62 %60
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpFunctionCall %bool %local_variable_hides_struct_b
OpBranch %65
%65 = OpLabel
%67 = OpPhi %bool %false %61 %66 %64
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpFunctionCall %bool %local_struct_variable_hides_struct_type_b
OpBranch %69
%69 = OpLabel
%71 = OpPhi %bool %false %65 %70 %68
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpFunctionCall %bool %local_variable_hides_global_variable_b
OpBranch %73
%73 = OpLabel
%75 = OpPhi %bool %false %69 %74 %72
OpSelectionMerge %80 None
OpBranchConditional %75 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%83 = OpLoad %v4float %81
OpStore %76 %83
OpBranch %80
%79 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%85 = OpLoad %v4float %84
OpStore %76 %85
OpBranch %80
%80 = OpLabel
%86 = OpLoad %v4float %76
OpReturnValue %86
OpFunctionEnd
