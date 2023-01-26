OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%ok = OpVariable %_ptr_Function_bool Function
%76 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpSelectionMerge %31 None
OpBranchConditional %true %30 %31
%30 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 1
%39 = OpFOrdEqual %bool %37 %float_1
%40 = OpSelect %bool %39 %true %false
OpBranch %31
%31 = OpLabel
%41 = OpPhi %bool %false %25 %40 %30
OpStore %ok %41
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %44
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %46 %float_1
%48 = OpSelect %bool %47 %false %true
OpBranch %43
%43 = OpLabel
%49 = OpPhi %bool %false %31 %48 %42
OpStore %ok %49
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 1 0
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%57 = OpLoad %v4float %55
%58 = OpVectorShuffle %v2float %57 %57 0 1
%59 = OpFOrdEqual %v2bool %54 %58
%61 = OpAll %bool %59
%62 = OpSelect %bool %61 %true %false
OpBranch %51
%51 = OpLabel
%63 = OpPhi %bool %false %43 %62 %50
OpStore %ok %63
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v2float %67 %67 1 0
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 0 1
%72 = OpFUnordNotEqual %v2bool %68 %71
%73 = OpAny %bool %72
%74 = OpSelect %bool %73 %false %true
OpBranch %65
%65 = OpLabel
%75 = OpPhi %bool %false %51 %74 %64
OpStore %ok %75
OpSelectionMerge %80 None
OpBranchConditional %75 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %81
OpStore %76 %82
OpBranch %80
%79 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %83
OpStore %76 %84
OpBranch %80
%80 = OpLabel
%85 = OpLoad %v4float %76
OpReturnValue %85
OpFunctionEnd
