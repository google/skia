               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedVec "expectedVec"
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
               OpDecorate %expectedVec RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
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
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %75 = OpConstantComposite %v2float %float_0 %float_1
         %82 = OpConstantComposite %v3float %float_0 %float_1 %float_0
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
%expectedVec = OpVariable %_ptr_Function_v4float Function
         %90 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedVec %26
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %30
         %35 = OpCompositeExtract %float %34 0
         %29 = OpExtInst %float %1 Normalize %35
         %36 = OpFOrdEqual %bool %29 %float_1
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpVectorShuffle %v2float %41 %41 0 1
         %39 = OpExtInst %v2float %1 Normalize %42
         %43 = OpVectorShuffle %v2float %26 %26 0 1
         %44 = OpFOrdEqual %v2bool %39 %43
         %46 = OpAll %bool %44
               OpBranch %38
         %38 = OpLabel
         %47 = OpPhi %bool %false %22 %46 %37
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %51 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %52 = OpLoad %v4float %51
         %53 = OpVectorShuffle %v3float %52 %52 0 1 2
         %50 = OpExtInst %v3float %1 Normalize %53
         %55 = OpVectorShuffle %v3float %26 %26 0 1 2
         %56 = OpFOrdEqual %v3bool %50 %55
         %58 = OpAll %bool %56
               OpBranch %49
         %49 = OpLabel
         %59 = OpPhi %bool %false %38 %58 %48
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %64 = OpLoad %v4float %63
         %62 = OpExtInst %v4float %1 Normalize %64
         %65 = OpFOrdEqual %v4bool %62 %26
         %67 = OpAll %bool %65
               OpBranch %61
         %61 = OpLabel
         %68 = OpPhi %bool %false %49 %67 %60
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
               OpBranch %70
         %70 = OpLabel
         %72 = OpPhi %bool %false %61 %true %69
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpVectorShuffle %v2float %26 %26 1 0
         %77 = OpFOrdEqual %v2bool %75 %76
         %78 = OpAll %bool %77
               OpBranch %74
         %74 = OpLabel
         %79 = OpPhi %bool %false %70 %78 %73
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpVectorShuffle %v3float %26 %26 2 0 1
         %84 = OpFOrdEqual %v3bool %82 %83
         %85 = OpAll %bool %84
               OpBranch %81
         %81 = OpLabel
         %86 = OpPhi %bool %false %74 %85 %80
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
               OpBranch %88
         %88 = OpLabel
         %89 = OpPhi %bool %false %81 %true %87
               OpSelectionMerge %93 None
               OpBranchConditional %89 %91 %92
         %91 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %96 = OpLoad %v4float %94
               OpStore %90 %96
               OpBranch %93
         %92 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %99 = OpLoad %v4float %97
               OpStore %90 %99
               OpBranch %93
         %93 = OpLabel
        %100 = OpLoad %v4float %90
               OpReturnValue %100
               OpFunctionEnd
