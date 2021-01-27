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
OpDecorate %30 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_2 = OpConstant %float 2
%34 = OpConstantComposite %v2float %float_1 %float_2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_3 = OpConstant %float 3
%46 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%v3bool = OpTypeVector %bool 3
%float_4 = OpConstant %float 4
%56 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
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
%61 = OpVariable %_ptr_Function_v4float Function
%23 = OpLoad %v4float %minus1234
%24 = OpCompositeExtract %float %23 0
%22 = OpExtInst %float %1 FAbs %24
%26 = OpFOrdEqual %bool %22 %float_1
OpSelectionMerge %28 None
OpBranchConditional %26 %27 %28
%27 = OpLabel
%30 = OpLoad %v4float %minus1234
%31 = OpVectorShuffle %v2float %30 %30 0 1
%29 = OpExtInst %v2float %1 FAbs %31
%35 = OpFOrdEqual %v2bool %29 %34
%37 = OpAll %bool %35
OpBranch %28
%28 = OpLabel
%38 = OpPhi %bool %false %20 %37 %27
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%42 = OpLoad %v4float %minus1234
%43 = OpVectorShuffle %v3float %42 %42 0 1 2
%41 = OpExtInst %v3float %1 FAbs %43
%47 = OpFOrdEqual %v3bool %41 %46
%49 = OpAll %bool %47
OpBranch %40
%40 = OpLabel
%50 = OpPhi %bool %false %28 %49 %39
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpLoad %v4float %minus1234
%53 = OpExtInst %v4float %1 FAbs %54
%57 = OpFOrdEqual %v4bool %53 %56
%59 = OpAll %bool %57
OpBranch %52
%52 = OpLabel
%60 = OpPhi %bool %false %40 %59 %51
OpSelectionMerge %65 None
OpBranchConditional %60 %63 %64
%63 = OpLabel
%66 = OpLoad %v4float %colorGreen
OpStore %61 %66
OpBranch %65
%64 = OpLabel
%67 = OpLoad %v4float %colorRed
OpStore %61 %67
OpBranch %65
%65 = OpLabel
%68 = OpLoad %v4float %61
OpReturnValue %68
OpFunctionEnd

1 error
