               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %integerInput "integerInput"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%float_2_1400001 = OpConstant %float 2.1400001
         %38 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
         %53 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%float_3_1400001 = OpConstant %float 3.1400001
         %63 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
         %72 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %73 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
%integerInput = OpVariable %_ptr_Function_int Function
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %26
         %30 = OpCompositeExtract %float %29 1
         %31 = OpConvertFToS %int %30
               OpStore %integerInput %31
         %32 = OpIEqual %bool %31 %int_0
               OpSelectionMerge %36 None
               OpBranchConditional %32 %34 %35
         %34 = OpLabel
               OpReturnValue %38
         %35 = OpLabel
         %40 = OpIEqual %bool %31 %int_1
               OpSelectionMerge %43 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
         %44 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %45 = OpLoad %v4float %44
               OpReturnValue %45
         %42 = OpLabel
         %47 = OpIEqual %bool %31 %int_2
               OpSelectionMerge %50 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
               OpReturnValue %53
         %49 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %56 = OpLoad %v4float %55
         %57 = OpCompositeExtract %float %56 0
         %58 = OpFMul %float %57 %float_3_1400001
         %59 = OpFOrdLessThan %bool %float_3_1400001 %58
               OpSelectionMerge %62 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
               OpReturnValue %63
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpCompositeExtract %float %65 0
         %67 = OpFMul %float %66 %float_2_1400001
         %68 = OpFOrdGreaterThanEqual %bool %float_2_1400001 %67
               OpSelectionMerge %71 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
               OpReturnValue %72
         %70 = OpLabel
               OpReturnValue %73
         %71 = OpLabel
               OpBranch %62
         %62 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpBranch %43
         %43 = OpLabel
               OpBranch %36
         %36 = OpLabel
               OpUnreachable
               OpFunctionEnd
