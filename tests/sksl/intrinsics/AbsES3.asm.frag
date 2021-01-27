### Compilation failed:

error: SPIR-V validation error: Uniform OpVariable <id> '10[%minus1234]' has illegal type.
From Vulkan spec, section 14.5.2:
Variables identified with the Uniform storage class are used to access transparent buffer backed resources. Such variables must be typed as OpTypeStruct, or an array of this type
  %minus1234 = OpVariable %_ptr_Uniform_v4float Uniform

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %minus1234 "minus1234"
OpName %colorGreen "colorGreen"
OpName %colorRed "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %minus1234 RelaxedPrecision
OpDecorate %minus1234 DescriptorSet 0
OpDecorate %colorGreen RelaxedPrecision
OpDecorate %colorGreen DescriptorSet 0
OpDecorate %colorRed RelaxedPrecision
OpDecorate %colorRed DescriptorSet 0
OpDecorate %23 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%minus1234 = OpVariable %_ptr_Uniform_v4float Uniform
%colorGreen = OpVariable %_ptr_Uniform_v4float Uniform
%colorRed = OpVariable %_ptr_Uniform_v4float Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%19 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%v2float = OpTypeVector %float 2
%v2int = OpTypeVector %int 2
%int_2 = OpConstant %int 2
%42 = OpConstantComposite %v2int %int_1 %int_2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%int_3 = OpConstant %int 3
%62 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%v3bool = OpTypeVector %bool 3
%v4int = OpTypeVector %int 4
%int_4 = OpConstant %int 4
%82 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %19
%20 = OpLabel
%87 = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %minus1234
%24 = OpCompositeExtract %float %23 0
%25 = OpConvertFToS %int %24
%22 = OpExtInst %int %1 SAbs %25
%28 = OpIEqual %bool %22 %int_1
OpSelectionMerge %30 None
OpBranchConditional %28 %29 %30
%29 = OpLabel
%32 = OpLoad %v4float %minus1234
%33 = OpVectorShuffle %v2float %32 %32 0 1
%35 = OpCompositeExtract %float %33 0
%36 = OpConvertFToS %int %35
%37 = OpCompositeExtract %float %33 1
%38 = OpConvertFToS %int %37
%39 = OpCompositeConstruct %v2int %36 %38
%31 = OpExtInst %v2int %1 SAbs %39
%43 = OpIEqual %v2bool %31 %42
%45 = OpAll %bool %43
OpBranch %30
%30 = OpLabel
%46 = OpPhi %bool %false %20 %45 %29
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpLoad %v4float %minus1234
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%53 = OpCompositeExtract %float %51 0
%54 = OpConvertFToS %int %53
%55 = OpCompositeExtract %float %51 1
%56 = OpConvertFToS %int %55
%57 = OpCompositeExtract %float %51 2
%58 = OpConvertFToS %int %57
%59 = OpCompositeConstruct %v3int %54 %56 %58
%49 = OpExtInst %v3int %1 SAbs %59
%63 = OpIEqual %v3bool %49 %62
%65 = OpAll %bool %63
OpBranch %48
%48 = OpLabel
%66 = OpPhi %bool %false %30 %65 %47
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpLoad %v4float %minus1234
%71 = OpCompositeExtract %float %70 0
%72 = OpConvertFToS %int %71
%73 = OpCompositeExtract %float %70 1
%74 = OpConvertFToS %int %73
%75 = OpCompositeExtract %float %70 2
%76 = OpConvertFToS %int %75
%77 = OpCompositeExtract %float %70 3
%78 = OpConvertFToS %int %77
%79 = OpCompositeConstruct %v4int %72 %74 %76 %78
%69 = OpExtInst %v4int %1 SAbs %79
%83 = OpIEqual %v4bool %69 %82
%85 = OpAll %bool %83
OpBranch %68
%68 = OpLabel
%86 = OpPhi %bool %false %48 %85 %67
OpSelectionMerge %91 None
OpBranchConditional %86 %89 %90
%89 = OpLabel
%92 = OpLoad %v4float %colorGreen
OpStore %87 %92
OpBranch %91
%90 = OpLabel
%93 = OpLoad %v4float %colorRed
OpStore %87 %93
OpBranch %91
%91 = OpLabel
%94 = OpLoad %v4float %87
OpReturnValue %94
OpFunctionEnd

1 error
