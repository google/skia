OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "a"
OpMemberName %_UniformBuffer 1 "b"
OpMemberName %_UniformBuffer 2 "c"
OpMemberName %_UniformBuffer 3 "d"
OpMemberName %_UniformBuffer 4 "e"
OpMemberName %_UniformBuffer 5 "f"
OpName %main "main"
OpName %expectTTFF "expectTTFF"
OpName %expectFFTT "expectFFTT"
OpName %expectTTTT "expectTTTT"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 3 Offset 40
OpMemberDecorate %_UniformBuffer 4 Offset 48
OpMemberDecorate %_UniformBuffer 5 Offset 64
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%uint = OpTypeInt 32 0
%v2uint = OpTypeVector %uint 2
%int = OpTypeInt 32 1
%v3int = OpTypeVector %int 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %v2uint %v2uint %v3int %v3int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%25 = OpConstantComposite %v4bool %true %true %false %false
%27 = OpConstantComposite %v4bool %false %false %true %true
%29 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v2uint = OpTypePointer Uniform %v2uint
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_v3int = OpTypePointer Uniform %v3int
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v3bool = OpTypeVector %bool 3
%main = OpFunction %void None %18
%19 = OpLabel
%expectTTFF = OpVariable %_ptr_Function_v4bool Function
%expectFFTT = OpVariable %_ptr_Function_v4bool Function
%expectTTTT = OpVariable %_ptr_Function_v4bool Function
OpStore %expectTTFF %25
OpStore %expectFFTT %27
OpStore %expectTTTT %29
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %31
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%37 = OpLoad %v4float %35
%30 = OpFOrdEqual %v4bool %34 %37
%38 = OpCompositeExtract %bool %30 0
%39 = OpSelect %int %38 %int_1 %int_0
%40 = OpConvertSToF %float %39
%41 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %41 %40
%44 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_2
%47 = OpLoad %v2uint %44
%48 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_3
%50 = OpLoad %v2uint %48
%43 = OpIEqual %v2bool %47 %50
%52 = OpCompositeExtract %bool %43 1
%53 = OpSelect %int %52 %int_1 %int_0
%54 = OpConvertSToF %float %53
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %55 %54
%57 = OpAccessChain %_ptr_Uniform_v3int %10 %int_4
%60 = OpLoad %v3int %57
%61 = OpAccessChain %_ptr_Uniform_v3int %10 %int_5
%63 = OpLoad %v3int %61
%56 = OpIEqual %v3bool %60 %63
%65 = OpCompositeExtract %bool %56 2
%66 = OpSelect %int %65 %int_1 %int_0
%67 = OpConvertSToF %float %66
%68 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %68 %67
%70 = OpLoad %v4bool %expectTTFF
%69 = OpAny %bool %70
OpSelectionMerge %72 None
OpBranchConditional %69 %72 %71
%71 = OpLabel
%74 = OpLoad %v4bool %expectFFTT
%73 = OpAny %bool %74
OpBranch %72
%72 = OpLabel
%75 = OpPhi %bool %true %19 %73 %71
OpSelectionMerge %77 None
OpBranchConditional %75 %77 %76
%76 = OpLabel
%79 = OpLoad %v4bool %expectTTTT
%78 = OpAny %bool %79
OpBranch %77
%77 = OpLabel
%80 = OpPhi %bool %true %72 %78 %76
%81 = OpSelect %int %80 %int_1 %int_0
%82 = OpConvertSToF %float %81
%83 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %83 %82
OpReturn
OpFunctionEnd
