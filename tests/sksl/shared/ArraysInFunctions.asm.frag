### Compilation failed:

error: SPIR-V validation error: Pointer operand 64[%64] must be a memory object declaration
  %65 = OpFunctionCall %void %modifies_an_array %64

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpName %_entrypoint "_entrypoint"
OpName %S "S"
OpMemberName %S 0 "a"
OpName %returns_an_array_in_a_struct "returns_an_array_in_a_struct"
OpName %s "s"
OpName %accepts_an_array "accepts_an_array"
OpName %modifies_an_array "modifies_an_array"
OpName %main "main"
OpName %s_0 "s"
OpName %x "x"
OpName %valid "valid"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %_arr_float_int_2 ArrayStride 16
OpMemberDecorate %S 0 Offset 0
OpDecorate %36 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%S = OpTypeStruct %_arr_float_int_2
%25 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_1 = OpConstant %int 1
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%37 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
%46 = OpTypeFunction %void %_ptr_Function__arr_float_int_2
%55 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %18
%19 = OpLabel
%20 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%returns_an_array_in_a_struct = OpFunction %S None %25
%26 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%31 = OpAccessChain %_ptr_Function_float %s %int_0 %int_0
OpStore %31 %float_1
%35 = OpAccessChain %_ptr_Function_float %s %int_0 %int_1
OpStore %35 %float_2
%36 = OpLoad %S %s
OpReturnValue %36
OpFunctionEnd
%accepts_an_array = OpFunction %float None %37
%39 = OpFunctionParameter %_ptr_Function__arr_float_int_2
%40 = OpLabel
%41 = OpAccessChain %_ptr_Function_float %39 %int_0
%42 = OpLoad %float %41
%43 = OpAccessChain %_ptr_Function_float %39 %int_1
%44 = OpLoad %float %43
%45 = OpFAdd %float %42 %44
OpReturnValue %45
OpFunctionEnd
%modifies_an_array = OpFunction %void None %46
%47 = OpFunctionParameter %_ptr_Function__arr_float_int_2
%48 = OpLabel
%49 = OpAccessChain %_ptr_Function_float %47 %int_0
%50 = OpLoad %float %49
%51 = OpFAdd %float %50 %float_1
OpStore %49 %51
%52 = OpAccessChain %_ptr_Function_float %47 %int_1
%53 = OpLoad %float %52
%54 = OpFAdd %float %53 %float_1
OpStore %52 %54
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %55
%56 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%62 = OpVariable %_ptr_Function__arr_float_int_2 Function
%valid = OpVariable %_ptr_Function_bool Function
%85 = OpVariable %_ptr_Function_v4float Function
%58 = OpFunctionCall %S %returns_an_array_in_a_struct
OpStore %s_0 %58
%60 = OpAccessChain %_ptr_Function__arr_float_int_2 %s_0 %int_0
%61 = OpLoad %_arr_float_int_2 %60
OpStore %62 %61
%63 = OpFunctionCall %float %accepts_an_array %62
OpStore %x %63
%64 = OpAccessChain %_ptr_Function__arr_float_int_2 %s_0 %int_0
%65 = OpFunctionCall %void %modifies_an_array %64
%69 = OpLoad %float %x
%71 = OpFOrdEqual %bool %69 %float_3
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpAccessChain %_ptr_Function_float %s_0 %int_0 %int_0
%75 = OpLoad %float %74
%76 = OpFOrdEqual %bool %75 %float_2
OpBranch %73
%73 = OpLabel
%77 = OpPhi %bool %false %56 %76 %72
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpAccessChain %_ptr_Function_float %s_0 %int_0 %int_1
%81 = OpLoad %float %80
%82 = OpFOrdEqual %bool %81 %float_3
OpBranch %79
%79 = OpLabel
%83 = OpPhi %bool %false %73 %82 %78
OpStore %valid %83
%84 = OpLoad %bool %valid
OpSelectionMerge %89 None
OpBranchConditional %84 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%92 = OpLoad %v4float %90
OpStore %85 %92
OpBranch %89
%88 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%94 = OpLoad %v4float %93
OpStore %85 %94
OpBranch %89
%89 = OpLabel
%95 = OpLoad %v4float %85
OpReturnValue %95
OpFunctionEnd

1 error
