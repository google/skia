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
OpName %expectFFTT "expectFFTT"
OpName %expectTTFF "expectTTFF"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%25 = OpConstantComposite %v4bool %false %false %true %true
%27 = OpConstantComposite %v4bool %true %true %false %false
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
%expectFFTT = OpVariable %_ptr_Function_v4bool Function
%expectTTFF = OpVariable %_ptr_Function_v4bool Function
OpStore %expectFFTT %25
OpStore %expectTTFF %27
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %29
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%35 = OpLoad %v4float %33
%28 = OpFOrdGreaterThan %v4bool %32 %35
%36 = OpCompositeExtract %bool %28 0
%37 = OpSelect %int %36 %int_1 %int_0
%38 = OpConvertSToF %float %37
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %38
%42 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_2
%45 = OpLoad %v2uint %42
%46 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_3
%48 = OpLoad %v2uint %46
%41 = OpUGreaterThan %v2bool %45 %48
%50 = OpCompositeExtract %bool %41 1
%51 = OpSelect %int %50 %int_1 %int_0
%52 = OpConvertSToF %float %51
%53 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %53 %52
%55 = OpAccessChain %_ptr_Uniform_v3int %10 %int_4
%58 = OpLoad %v3int %55
%59 = OpAccessChain %_ptr_Uniform_v3int %10 %int_5
%61 = OpLoad %v3int %59
%54 = OpSGreaterThan %v3bool %58 %61
%63 = OpCompositeExtract %bool %54 2
%64 = OpSelect %int %63 %int_1 %int_0
%65 = OpConvertSToF %float %64
%66 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %66 %65
%68 = OpLoad %v4bool %expectTTFF
%67 = OpAny %bool %68
OpSelectionMerge %70 None
OpBranchConditional %67 %70 %69
%69 = OpLabel
%72 = OpLoad %v4bool %expectFFTT
%71 = OpAny %bool %72
OpBranch %70
%70 = OpLabel
%73 = OpPhi %bool %true %19 %71 %69
%74 = OpSelect %int %73 %int_1 %int_0
%75 = OpConvertSToF %float %74
%76 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %76 %75
OpReturn
OpFunctionEnd
