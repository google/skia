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
OpName %_1_m42 "_1_m42"
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
OpDecorate %39 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
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
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
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
%_1_m42 = OpVariable %_ptr_Function_mat4v2float Function
%70 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%34 = OpCompositeConstruct %v2float %float_6 %float_0
%35 = OpCompositeConstruct %v2float %float_0 %float_6
%36 = OpCompositeConstruct %v2float %float_0 %float_0
%37 = OpCompositeConstruct %v2float %float_0 %float_0
%33 = OpCompositeConstruct %mat4v2float %34 %35 %36 %37
OpStore %_1_m42 %33
%39 = OpLoad %bool %_0_ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %mat4v2float %_1_m42
%44 = OpCompositeConstruct %v2float %float_6 %float_0
%45 = OpCompositeConstruct %v2float %float_0 %float_6
%46 = OpCompositeConstruct %v2float %float_0 %float_0
%47 = OpCompositeConstruct %v2float %float_0 %float_0
%43 = OpCompositeConstruct %mat4v2float %44 %45 %46 %47
%49 = OpCompositeExtract %v2float %42 0
%50 = OpCompositeExtract %v2float %43 0
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpCompositeExtract %v2float %42 1
%54 = OpCompositeExtract %v2float %43 1
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %52 %56
%58 = OpCompositeExtract %v2float %42 2
%59 = OpCompositeExtract %v2float %43 2
%60 = OpFOrdEqual %v2bool %58 %59
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %57 %61
%63 = OpCompositeExtract %v2float %42 3
%64 = OpCompositeExtract %v2float %43 3
%65 = OpFOrdEqual %v2bool %63 %64
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %62 %66
OpBranch %41
%41 = OpLabel
%68 = OpPhi %bool %false %25 %67 %40
OpStore %_0_ok %68
%69 = OpLoad %bool %_0_ok
OpSelectionMerge %74 None
OpBranchConditional %69 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %75
OpStore %70 %79
OpBranch %74
%73 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%82 = OpLoad %v4float %80
OpStore %70 %82
OpBranch %74
%74 = OpLabel
%83 = OpLoad %v4float %70
OpReturnValue %83
OpFunctionEnd
