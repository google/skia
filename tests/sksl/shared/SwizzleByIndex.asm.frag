OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %non_constant_swizzle "non_constant_swizzle"
OpName %v "v"
OpName %i "i"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
%float_0 = OpConstant %float 0
%73 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%non_constant_swizzle = OpFunction %v4float None %19
%20 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%i = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%27 = OpLoad %v4float %23
OpStore %v %27
%31 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%33 = OpLoad %v4float %31
%34 = OpCompositeExtract %float %33 0
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %33 1
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %33 2
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %33 3
%41 = OpConvertFToS %int %40
%42 = OpCompositeConstruct %v4int %35 %37 %39 %41
OpStore %i %42
%45 = OpLoad %v4float %v
%46 = OpLoad %v4int %i
%47 = OpCompositeExtract %int %46 0
%48 = OpVectorExtractDynamic %float %45 %47
OpStore %x %48
%50 = OpLoad %v4float %v
%51 = OpLoad %v4int %i
%52 = OpCompositeExtract %int %51 1
%53 = OpVectorExtractDynamic %float %50 %52
OpStore %y %53
%55 = OpLoad %v4float %v
%56 = OpLoad %v4int %i
%57 = OpCompositeExtract %int %56 2
%58 = OpVectorExtractDynamic %float %55 %57
OpStore %z %58
%60 = OpLoad %v4float %v
%61 = OpLoad %v4int %i
%62 = OpCompositeExtract %int %61 3
%63 = OpVectorExtractDynamic %float %60 %62
OpStore %w %63
%64 = OpLoad %float %x
%65 = OpLoad %float %y
%66 = OpLoad %float %z
%67 = OpLoad %float %w
%68 = OpCompositeConstruct %v4float %64 %65 %66 %67
OpReturnValue %68
OpFunctionEnd
%main = OpFunction %v4float None %19
%69 = OpLabel
%77 = OpVariable %_ptr_Function_v4float Function
%70 = OpFunctionCall %v4float %non_constant_swizzle
%74 = OpFOrdEqual %v4bool %70 %73
%76 = OpAll %bool %74
OpSelectionMerge %80 None
OpBranchConditional %76 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%83 = OpLoad %v4float %81
OpStore %77 %83
OpBranch %80
%79 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%86 = OpLoad %v4float %84
OpStore %77 %86
OpBranch %80
%80 = OpLabel
%87 = OpLoad %v4float %77
OpReturnValue %87
OpFunctionEnd
