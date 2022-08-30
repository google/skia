OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %non_constant_swizzle_h4 "non_constant_swizzle_h4"
OpName %v "v"
OpName %i "i"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %v RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %x RelaxedPrecision
OpDecorate %y RelaxedPrecision
OpDecorate %z RelaxedPrecision
OpDecorate %w RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%62 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_n1_25 = OpConstant %float -1.25
%67 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%non_constant_swizzle_h4 = OpFunction %v4float None %24
%25 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%32 = OpLoad %v4float %28
OpStore %v %32
%36 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %38 1
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %38 2
%44 = OpConvertFToS %int %43
%45 = OpCompositeExtract %float %38 3
%46 = OpConvertFToS %int %45
%47 = OpCompositeConstruct %v4int %40 %42 %44 %46
OpStore %i %47
%50 = OpCompositeExtract %int %47 0
%51 = OpVectorExtractDynamic %float %32 %50
OpStore %x %51
%53 = OpCompositeExtract %int %47 1
%54 = OpVectorExtractDynamic %float %32 %53
OpStore %y %54
%56 = OpCompositeExtract %int %47 2
%57 = OpVectorExtractDynamic %float %32 %56
OpStore %z %57
%59 = OpCompositeExtract %int %47 3
%60 = OpVectorExtractDynamic %float %32 %59
OpStore %w %60
%61 = OpCompositeConstruct %v4float %51 %54 %57 %60
OpReturnValue %61
OpFunctionEnd
%main = OpFunction %v4float None %62
%63 = OpFunctionParameter %_ptr_Function_v2float
%64 = OpLabel
%71 = OpVariable %_ptr_Function_v4float Function
%65 = OpFunctionCall %v4float %non_constant_swizzle_h4
%68 = OpFOrdEqual %v4bool %65 %67
%70 = OpAll %bool %68
OpSelectionMerge %74 None
OpBranchConditional %70 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%77 = OpLoad %v4float %75
OpStore %71 %77
OpBranch %74
%73 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%80 = OpLoad %v4float %78
OpStore %71 %80
OpBranch %74
%74 = OpLabel
%81 = OpLoad %v4float %71
OpReturnValue %81
OpFunctionEnd
