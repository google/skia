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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m43 "_1_m43"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m43 = OpVariable %_ptr_Function_mat4v3float Function
%71 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%34 = OpCompositeConstruct %mat4v3float %35 %36 %37 %38
OpStore %_1_m43 %34
%40 = OpLoad %bool %_0_ok
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %mat4v3float %_1_m43
%45 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%46 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%47 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%48 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%44 = OpCompositeConstruct %mat4v3float %45 %46 %47 %48
%50 = OpCompositeExtract %v3float %43 0
%51 = OpCompositeExtract %v3float %44 0
%52 = OpFOrdEqual %v3bool %50 %51
%53 = OpAll %bool %52
%54 = OpCompositeExtract %v3float %43 1
%55 = OpCompositeExtract %v3float %44 1
%56 = OpFOrdEqual %v3bool %54 %55
%57 = OpAll %bool %56
%58 = OpLogicalAnd %bool %53 %57
%59 = OpCompositeExtract %v3float %43 2
%60 = OpCompositeExtract %v3float %44 2
%61 = OpFOrdEqual %v3bool %59 %60
%62 = OpAll %bool %61
%63 = OpLogicalAnd %bool %58 %62
%64 = OpCompositeExtract %v3float %43 3
%65 = OpCompositeExtract %v3float %44 3
%66 = OpFOrdEqual %v3bool %64 %65
%67 = OpAll %bool %66
%68 = OpLogicalAnd %bool %63 %67
OpBranch %42
%42 = OpLabel
%69 = OpPhi %bool %false %25 %68 %41
OpStore %_0_ok %69
%70 = OpLoad %bool %_0_ok
OpSelectionMerge %75 None
OpBranchConditional %70 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %76
OpStore %71 %80
OpBranch %75
%74 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%83 = OpLoad %v4float %81
OpStore %71 %83
OpBranch %75
%75 = OpLabel
%84 = OpLoad %v4float %71
OpReturnValue %84
OpFunctionEnd
