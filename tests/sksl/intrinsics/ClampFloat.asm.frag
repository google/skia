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
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
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
   %float_n1 = OpConstant %float -1
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %28 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
  %float_0_5 = OpConstant %float 0.5
 %float_2_25 = OpConstant %float 2.25
         %32 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %49 = OpConstantComposite %v2float %float_n1 %float_n1
         %50 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %63 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
         %64 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %75 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
         %76 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
   %float_n2 = OpConstant %float -2
         %96 = OpConstantComposite %v2float %float_n1 %float_n2
    %float_2 = OpConstant %float 2
         %98 = OpConstantComposite %v2float %float_1 %float_2
        %109 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
        %110 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
        %120 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
    %float_3 = OpConstant %float 3
        %122 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
       %true = OpConstantTrue %bool
        %132 = OpConstantComposite %v2float %float_n1 %float_0
        %139 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
        %158 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
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
        %166 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %28
               OpStore %expectedB %32
         %36 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %40 = OpLoad %v4float %36
         %41 = OpCompositeExtract %float %40 0
         %35 = OpExtInst %float %1 FClamp %41 %float_n1 %float_1
         %42 = OpFOrdEqual %bool %35 %float_n1
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %45 = OpExtInst %v2float %1 FClamp %48 %49 %50
         %51 = OpVectorShuffle %v2float %28 %28 0 1
         %52 = OpFOrdEqual %v2bool %45 %51
         %54 = OpAll %bool %52
               OpBranch %44
         %44 = OpLabel
         %55 = OpPhi %bool %false %22 %54 %43
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %58 = OpExtInst %v3float %1 FClamp %61 %63 %64
         %65 = OpVectorShuffle %v3float %28 %28 0 1 2
         %66 = OpFOrdEqual %v3bool %58 %65
         %68 = OpAll %bool %66
               OpBranch %57
         %57 = OpLabel
         %69 = OpPhi %bool %false %44 %68 %56
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %74 = OpLoad %v4float %73
         %72 = OpExtInst %v4float %1 FClamp %74 %75 %76
         %77 = OpFOrdEqual %v4bool %72 %28
         %79 = OpAll %bool %77
               OpBranch %71
         %71 = OpLabel
         %80 = OpPhi %bool %false %57 %79 %70
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 0
         %83 = OpExtInst %float %1 FClamp %86 %float_n1 %float_1
         %87 = OpFOrdEqual %bool %83 %float_n1
               OpBranch %82
         %82 = OpLabel
         %88 = OpPhi %bool %false %71 %87 %81
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %93 = OpLoad %v4float %92
         %94 = OpVectorShuffle %v2float %93 %93 0 1
         %91 = OpExtInst %v2float %1 FClamp %94 %96 %98
         %99 = OpVectorShuffle %v2float %32 %32 0 1
        %100 = OpFOrdEqual %v2bool %91 %99
        %101 = OpAll %bool %100
               OpBranch %90
         %90 = OpLabel
        %102 = OpPhi %bool %false %82 %101 %89
               OpSelectionMerge %104 None
               OpBranchConditional %102 %103 %104
        %103 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %107 = OpLoad %v4float %106
        %108 = OpVectorShuffle %v3float %107 %107 0 1 2
        %105 = OpExtInst %v3float %1 FClamp %108 %109 %110
        %111 = OpVectorShuffle %v3float %32 %32 0 1 2
        %112 = OpFOrdEqual %v3bool %105 %111
        %113 = OpAll %bool %112
               OpBranch %104
        %104 = OpLabel
        %114 = OpPhi %bool %false %90 %113 %103
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %118 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %119 = OpLoad %v4float %118
        %117 = OpExtInst %v4float %1 FClamp %119 %120 %122
        %123 = OpFOrdEqual %v4bool %117 %32
        %124 = OpAll %bool %123
               OpBranch %116
        %116 = OpLabel
        %125 = OpPhi %bool %false %104 %124 %115
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
               OpBranch %127
        %127 = OpLabel
        %129 = OpPhi %bool %false %116 %true %126
               OpSelectionMerge %131 None
               OpBranchConditional %129 %130 %131
        %130 = OpLabel
        %133 = OpVectorShuffle %v2float %28 %28 0 1
        %134 = OpFOrdEqual %v2bool %132 %133
        %135 = OpAll %bool %134
               OpBranch %131
        %131 = OpLabel
        %136 = OpPhi %bool %false %127 %135 %130
               OpSelectionMerge %138 None
               OpBranchConditional %136 %137 %138
        %137 = OpLabel
        %140 = OpVectorShuffle %v3float %28 %28 0 1 2
        %141 = OpFOrdEqual %v3bool %139 %140
        %142 = OpAll %bool %141
               OpBranch %138
        %138 = OpLabel
        %143 = OpPhi %bool %false %131 %142 %137
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
               OpBranch %145
        %145 = OpLabel
        %146 = OpPhi %bool %false %138 %true %144
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
               OpBranch %148
        %148 = OpLabel
        %149 = OpPhi %bool %false %145 %true %147
               OpSelectionMerge %151 None
               OpBranchConditional %149 %150 %151
        %150 = OpLabel
        %152 = OpVectorShuffle %v2float %32 %32 0 1
        %153 = OpFOrdEqual %v2bool %132 %152
        %154 = OpAll %bool %153
               OpBranch %151
        %151 = OpLabel
        %155 = OpPhi %bool %false %148 %154 %150
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpVectorShuffle %v3float %32 %32 0 1 2
        %160 = OpFOrdEqual %v3bool %158 %159
        %161 = OpAll %bool %160
               OpBranch %157
        %157 = OpLabel
        %162 = OpPhi %bool %false %151 %161 %156
               OpSelectionMerge %164 None
               OpBranchConditional %162 %163 %164
        %163 = OpLabel
               OpBranch %164
        %164 = OpLabel
        %165 = OpPhi %bool %false %157 %true %163
               OpSelectionMerge %169 None
               OpBranchConditional %165 %167 %168
        %167 = OpLabel
        %170 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %172 = OpLoad %v4float %170
               OpStore %166 %172
               OpBranch %169
        %168 = OpLabel
        %173 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %175 = OpLoad %v4float %173
               OpStore %166 %175
               OpBranch %169
        %169 = OpLabel
        %176 = OpLoad %v4float %166
               OpReturnValue %176
               OpFunctionEnd
