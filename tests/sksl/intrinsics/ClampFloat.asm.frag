               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedA "expectedA"
               OpName %expectedB "expectedB"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
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
   %float_n1 = OpConstant %float -1
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
  %float_0_5 = OpConstant %float 0.5
 %float_2_25 = OpConstant %float 2.25
         %35 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %51 = OpConstantComposite %v2float %float_n1 %float_n1
         %52 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %65 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
         %66 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %77 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
         %78 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
   %float_n2 = OpConstant %float -2
         %98 = OpConstantComposite %v2float %float_n1 %float_n2
    %float_2 = OpConstant %float 2
        %100 = OpConstantComposite %v2float %float_1 %float_2
        %111 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
        %112 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
        %122 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
    %float_3 = OpConstant %float 3
        %124 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
       %true = OpConstantTrue %bool
        %134 = OpConstantComposite %v2float %float_n1 %float_0
        %141 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
        %160 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
  %expectedA = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4float Function
        %168 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %31
               OpStore %expectedB %35
         %38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %42 = OpLoad %v4float %38
         %43 = OpCompositeExtract %float %42 0
         %37 = OpExtInst %float %1 FClamp %43 %float_n1 %float_1
         %44 = OpFOrdEqual %bool %37 %float_n1
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 0 1
         %47 = OpExtInst %v2float %1 FClamp %50 %51 %52
         %53 = OpVectorShuffle %v2float %31 %31 0 1
         %54 = OpFOrdEqual %v2bool %47 %53
         %56 = OpAll %bool %54
               OpBranch %46
         %46 = OpLabel
         %57 = OpPhi %bool %false %25 %56 %45
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpVectorShuffle %v3float %62 %62 0 1 2
         %60 = OpExtInst %v3float %1 FClamp %63 %65 %66
         %67 = OpVectorShuffle %v3float %31 %31 0 1 2
         %68 = OpFOrdEqual %v3bool %60 %67
         %70 = OpAll %bool %68
               OpBranch %59
         %59 = OpLabel
         %71 = OpPhi %bool %false %46 %70 %58
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %76 = OpLoad %v4float %75
         %74 = OpExtInst %v4float %1 FClamp %76 %77 %78
         %79 = OpFOrdEqual %v4bool %74 %31
         %81 = OpAll %bool %79
               OpBranch %73
         %73 = OpLabel
         %82 = OpPhi %bool %false %59 %81 %72
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %87 = OpLoad %v4float %86
         %88 = OpCompositeExtract %float %87 0
         %85 = OpExtInst %float %1 FClamp %88 %float_n1 %float_1
         %89 = OpFOrdEqual %bool %85 %float_n1
               OpBranch %84
         %84 = OpLabel
         %90 = OpPhi %bool %false %73 %89 %83
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %95 = OpLoad %v4float %94
         %96 = OpVectorShuffle %v2float %95 %95 0 1
         %93 = OpExtInst %v2float %1 FClamp %96 %98 %100
        %101 = OpVectorShuffle %v2float %35 %35 0 1
        %102 = OpFOrdEqual %v2bool %93 %101
        %103 = OpAll %bool %102
               OpBranch %92
         %92 = OpLabel
        %104 = OpPhi %bool %false %84 %103 %91
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %109 = OpLoad %v4float %108
        %110 = OpVectorShuffle %v3float %109 %109 0 1 2
        %107 = OpExtInst %v3float %1 FClamp %110 %111 %112
        %113 = OpVectorShuffle %v3float %35 %35 0 1 2
        %114 = OpFOrdEqual %v3bool %107 %113
        %115 = OpAll %bool %114
               OpBranch %106
        %106 = OpLabel
        %116 = OpPhi %bool %false %92 %115 %105
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %121 = OpLoad %v4float %120
        %119 = OpExtInst %v4float %1 FClamp %121 %122 %124
        %125 = OpFOrdEqual %v4bool %119 %35
        %126 = OpAll %bool %125
               OpBranch %118
        %118 = OpLabel
        %127 = OpPhi %bool %false %106 %126 %117
               OpSelectionMerge %129 None
               OpBranchConditional %127 %128 %129
        %128 = OpLabel
               OpBranch %129
        %129 = OpLabel
        %131 = OpPhi %bool %false %118 %true %128
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %135 = OpVectorShuffle %v2float %31 %31 0 1
        %136 = OpFOrdEqual %v2bool %134 %135
        %137 = OpAll %bool %136
               OpBranch %133
        %133 = OpLabel
        %138 = OpPhi %bool %false %129 %137 %132
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
        %142 = OpVectorShuffle %v3float %31 %31 0 1 2
        %143 = OpFOrdEqual %v3bool %141 %142
        %144 = OpAll %bool %143
               OpBranch %140
        %140 = OpLabel
        %145 = OpPhi %bool %false %133 %144 %139
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
               OpBranch %147
        %147 = OpLabel
        %148 = OpPhi %bool %false %140 %true %146
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
               OpBranch %150
        %150 = OpLabel
        %151 = OpPhi %bool %false %147 %true %149
               OpSelectionMerge %153 None
               OpBranchConditional %151 %152 %153
        %152 = OpLabel
        %154 = OpVectorShuffle %v2float %35 %35 0 1
        %155 = OpFOrdEqual %v2bool %134 %154
        %156 = OpAll %bool %155
               OpBranch %153
        %153 = OpLabel
        %157 = OpPhi %bool %false %150 %156 %152
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %161 = OpVectorShuffle %v3float %35 %35 0 1 2
        %162 = OpFOrdEqual %v3bool %160 %161
        %163 = OpAll %bool %162
               OpBranch %159
        %159 = OpLabel
        %164 = OpPhi %bool %false %153 %163 %158
               OpSelectionMerge %166 None
               OpBranchConditional %164 %165 %166
        %165 = OpLabel
               OpBranch %166
        %166 = OpLabel
        %167 = OpPhi %bool %false %159 %true %165
               OpSelectionMerge %171 None
               OpBranchConditional %167 %169 %170
        %169 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %174 = OpLoad %v4float %172
               OpStore %168 %174
               OpBranch %171
        %170 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %177 = OpLoad %v4float %175
               OpStore %168 %177
               OpBranch %171
        %171 = OpLabel
        %178 = OpLoad %v4float %168
               OpReturnValue %178
               OpFunctionEnd
