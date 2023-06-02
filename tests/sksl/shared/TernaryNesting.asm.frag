OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %colorBlue "colorBlue"
OpName %colorGreen "colorGreen"
OpName %colorRed "colorRed"
OpName %result "result"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %colorBlue RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %colorGreen RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %colorRed RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4bool = OpTypeVector %bool 4
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
%colorBlue = OpVariable %_ptr_Function_v4float Function
%colorGreen = OpVariable %_ptr_Function_v4float Function
%colorRed = OpVariable %_ptr_Function_v4float Function
%result = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%76 = OpVariable %_ptr_Function_v4float Function
%84 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v2float %32 %32 2 3
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%36 = OpCompositeConstruct %v4float %float_0 %float_0 %34 %35
OpStore %colorBlue %36
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%40 = OpCompositeExtract %float %39 1
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%42 = OpLoad %v4float %41
%43 = OpCompositeExtract %float %42 3
%44 = OpCompositeConstruct %v4float %float_0 %40 %float_0 %43
OpStore %colorGreen %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpCompositeExtract %float %47 0
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 3
%52 = OpCompositeConstruct %v4float %48 %float_0 %float_0 %51
OpStore %colorRed %52
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpFUnordNotEqual %v4bool %55 %36
%58 = OpAny %bool %56
OpSelectionMerge %62 None
OpBranchConditional %58 %60 %61
%60 = OpLabel
%63 = OpFOrdEqual %v4bool %44 %52
%64 = OpAll %bool %63
%65 = OpCompositeConstruct %v4bool %64 %64 %64 %64
%66 = OpSelect %v4float %65 %52 %44
OpStore %59 %66
OpBranch %62
%61 = OpLabel
%67 = OpFUnordNotEqual %v4bool %52 %44
%68 = OpAny %bool %67
%69 = OpCompositeConstruct %v4bool %68 %68 %68 %68
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%70 = OpSelect %v4float %69 %36 %72
OpStore %59 %70
OpBranch %62
%62 = OpLabel
%73 = OpLoad %v4float %59
OpStore %result %73
%74 = OpFOrdEqual %v4bool %52 %36
%75 = OpAll %bool %74
OpSelectionMerge %79 None
OpBranchConditional %75 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%81 = OpLoad %v4float %80
OpStore %76 %81
OpBranch %79
%78 = OpLabel
%82 = OpFUnordNotEqual %v4bool %52 %44
%83 = OpAny %bool %82
OpSelectionMerge %87 None
OpBranchConditional %83 %85 %86
%85 = OpLabel
OpStore %84 %73
OpBranch %87
%86 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%89 = OpLoad %v4float %88
%90 = OpFOrdEqual %v4bool %52 %89
%91 = OpAll %bool %90
%92 = OpCompositeConstruct %v4bool %91 %91 %91 %91
%93 = OpSelect %v4float %92 %36 %52
OpStore %84 %93
OpBranch %87
%87 = OpLabel
%94 = OpLoad %v4float %84
OpStore %76 %94
OpBranch %79
%79 = OpLabel
%95 = OpLoad %v4float %76
OpReturnValue %95
OpFunctionEnd
