               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %main "main"
               OpName %i "i"
               OpName %i_0 "i"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_5 = OpConstant %float 5
       %bool = OpTypeBool
 %float_0_75 = OpConstant %float 0.75
         %25 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%_ptr_Function_int = OpTypePointer Function %int
     %int_10 = OpConstant %int 10
  %float_0_5 = OpConstant %float 0.5
      %int_1 = OpConstant %int 1
 %float_0_25 = OpConstant %float 0.25
         %49 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
      %int_2 = OpConstant %int 2
    %int_100 = OpConstant %int 100
       %main = OpFunction %void None %11
         %12 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
        %i_0 = OpVariable %_ptr_Function_int Function
         %13 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %17 = OpLoad %float %13
         %19 = OpFOrdGreaterThan %bool %17 %float_5
               OpSelectionMerge %23 None
               OpBranchConditional %19 %21 %22
         %21 = OpLabel
               OpStore %sk_FragColor %25
               OpBranch %23
         %22 = OpLabel
               OpKill
         %23 = OpLabel
               OpStore %i %int_0
               OpBranch %28
         %28 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranch %29
         %29 = OpLabel
         %33 = OpLoad %int %i
         %35 = OpSLessThan %bool %33 %int_10
               OpBranchConditional %35 %30 %32
         %30 = OpLabel
         %36 = OpLoad %v4float %sk_FragColor
         %38 = OpVectorTimesScalar %v4float %36 %float_0_5
               OpStore %sk_FragColor %38
         %39 = OpLoad %int %i
         %41 = OpIAdd %int %39 %int_1
               OpStore %i %41
               OpBranch %31
         %31 = OpLabel
               OpBranch %28
         %32 = OpLabel
               OpBranch %42
         %42 = OpLabel
               OpLoopMerge %46 %45 None
               OpBranch %43
         %43 = OpLabel
         %47 = OpLoad %v4float %sk_FragColor
         %50 = OpFAdd %v4float %47 %49
               OpStore %sk_FragColor %50
               OpBranch %44
         %44 = OpLabel
               OpBranch %45
         %45 = OpLabel
         %51 = OpLoad %v4float %sk_FragColor
         %52 = OpCompositeExtract %float %51 0
         %53 = OpFOrdLessThan %bool %52 %float_0_75
               OpBranchConditional %53 %42 %46
         %46 = OpLabel
               OpStore %i_0 %int_0
               OpBranch %55
         %55 = OpLabel
               OpLoopMerge %59 %58 None
               OpBranch %56
         %56 = OpLabel
         %60 = OpLoad %int %i_0
         %61 = OpSLessThan %bool %60 %int_10
               OpBranchConditional %61 %57 %59
         %57 = OpLabel
         %62 = OpLoad %int %i_0
         %64 = OpSMod %int %62 %int_2
         %65 = OpIEqual %bool %64 %int_1
               OpSelectionMerge %68 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
               OpBranch %59
         %67 = OpLabel
         %69 = OpLoad %int %i_0
         %71 = OpSGreaterThan %bool %69 %int_100
               OpSelectionMerge %74 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
               OpReturn
         %73 = OpLabel
               OpBranch %58
         %74 = OpLabel
               OpBranch %68
         %68 = OpLabel
               OpBranch %58
         %58 = OpLabel
         %75 = OpLoad %int %i_0
         %76 = OpIAdd %int %75 %int_1
               OpStore %i_0 %76
               OpBranch %55
         %59 = OpLabel
               OpReturn
               OpFunctionEnd
