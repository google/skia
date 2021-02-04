OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %test_int "test_int"
OpName %result "result"
OpName %main "main"
OpName %_1_result "_1_result"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%46 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_0 = OpConstant %float 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%test_int = OpFunction %bool None %19
%20 = OpLabel
%result = OpVariable %_ptr_Function_v4int Function
%26 = OpAccessChain %_ptr_Function_int %result %int_0
OpStore %26 %int_1
%29 = OpAccessChain %_ptr_Function_int %result %int_1
OpStore %29 %int_1
%30 = OpAccessChain %_ptr_Function_int %result %int_2
OpStore %30 %int_1
%32 = OpAccessChain %_ptr_Function_int %result %int_3
OpStore %32 %int_1
%34 = OpLoad %v4int %result
%35 = OpCompositeExtract %int %34 0
%36 = OpLoad %v4int %result
%37 = OpCompositeExtract %int %36 1
%38 = OpIMul %int %35 %37
%39 = OpLoad %v4int %result
%40 = OpCompositeExtract %int %39 2
%41 = OpIMul %int %38 %40
%42 = OpLoad %v4int %result
%43 = OpCompositeExtract %int %42 3
%44 = OpIMul %int %41 %43
%45 = OpINotEqual %bool %44 %int_0
OpReturnValue %45
OpFunctionEnd
%main = OpFunction %v4float None %46
%47 = OpLabel
%_1_result = OpVariable %_ptr_Function_v4float Function
%74 = OpVariable %_ptr_Function_v4float Function
%51 = OpAccessChain %_ptr_Function_float %_1_result %int_0
OpStore %51 %float_1
%53 = OpAccessChain %_ptr_Function_float %_1_result %int_1
OpStore %53 %float_1
%54 = OpAccessChain %_ptr_Function_float %_1_result %int_2
OpStore %54 %float_1
%55 = OpAccessChain %_ptr_Function_float %_1_result %int_3
OpStore %55 %float_1
%57 = OpLoad %v4float %_1_result
%58 = OpCompositeExtract %float %57 0
%59 = OpLoad %v4float %_1_result
%60 = OpCompositeExtract %float %59 1
%61 = OpFMul %float %58 %60
%62 = OpLoad %v4float %_1_result
%63 = OpCompositeExtract %float %62 2
%64 = OpFMul %float %61 %63
%65 = OpLoad %v4float %_1_result
%66 = OpCompositeExtract %float %65 3
%67 = OpFMul %float %64 %66
%68 = OpFUnordNotEqual %bool %67 %float_0
OpSelectionMerge %71 None
OpBranchConditional %68 %70 %71
%70 = OpLabel
%72 = OpFunctionCall %bool %test_int
OpBranch %71
%71 = OpLabel
%73 = OpPhi %bool %false %47 %72 %70
OpSelectionMerge %77 None
OpBranchConditional %73 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%80 = OpLoad %v4float %78
OpStore %74 %80
OpBranch %77
%76 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%82 = OpLoad %v4float %81
OpStore %74 %82
OpBranch %77
%77 = OpLabel
%83 = OpLoad %v4float %74
OpReturnValue %83
OpFunctionEnd
