               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %value "value"
               OpName %ok "ok"
               OpName %whole "whole"
               OpName %fraction "fraction"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %125 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
  %float_2_5 = OpConstant %float 2.5
 %float_n2_5 = OpConstant %float -2.5
    %float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
         %29 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
      %false = OpConstantFalse %bool
         %35 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
   %float_n2 = OpConstant %float -2
         %70 = OpConstantComposite %v2float %float_2 %float_n2
     %v2bool = OpTypeVector %bool 2
 %float_n0_5 = OpConstant %float -0.5
         %78 = OpConstantComposite %v2float %float_0_5 %float_n0_5
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %96 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
     %v3bool = OpTypeVector %bool 3
        %103 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
      %int_2 = OpConstant %int 2
        %113 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
        %118 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
      %value = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_v4bool Function
      %whole = OpVariable %_ptr_Function_v4float Function
   %fraction = OpVariable %_ptr_Function_v4float Function
         %43 = OpVariable %_ptr_Function_float Function
         %62 = OpVariable %_ptr_Function_v2float Function
         %88 = OpVariable %_ptr_Function_v3float Function
        %111 = OpVariable %_ptr_Function_v4float Function
        %126 = OpVariable %_ptr_Function_v4float Function
               OpStore %value %29
               OpStore %ok %35
         %39 = OpAccessChain %_ptr_Function_float %whole %int_0
         %38 = OpExtInst %float %1 Modf %float_2_5 %43
         %44 = OpLoad %float %43
               OpStore %39 %44
         %45 = OpAccessChain %_ptr_Function_float %fraction %int_0
               OpStore %45 %38
         %46 = OpLoad %v4float %whole
         %47 = OpCompositeExtract %float %46 0
         %49 = OpFOrdEqual %bool %47 %float_2
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %52 = OpLoad %v4float %fraction
         %53 = OpCompositeExtract %float %52 0
         %55 = OpFOrdEqual %bool %53 %float_0_5
               OpBranch %51
         %51 = OpLabel
         %56 = OpPhi %bool %false %22 %55 %50
         %57 = OpAccessChain %_ptr_Function_bool %ok %int_0
               OpStore %57 %56
         %60 = OpLoad %v4float %value
         %61 = OpVectorShuffle %v2float %60 %60 0 1
         %59 = OpExtInst %v2float %1 Modf %61 %62
         %63 = OpLoad %v2float %62
         %64 = OpLoad %v4float %whole
         %65 = OpVectorShuffle %v4float %64 %63 4 5 2 3
               OpStore %whole %65
         %66 = OpLoad %v4float %fraction
         %67 = OpVectorShuffle %v4float %66 %59 4 5 2 3
               OpStore %fraction %67
         %68 = OpVectorShuffle %v2float %65 %65 0 1
         %71 = OpFOrdEqual %v2bool %68 %70
         %73 = OpAll %bool %71
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %76 = OpVectorShuffle %v2float %67 %67 0 1
         %79 = OpFOrdEqual %v2bool %76 %78
         %80 = OpAll %bool %79
               OpBranch %75
         %75 = OpLabel
         %81 = OpPhi %bool %false %51 %80 %74
         %82 = OpAccessChain %_ptr_Function_bool %ok %int_1
               OpStore %82 %81
         %85 = OpLoad %v4float %value
         %86 = OpVectorShuffle %v3float %85 %85 0 1 2
         %84 = OpExtInst %v3float %1 Modf %86 %88
         %90 = OpLoad %v3float %88
         %91 = OpLoad %v4float %whole
         %92 = OpVectorShuffle %v4float %91 %90 4 5 6 3
               OpStore %whole %92
         %93 = OpLoad %v4float %fraction
         %94 = OpVectorShuffle %v4float %93 %84 4 5 6 3
               OpStore %fraction %94
         %95 = OpVectorShuffle %v3float %92 %92 0 1 2
         %97 = OpFOrdEqual %v3bool %95 %96
         %99 = OpAll %bool %97
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %102 = OpVectorShuffle %v3float %94 %94 0 1 2
        %104 = OpFOrdEqual %v3bool %102 %103
        %105 = OpAll %bool %104
               OpBranch %101
        %101 = OpLabel
        %106 = OpPhi %bool %false %75 %105 %100
        %107 = OpAccessChain %_ptr_Function_bool %ok %int_2
               OpStore %107 %106
        %110 = OpLoad %v4float %value
        %109 = OpExtInst %v4float %1 Modf %110 %111
        %112 = OpLoad %v4float %111
               OpStore %whole %112
               OpStore %fraction %109
        %114 = OpFOrdEqual %v4bool %112 %113
        %115 = OpAll %bool %114
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
        %119 = OpFOrdEqual %v4bool %109 %118
        %120 = OpAll %bool %119
               OpBranch %117
        %117 = OpLabel
        %121 = OpPhi %bool %false %101 %120 %116
        %122 = OpAccessChain %_ptr_Function_bool %ok %int_3
               OpStore %122 %121
        %125 = OpLoad %v4bool %ok
        %124 = OpAll %bool %125
               OpSelectionMerge %129 None
               OpBranchConditional %124 %127 %128
        %127 = OpLabel
        %130 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %132 = OpLoad %v4float %130
               OpStore %126 %132
               OpBranch %129
        %128 = OpLabel
        %133 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %134 = OpLoad %v4float %133
               OpStore %126 %134
               OpBranch %129
        %129 = OpLabel
        %135 = OpLoad %v4float %126
               OpReturnValue %135
               OpFunctionEnd
