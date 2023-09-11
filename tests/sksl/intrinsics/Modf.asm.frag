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
               OpName %value "value"
               OpName %ok "ok"
               OpName %whole "whole"
               OpName %fraction "fraction"
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
               OpDecorate %127 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %float_2_5 = OpConstant %float 2.5
 %float_n2_5 = OpConstant %float -2.5
    %float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
         %32 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
      %false = OpConstantFalse %bool
         %37 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
   %float_n2 = OpConstant %float -2
         %72 = OpConstantComposite %v2float %float_2 %float_n2
     %v2bool = OpTypeVector %bool 2
 %float_n0_5 = OpConstant %float -0.5
         %80 = OpConstantComposite %v2float %float_0_5 %float_n0_5
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %98 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
     %v3bool = OpTypeVector %bool 3
        %105 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
      %int_2 = OpConstant %int 2
        %115 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
        %120 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
      %value = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_v4bool Function
      %whole = OpVariable %_ptr_Function_v4float Function
   %fraction = OpVariable %_ptr_Function_v4float Function
         %45 = OpVariable %_ptr_Function_float Function
         %64 = OpVariable %_ptr_Function_v2float Function
         %90 = OpVariable %_ptr_Function_v3float Function
        %113 = OpVariable %_ptr_Function_v4float Function
        %128 = OpVariable %_ptr_Function_v4float Function
               OpStore %value %32
               OpStore %ok %37
         %41 = OpAccessChain %_ptr_Function_float %whole %int_0
         %40 = OpExtInst %float %1 Modf %float_2_5 %45
         %46 = OpLoad %float %45
               OpStore %41 %46
         %47 = OpAccessChain %_ptr_Function_float %fraction %int_0
               OpStore %47 %40
         %48 = OpLoad %v4float %whole
         %49 = OpCompositeExtract %float %48 0
         %51 = OpFOrdEqual %bool %49 %float_2
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %54 = OpLoad %v4float %fraction
         %55 = OpCompositeExtract %float %54 0
         %57 = OpFOrdEqual %bool %55 %float_0_5
               OpBranch %53
         %53 = OpLabel
         %58 = OpPhi %bool %false %25 %57 %52
         %59 = OpAccessChain %_ptr_Function_bool %ok %int_0
               OpStore %59 %58
         %62 = OpLoad %v4float %value
         %63 = OpVectorShuffle %v2float %62 %62 0 1
         %61 = OpExtInst %v2float %1 Modf %63 %64
         %65 = OpLoad %v2float %64
         %66 = OpLoad %v4float %whole
         %67 = OpVectorShuffle %v4float %66 %65 4 5 2 3
               OpStore %whole %67
         %68 = OpLoad %v4float %fraction
         %69 = OpVectorShuffle %v4float %68 %61 4 5 2 3
               OpStore %fraction %69
         %70 = OpVectorShuffle %v2float %67 %67 0 1
         %73 = OpFOrdEqual %v2bool %70 %72
         %75 = OpAll %bool %73
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpVectorShuffle %v2float %69 %69 0 1
         %81 = OpFOrdEqual %v2bool %78 %80
         %82 = OpAll %bool %81
               OpBranch %77
         %77 = OpLabel
         %83 = OpPhi %bool %false %53 %82 %76
         %84 = OpAccessChain %_ptr_Function_bool %ok %int_1
               OpStore %84 %83
         %87 = OpLoad %v4float %value
         %88 = OpVectorShuffle %v3float %87 %87 0 1 2
         %86 = OpExtInst %v3float %1 Modf %88 %90
         %92 = OpLoad %v3float %90
         %93 = OpLoad %v4float %whole
         %94 = OpVectorShuffle %v4float %93 %92 4 5 6 3
               OpStore %whole %94
         %95 = OpLoad %v4float %fraction
         %96 = OpVectorShuffle %v4float %95 %86 4 5 6 3
               OpStore %fraction %96
         %97 = OpVectorShuffle %v3float %94 %94 0 1 2
         %99 = OpFOrdEqual %v3bool %97 %98
        %101 = OpAll %bool %99
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %104 = OpVectorShuffle %v3float %96 %96 0 1 2
        %106 = OpFOrdEqual %v3bool %104 %105
        %107 = OpAll %bool %106
               OpBranch %103
        %103 = OpLabel
        %108 = OpPhi %bool %false %77 %107 %102
        %109 = OpAccessChain %_ptr_Function_bool %ok %int_2
               OpStore %109 %108
        %112 = OpLoad %v4float %value
        %111 = OpExtInst %v4float %1 Modf %112 %113
        %114 = OpLoad %v4float %113
               OpStore %whole %114
               OpStore %fraction %111
        %116 = OpFOrdEqual %v4bool %114 %115
        %117 = OpAll %bool %116
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
        %121 = OpFOrdEqual %v4bool %111 %120
        %122 = OpAll %bool %121
               OpBranch %119
        %119 = OpLabel
        %123 = OpPhi %bool %false %103 %122 %118
        %124 = OpAccessChain %_ptr_Function_bool %ok %int_3
               OpStore %124 %123
        %127 = OpLoad %v4bool %ok
        %126 = OpAll %bool %127
               OpSelectionMerge %131 None
               OpBranchConditional %126 %129 %130
        %129 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %134 = OpLoad %v4float %132
               OpStore %128 %134
               OpBranch %131
        %130 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %136 = OpLoad %v4float %135
               OpStore %128 %136
               OpBranch %131
        %131 = OpLabel
        %137 = OpLoad %v4float %128
               OpReturnValue %137
               OpFunctionEnd
