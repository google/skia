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
               OpName %uintValues "uintValues"
               OpName %uintGreen "uintGreen"
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
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
    %uint_50 = OpConstant %uint 50
     %uint_0 = OpConstant %uint 0
         %61 = OpConstantComposite %v4uint %uint_50 %uint_0 %uint_50 %uint_50
   %uint_100 = OpConstant %uint 100
         %64 = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_100
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %75 = OpConstantComposite %v2uint %uint_50 %uint_50
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %86 = OpConstantComposite %v3uint %uint_50 %uint_50 %uint_50
     %v3bool = OpTypeVector %bool 3
         %95 = OpConstantComposite %v4uint %uint_50 %uint_50 %uint_50 %uint_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %106 = OpConstantComposite %v2uint %uint_50 %uint_0
        %113 = OpConstantComposite %v3uint %uint_50 %uint_0 %uint_50
        %156 = OpConstantComposite %v2uint %uint_0 %uint_0
        %163 = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
 %uintValues = OpVariable %_ptr_Function_v4uint Function
  %uintGreen = OpVariable %_ptr_Function_v4uint Function
  %expectedA = OpVariable %_ptr_Function_v4uint Function
  %expectedB = OpVariable %_ptr_Function_v4uint Function
        %171 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 = OpLoad %v4float %28
         %27 = OpExtInst %v4float %1 FAbs %32
         %34 = OpVectorTimesScalar %v4float %27 %float_100
         %35 = OpCompositeExtract %float %34 0
         %36 = OpConvertFToU %uint %35
         %37 = OpCompositeExtract %float %34 1
         %38 = OpConvertFToU %uint %37
         %39 = OpCompositeExtract %float %34 2
         %40 = OpConvertFToU %uint %39
         %41 = OpCompositeExtract %float %34 3
         %42 = OpConvertFToU %uint %41
         %43 = OpCompositeConstruct %v4uint %36 %38 %40 %42
               OpStore %uintValues %43
         %45 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %47 = OpLoad %v4float %45
         %48 = OpVectorTimesScalar %v4float %47 %float_100
         %49 = OpCompositeExtract %float %48 0
         %50 = OpConvertFToU %uint %49
         %51 = OpCompositeExtract %float %48 1
         %52 = OpConvertFToU %uint %51
         %53 = OpCompositeExtract %float %48 2
         %54 = OpConvertFToU %uint %53
         %55 = OpCompositeExtract %float %48 3
         %56 = OpConvertFToU %uint %55
         %57 = OpCompositeConstruct %v4uint %50 %52 %54 %56
               OpStore %uintGreen %57
               OpStore %expectedA %61
               OpStore %expectedB %64
         %68 = OpCompositeExtract %uint %43 0
         %67 = OpExtInst %uint %1 UMin %68 %uint_50
         %69 = OpIEqual %bool %67 %uint_50
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %73 = OpVectorShuffle %v2uint %43 %43 0 1
         %72 = OpExtInst %v2uint %1 UMin %73 %75
         %76 = OpVectorShuffle %v2uint %61 %61 0 1
         %77 = OpIEqual %v2bool %72 %76
         %79 = OpAll %bool %77
               OpBranch %71
         %71 = OpLabel
         %80 = OpPhi %bool %false %22 %79 %70
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %84 = OpVectorShuffle %v3uint %43 %43 0 1 2
         %83 = OpExtInst %v3uint %1 UMin %84 %86
         %87 = OpVectorShuffle %v3uint %61 %61 0 1 2
         %88 = OpIEqual %v3bool %83 %87
         %90 = OpAll %bool %88
               OpBranch %82
         %82 = OpLabel
         %91 = OpPhi %bool %false %71 %90 %81
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %94 = OpExtInst %v4uint %1 UMin %43 %95
         %96 = OpIEqual %v4bool %94 %61
         %98 = OpAll %bool %96
               OpBranch %93
         %93 = OpLabel
         %99 = OpPhi %bool %false %82 %98 %92
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %103 = OpPhi %bool %false %93 %true %100
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %107 = OpVectorShuffle %v2uint %61 %61 0 1
        %108 = OpIEqual %v2bool %106 %107
        %109 = OpAll %bool %108
               OpBranch %105
        %105 = OpLabel
        %110 = OpPhi %bool %false %101 %109 %104
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %114 = OpVectorShuffle %v3uint %61 %61 0 1 2
        %115 = OpIEqual %v3bool %113 %114
        %116 = OpAll %bool %115
               OpBranch %112
        %112 = OpLabel
        %117 = OpPhi %bool %false %105 %116 %111
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
               OpBranch %119
        %119 = OpLabel
        %120 = OpPhi %bool %false %112 %true %118
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %124 = OpCompositeExtract %uint %57 0
        %123 = OpExtInst %uint %1 UMin %68 %124
        %125 = OpIEqual %bool %123 %uint_0
               OpBranch %122
        %122 = OpLabel
        %126 = OpPhi %bool %false %119 %125 %121
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpVectorShuffle %v2uint %43 %43 0 1
        %131 = OpVectorShuffle %v2uint %57 %57 0 1
        %129 = OpExtInst %v2uint %1 UMin %130 %131
        %132 = OpVectorShuffle %v2uint %64 %64 0 1
        %133 = OpIEqual %v2bool %129 %132
        %134 = OpAll %bool %133
               OpBranch %128
        %128 = OpLabel
        %135 = OpPhi %bool %false %122 %134 %127
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %139 = OpVectorShuffle %v3uint %43 %43 0 1 2
        %140 = OpVectorShuffle %v3uint %57 %57 0 1 2
        %138 = OpExtInst %v3uint %1 UMin %139 %140
        %141 = OpVectorShuffle %v3uint %64 %64 0 1 2
        %142 = OpIEqual %v3bool %138 %141
        %143 = OpAll %bool %142
               OpBranch %137
        %137 = OpLabel
        %144 = OpPhi %bool %false %128 %143 %136
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpExtInst %v4uint %1 UMin %43 %57
        %148 = OpIEqual %v4bool %147 %64
        %149 = OpAll %bool %148
               OpBranch %146
        %146 = OpLabel
        %150 = OpPhi %bool %false %137 %149 %145
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
               OpBranch %152
        %152 = OpLabel
        %153 = OpPhi %bool %false %146 %true %151
               OpSelectionMerge %155 None
               OpBranchConditional %153 %154 %155
        %154 = OpLabel
        %157 = OpVectorShuffle %v2uint %64 %64 0 1
        %158 = OpIEqual %v2bool %156 %157
        %159 = OpAll %bool %158
               OpBranch %155
        %155 = OpLabel
        %160 = OpPhi %bool %false %152 %159 %154
               OpSelectionMerge %162 None
               OpBranchConditional %160 %161 %162
        %161 = OpLabel
        %164 = OpVectorShuffle %v3uint %64 %64 0 1 2
        %165 = OpIEqual %v3bool %163 %164
        %166 = OpAll %bool %165
               OpBranch %162
        %162 = OpLabel
        %167 = OpPhi %bool %false %155 %166 %161
               OpSelectionMerge %169 None
               OpBranchConditional %167 %168 %169
        %168 = OpLabel
               OpBranch %169
        %169 = OpLabel
        %170 = OpPhi %bool %false %162 %true %168
               OpSelectionMerge %175 None
               OpBranchConditional %170 %173 %174
        %173 = OpLabel
        %176 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %177 = OpLoad %v4float %176
               OpStore %171 %177
               OpBranch %175
        %174 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %180 = OpLoad %v4float %178
               OpStore %171 %180
               OpBranch %175
        %175 = OpLabel
        %181 = OpLoad %v4float %171
               OpReturnValue %181
               OpFunctionEnd
