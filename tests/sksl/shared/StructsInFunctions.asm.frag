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
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %returns_a_struct "returns_a_struct"
OpName %s "s"
OpName %accepts_a_struct "accepts_a_struct"
OpName %modifies_a_struct "modifies_a_struct"
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
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %35 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
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
%S = OpTypeStruct %float %int
%23 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%36 = OpTypeFunction %float %_ptr_Function_S
%45 = OpTypeFunction %void %_ptr_Function_S
%54 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %18
%19 = OpLabel
%20 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%returns_a_struct = OpFunction %S None %23
%24 = OpLabel
%s = OpVariable %_ptr_Function_S Function
%29 = OpAccessChain %_ptr_Function_float %s %int_0
OpStore %29 %float_1
%33 = OpAccessChain %_ptr_Function_int %s %int_1
OpStore %33 %int_2
%35 = OpLoad %S %s
OpReturnValue %35
OpFunctionEnd
%accepts_a_struct = OpFunction %float None %36
%37 = OpFunctionParameter %_ptr_Function_S
%38 = OpLabel
%39 = OpAccessChain %_ptr_Function_float %37 %int_0
%40 = OpLoad %float %39
%41 = OpAccessChain %_ptr_Function_int %37 %int_1
%42 = OpLoad %int %41
%43 = OpConvertSToF %float %42
%44 = OpFAdd %float %40 %43
OpReturnValue %44
OpFunctionEnd
%modifies_a_struct = OpFunction %void None %45
%46 = OpFunctionParameter %_ptr_Function_S
%47 = OpLabel
%48 = OpAccessChain %_ptr_Function_float %46 %int_0
%49 = OpLoad %float %48
%50 = OpFAdd %float %49 %float_1
OpStore %48 %50
%51 = OpAccessChain %_ptr_Function_int %46 %int_1
%52 = OpLoad %int %51
%53 = OpIAdd %int %52 %int_1
OpStore %51 %53
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpLabel
%s_0 = OpVariable %_ptr_Function_S Function
%x = OpVariable %_ptr_Function_float Function
%60 = OpVariable %_ptr_Function_S Function
%63 = OpVariable %_ptr_Function_S Function
%valid = OpVariable %_ptr_Function_bool Function
%87 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %S %returns_a_struct
OpStore %s_0 %57
%59 = OpLoad %S %s_0
OpStore %60 %59
%61 = OpFunctionCall %float %accepts_a_struct %60
OpStore %x %61
%62 = OpLoad %S %s_0
OpStore %63 %62
%64 = OpFunctionCall %void %modifies_a_struct %63
%65 = OpLoad %S %63
OpStore %s_0 %65
%69 = OpLoad %float %x
%71 = OpFOrdEqual %bool %69 %float_3
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpAccessChain %_ptr_Function_float %s_0 %int_0
%75 = OpLoad %float %74
%77 = OpFOrdEqual %bool %75 %float_2
OpBranch %73
%73 = OpLabel
%78 = OpPhi %bool %false %55 %77 %72
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpAccessChain %_ptr_Function_int %s_0 %int_1
%82 = OpLoad %int %81
%84 = OpIEqual %bool %82 %int_3
OpBranch %80
%80 = OpLabel
%85 = OpPhi %bool %false %73 %84 %79
OpStore %valid %85
%86 = OpLoad %bool %valid
OpSelectionMerge %91 None
OpBranchConditional %86 %89 %90
%89 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%94 = OpLoad %v4float %92
OpStore %87 %94
OpBranch %91
%90 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%96 = OpLoad %v4float %95
OpStore %87 %96
OpBranch %91
%91 = OpLabel
%97 = OpLoad %v4float %87
OpReturnValue %97
OpFunctionEnd
