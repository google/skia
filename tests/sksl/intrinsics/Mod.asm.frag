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
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedA "expectedA"
               OpName %expectedB "expectedB"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
 %float_0_25 = OpConstant %float 0.25
         %27 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0_75 %float_0_25
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0_75 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %47 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %60 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %71 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %82 = OpConstantComposite %v2float %float_0_75 %float_0
         %89 = OpConstantComposite %v3float %float_0_75 %float_0 %float_0_75
      %int_3 = OpConstant %int 3
        %150 = OpConstantComposite %v2float %float_0_25 %float_0
        %157 = OpConstantComposite %v3float %float_0_25 %float_0 %float_0_75
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
  %expectedA = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4float Function
        %165 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %27
               OpStore %expectedB %30
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %34
         %39 = OpCompositeExtract %float %38 0
         %33 = OpFMod %float %39 %float_1
         %40 = OpFOrdEqual %bool %33 %float_0_75
               OpSelectionMerge %42 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
         %44 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %45 = OpLoad %v4float %44
         %46 = OpVectorShuffle %v2float %45 %45 0 1
         %43 = OpFMod %v2float %46 %47
         %48 = OpVectorShuffle %v2float %27 %27 0 1
         %49 = OpFOrdEqual %v2bool %43 %48
         %51 = OpAll %bool %49
               OpBranch %42
         %42 = OpLabel
         %52 = OpPhi %bool %false %22 %51 %41
               OpSelectionMerge %54 None
               OpBranchConditional %52 %53 %54
         %53 = OpLabel
         %56 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %57 = OpLoad %v4float %56
         %58 = OpVectorShuffle %v3float %57 %57 0 1 2
         %55 = OpFMod %v3float %58 %60
         %61 = OpVectorShuffle %v3float %27 %27 0 1 2
         %62 = OpFOrdEqual %v3bool %55 %61
         %64 = OpAll %bool %62
               OpBranch %54
         %54 = OpLabel
         %65 = OpPhi %bool %false %42 %64 %53
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %70 = OpLoad %v4float %69
         %68 = OpFMod %v4float %70 %71
         %72 = OpFOrdEqual %v4bool %68 %27
         %74 = OpAll %bool %72
               OpBranch %67
         %67 = OpLabel
         %75 = OpPhi %bool %false %54 %74 %66
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
               OpBranch %77
         %77 = OpLabel
         %79 = OpPhi %bool %false %67 %true %76
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpVectorShuffle %v2float %27 %27 0 1
         %84 = OpFOrdEqual %v2bool %82 %83
         %85 = OpAll %bool %84
               OpBranch %81
         %81 = OpLabel
         %86 = OpPhi %bool %false %77 %85 %80
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpVectorShuffle %v3float %27 %27 0 1 2
         %91 = OpFOrdEqual %v3bool %89 %90
         %92 = OpAll %bool %91
               OpBranch %88
         %88 = OpLabel
         %93 = OpPhi %bool %false %81 %92 %87
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
               OpBranch %95
         %95 = OpLabel
         %96 = OpPhi %bool %false %88 %true %94
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %101 = OpLoad %v4float %100
        %102 = OpCompositeExtract %float %101 0
        %103 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %105 = OpLoad %v4float %103
        %106 = OpCompositeExtract %float %105 0
         %99 = OpFMod %float %102 %106
        %107 = OpFOrdEqual %bool %99 %float_0_75
               OpBranch %98
         %98 = OpLabel
        %108 = OpPhi %bool %false %95 %107 %97
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %113 = OpLoad %v4float %112
        %114 = OpVectorShuffle %v2float %113 %113 0 1
        %115 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 0 1
        %111 = OpFMod %v2float %114 %117
        %118 = OpVectorShuffle %v2float %27 %27 0 1
        %119 = OpFOrdEqual %v2bool %111 %118
        %120 = OpAll %bool %119
               OpBranch %110
        %110 = OpLabel
        %121 = OpPhi %bool %false %98 %120 %109
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %126 = OpLoad %v4float %125
        %127 = OpVectorShuffle %v3float %126 %126 0 1 2
        %128 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %129 = OpLoad %v4float %128
        %130 = OpVectorShuffle %v3float %129 %129 0 1 2
        %124 = OpFMod %v3float %127 %130
        %131 = OpVectorShuffle %v3float %27 %27 0 1 2
        %132 = OpFOrdEqual %v3bool %124 %131
        %133 = OpAll %bool %132
               OpBranch %123
        %123 = OpLabel
        %134 = OpPhi %bool %false %110 %133 %122
               OpSelectionMerge %136 None
               OpBranchConditional %134 %135 %136
        %135 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %139 = OpLoad %v4float %138
        %140 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %141 = OpLoad %v4float %140
        %137 = OpFMod %v4float %139 %141
        %142 = OpFOrdEqual %v4bool %137 %27
        %143 = OpAll %bool %142
               OpBranch %136
        %136 = OpLabel
        %144 = OpPhi %bool %false %123 %143 %135
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
               OpBranch %146
        %146 = OpLabel
        %147 = OpPhi %bool %false %136 %true %145
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
        %151 = OpVectorShuffle %v2float %30 %30 0 1
        %152 = OpFOrdEqual %v2bool %150 %151
        %153 = OpAll %bool %152
               OpBranch %149
        %149 = OpLabel
        %154 = OpPhi %bool %false %146 %153 %148
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %158 = OpVectorShuffle %v3float %30 %30 0 1 2
        %159 = OpFOrdEqual %v3bool %157 %158
        %160 = OpAll %bool %159
               OpBranch %156
        %156 = OpLabel
        %161 = OpPhi %bool %false %149 %160 %155
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %163
        %162 = OpLabel
               OpBranch %163
        %163 = OpLabel
        %164 = OpPhi %bool %false %156 %true %162
               OpSelectionMerge %168 None
               OpBranchConditional %164 %166 %167
        %166 = OpLabel
        %169 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %171 = OpLoad %v4float %169
               OpStore %165 %171
               OpBranch %168
        %167 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %174 = OpLoad %v4float %172
               OpStore %165 %174
               OpBranch %168
        %168 = OpLabel
        %175 = OpLoad %v4float %165
               OpReturnValue %175
               OpFunctionEnd
