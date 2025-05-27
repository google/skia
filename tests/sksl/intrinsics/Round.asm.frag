               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
   %float_n1 = OpConstant %float -1
         %40 = OpConstantComposite %v2float %float_n1 %float_0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
         %53 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
     %v3bool = OpTypeVector %bool 3
    %float_2 = OpConstant %float 2
         %64 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_2
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
         %69 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %26
         %31 = OpCompositeExtract %float %30 0
         %25 = OpExtInst %float %1 Round %31
         %33 = OpFOrdEqual %bool %25 %float_n1
               OpSelectionMerge %35 None
               OpBranchConditional %33 %34 %35
         %34 = OpLabel
         %37 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %37
         %39 = OpVectorShuffle %v2float %38 %38 0 1
         %36 = OpExtInst %v2float %1 Round %39
         %41 = OpFOrdEqual %v2bool %36 %40
         %43 = OpAll %bool %41
               OpBranch %35
         %35 = OpLabel
         %44 = OpPhi %bool %false %22 %43 %34
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v3float %49 %49 0 1 2
         %47 = OpExtInst %v3float %1 Round %50
         %54 = OpFOrdEqual %v3bool %47 %53
         %56 = OpAll %bool %54
               OpBranch %46
         %46 = OpLabel
         %57 = OpPhi %bool %false %35 %56 %45
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 = OpLoad %v4float %61
         %60 = OpExtInst %v4float %1 Round %62
         %65 = OpFOrdEqual %v4bool %60 %64
         %67 = OpAll %bool %65
               OpBranch %59
         %59 = OpLabel
         %68 = OpPhi %bool %false %46 %67 %58
               OpSelectionMerge %73 None
               OpBranchConditional %68 %71 %72
         %71 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %76 = OpLoad %v4float %74
               OpStore %69 %76
               OpBranch %73
         %72 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %79 = OpLoad %v4float %77
               OpStore %69 %79
               OpBranch %73
         %73 = OpLabel
         %80 = OpLoad %v4float %69
               OpReturnValue %80
               OpFunctionEnd
