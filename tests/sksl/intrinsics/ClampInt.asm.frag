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
               OpName %intValues "intValues"
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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
   %int_n100 = OpConstant %int -100
     %int_75 = OpConstant %int 75
    %int_100 = OpConstant %int 100
         %49 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
     %int_50 = OpConstant %int 50
    %int_225 = OpConstant %int 225
         %53 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %63 = OpConstantComposite %v2int %int_n100 %int_n100
         %64 = OpConstantComposite %v2int %int_100 %int_100
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %75 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
         %76 = OpConstantComposite %v3int %int_100 %int_100 %int_100
     %v3bool = OpTypeVector %bool 3
         %85 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
         %86 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %97 = OpConstantComposite %v2int %int_n100 %int_0
        %104 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
   %int_n200 = OpConstant %int -200
        %122 = OpConstantComposite %v2int %int_n100 %int_n200
    %int_200 = OpConstant %int 200
        %124 = OpConstantComposite %v2int %int_100 %int_200
        %133 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
        %134 = OpConstantComposite %v3int %int_100 %int_200 %int_50
        %142 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
    %int_300 = OpConstant %int 300
        %144 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
        %159 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
  %intValues = OpVariable %_ptr_Function_v4int Function
  %expectedA = OpVariable %_ptr_Function_v4int Function
  %expectedB = OpVariable %_ptr_Function_v4int Function
        %167 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %30
         %35 = OpVectorTimesScalar %v4float %33 %float_100
         %36 = OpCompositeExtract %float %35 0
         %37 = OpConvertFToS %int %36
         %38 = OpCompositeExtract %float %35 1
         %39 = OpConvertFToS %int %38
         %40 = OpCompositeExtract %float %35 2
         %41 = OpConvertFToS %int %40
         %42 = OpCompositeExtract %float %35 3
         %43 = OpConvertFToS %int %42
         %44 = OpCompositeConstruct %v4int %37 %39 %41 %43
               OpStore %intValues %44
               OpStore %expectedA %49
               OpStore %expectedB %53
         %56 = OpCompositeExtract %int %44 0
         %55 = OpExtInst %int %1 SClamp %56 %int_n100 %int_100
         %57 = OpIEqual %bool %55 %int_n100
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %61 = OpVectorShuffle %v2int %44 %44 0 1
         %60 = OpExtInst %v2int %1 SClamp %61 %63 %64
         %65 = OpVectorShuffle %v2int %49 %49 0 1
         %66 = OpIEqual %v2bool %60 %65
         %68 = OpAll %bool %66
               OpBranch %59
         %59 = OpLabel
         %69 = OpPhi %bool %false %25 %68 %58
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %73 = OpVectorShuffle %v3int %44 %44 0 1 2
         %72 = OpExtInst %v3int %1 SClamp %73 %75 %76
         %77 = OpVectorShuffle %v3int %49 %49 0 1 2
         %78 = OpIEqual %v3bool %72 %77
         %80 = OpAll %bool %78
               OpBranch %71
         %71 = OpLabel
         %81 = OpPhi %bool %false %59 %80 %70
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %84 = OpExtInst %v4int %1 SClamp %44 %85 %86
         %87 = OpIEqual %v4bool %84 %49
         %89 = OpAll %bool %87
               OpBranch %83
         %83 = OpLabel
         %90 = OpPhi %bool %false %71 %89 %82
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
               OpBranch %92
         %92 = OpLabel
         %94 = OpPhi %bool %false %83 %true %91
               OpSelectionMerge %96 None
               OpBranchConditional %94 %95 %96
         %95 = OpLabel
         %98 = OpVectorShuffle %v2int %49 %49 0 1
         %99 = OpIEqual %v2bool %97 %98
        %100 = OpAll %bool %99
               OpBranch %96
         %96 = OpLabel
        %101 = OpPhi %bool %false %92 %100 %95
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %105 = OpVectorShuffle %v3int %49 %49 0 1 2
        %106 = OpIEqual %v3bool %104 %105
        %107 = OpAll %bool %106
               OpBranch %103
        %103 = OpLabel
        %108 = OpPhi %bool %false %96 %107 %102
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
               OpBranch %110
        %110 = OpLabel
        %111 = OpPhi %bool %false %103 %true %109
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
        %114 = OpExtInst %int %1 SClamp %56 %int_n100 %int_100
        %115 = OpIEqual %bool %114 %int_n100
               OpBranch %113
        %113 = OpLabel
        %116 = OpPhi %bool %false %110 %115 %112
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %120 = OpVectorShuffle %v2int %44 %44 0 1
        %119 = OpExtInst %v2int %1 SClamp %120 %122 %124
        %125 = OpVectorShuffle %v2int %53 %53 0 1
        %126 = OpIEqual %v2bool %119 %125
        %127 = OpAll %bool %126
               OpBranch %118
        %118 = OpLabel
        %128 = OpPhi %bool %false %113 %127 %117
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpVectorShuffle %v3int %44 %44 0 1 2
        %131 = OpExtInst %v3int %1 SClamp %132 %133 %134
        %135 = OpVectorShuffle %v3int %53 %53 0 1 2
        %136 = OpIEqual %v3bool %131 %135
        %137 = OpAll %bool %136
               OpBranch %130
        %130 = OpLabel
        %138 = OpPhi %bool %false %118 %137 %129
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
        %141 = OpExtInst %v4int %1 SClamp %44 %142 %144
        %145 = OpIEqual %v4bool %141 %53
        %146 = OpAll %bool %145
               OpBranch %140
        %140 = OpLabel
        %147 = OpPhi %bool %false %130 %146 %139
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
               OpBranch %149
        %149 = OpLabel
        %150 = OpPhi %bool %false %140 %true %148
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
        %153 = OpVectorShuffle %v2int %53 %53 0 1
        %154 = OpIEqual %v2bool %97 %153
        %155 = OpAll %bool %154
               OpBranch %152
        %152 = OpLabel
        %156 = OpPhi %bool %false %149 %155 %151
               OpSelectionMerge %158 None
               OpBranchConditional %156 %157 %158
        %157 = OpLabel
        %160 = OpVectorShuffle %v3int %53 %53 0 1 2
        %161 = OpIEqual %v3bool %159 %160
        %162 = OpAll %bool %161
               OpBranch %158
        %158 = OpLabel
        %163 = OpPhi %bool %false %152 %162 %157
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
               OpBranch %165
        %165 = OpLabel
        %166 = OpPhi %bool %false %158 %true %164
               OpSelectionMerge %171 None
               OpBranchConditional %166 %169 %170
        %169 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %174 = OpLoad %v4float %172
               OpStore %167 %174
               OpBranch %171
        %170 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %177 = OpLoad %v4float %175
               OpStore %167 %177
               OpBranch %171
        %171 = OpLabel
        %178 = OpLoad %v4float %167
               OpReturnValue %178
               OpFunctionEnd
