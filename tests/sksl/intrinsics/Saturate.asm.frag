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
               OpName %expected "expected"
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
               OpDecorate %expected RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %44 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %57 = OpConstantComposite %v3float %float_0 %float_0 %float_0
         %58 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %69 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %70 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %87 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
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
   %expected = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %27
         %31 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %35 = OpLoad %v4float %31
         %36 = OpCompositeExtract %float %35 0
         %30 = OpExtInst %float %1 FClamp %36 %float_0 %float_1
         %37 = OpFOrdEqual %bool %30 %float_0
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %41 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpVectorShuffle %v2float %42 %42 0 1
         %40 = OpExtInst %v2float %1 FClamp %43 %16 %44
         %45 = OpVectorShuffle %v2float %27 %27 0 1
         %46 = OpFOrdEqual %v2bool %40 %45
         %48 = OpAll %bool %46
               OpBranch %39
         %39 = OpLabel
         %49 = OpPhi %bool %false %22 %48 %38
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %54 = OpLoad %v4float %53
         %55 = OpVectorShuffle %v3float %54 %54 0 1 2
         %52 = OpExtInst %v3float %1 FClamp %55 %57 %58
         %59 = OpVectorShuffle %v3float %27 %27 0 1 2
         %60 = OpFOrdEqual %v3bool %52 %59
         %62 = OpAll %bool %60
               OpBranch %51
         %51 = OpLabel
         %63 = OpPhi %bool %false %39 %62 %50
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %67 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %68 = OpLoad %v4float %67
         %66 = OpExtInst %v4float %1 FClamp %68 %69 %70
         %71 = OpFOrdEqual %v4bool %66 %27
         %73 = OpAll %bool %71
               OpBranch %65
         %65 = OpLabel
         %74 = OpPhi %bool %false %51 %73 %64
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
               OpBranch %76
         %76 = OpLabel
         %78 = OpPhi %bool %false %65 %true %75
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %81 = OpVectorShuffle %v2float %27 %27 0 1
         %82 = OpFOrdEqual %v2bool %16 %81
         %83 = OpAll %bool %82
               OpBranch %80
         %80 = OpLabel
         %84 = OpPhi %bool %false %76 %83 %79
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpVectorShuffle %v3float %27 %27 0 1 2
         %89 = OpFOrdEqual %v3bool %87 %88
         %90 = OpAll %bool %89
               OpBranch %86
         %86 = OpLabel
         %91 = OpPhi %bool %false %80 %90 %85
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
               OpBranch %93
         %93 = OpLabel
         %94 = OpPhi %bool %false %86 %true %92
               OpSelectionMerge %98 None
               OpBranchConditional %94 %96 %97
         %96 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %101 = OpLoad %v4float %99
               OpStore %95 %101
               OpBranch %98
         %97 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %104 = OpLoad %v4float %102
               OpStore %95 %104
               OpBranch %98
         %98 = OpLabel
        %105 = OpLoad %v4float %95
               OpReturnValue %105
               OpFunctionEnd
