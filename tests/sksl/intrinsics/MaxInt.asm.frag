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
               OpName %intValues "intValues"
               OpName %intGreen "intGreen"
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
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
     %int_50 = OpConstant %int 50
     %int_75 = OpConstant %int 75
    %int_225 = OpConstant %int 225
         %60 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
    %int_100 = OpConstant %int 100
         %63 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %74 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %85 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %94 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %111 = OpConstantComposite %v3int %int_50 %int_50 %int_75
        %154 = OpConstantComposite %v2int %int_0 %int_100
        %161 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
  %intValues = OpVariable %_ptr_Function_v4int Function
   %intGreen = OpVariable %_ptr_Function_v4int Function
  %expectedA = OpVariable %_ptr_Function_v4int Function
  %expectedB = OpVariable %_ptr_Function_v4int Function
        %169 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %27
         %32 = OpVectorTimesScalar %v4float %30 %float_100
         %33 = OpCompositeExtract %float %32 0
         %34 = OpConvertFToS %int %33
         %35 = OpCompositeExtract %float %32 1
         %36 = OpConvertFToS %int %35
         %37 = OpCompositeExtract %float %32 2
         %38 = OpConvertFToS %int %37
         %39 = OpCompositeExtract %float %32 3
         %40 = OpConvertFToS %int %39
         %41 = OpCompositeConstruct %v4int %34 %36 %38 %40
               OpStore %intValues %41
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %45 = OpLoad %v4float %43
         %46 = OpVectorTimesScalar %v4float %45 %float_100
         %47 = OpCompositeExtract %float %46 0
         %48 = OpConvertFToS %int %47
         %49 = OpCompositeExtract %float %46 1
         %50 = OpConvertFToS %int %49
         %51 = OpCompositeExtract %float %46 2
         %52 = OpConvertFToS %int %51
         %53 = OpCompositeExtract %float %46 3
         %54 = OpConvertFToS %int %53
         %55 = OpCompositeConstruct %v4int %48 %50 %52 %54
               OpStore %intGreen %55
               OpStore %expectedA %60
               OpStore %expectedB %63
         %67 = OpCompositeExtract %int %41 0
         %66 = OpExtInst %int %1 SMax %67 %int_50
         %68 = OpIEqual %bool %66 %int_50
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %72 = OpVectorShuffle %v2int %41 %41 0 1
         %71 = OpExtInst %v2int %1 SMax %72 %74
         %75 = OpVectorShuffle %v2int %60 %60 0 1
         %76 = OpIEqual %v2bool %71 %75
         %78 = OpAll %bool %76
               OpBranch %70
         %70 = OpLabel
         %79 = OpPhi %bool %false %22 %78 %69
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %83 = OpVectorShuffle %v3int %41 %41 0 1 2
         %82 = OpExtInst %v3int %1 SMax %83 %85
         %86 = OpVectorShuffle %v3int %60 %60 0 1 2
         %87 = OpIEqual %v3bool %82 %86
         %89 = OpAll %bool %87
               OpBranch %81
         %81 = OpLabel
         %90 = OpPhi %bool %false %70 %89 %80
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %93 = OpExtInst %v4int %1 SMax %41 %94
         %95 = OpIEqual %v4bool %93 %60
         %97 = OpAll %bool %95
               OpBranch %92
         %92 = OpLabel
         %98 = OpPhi %bool %false %81 %97 %91
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
               OpBranch %100
        %100 = OpLabel
        %102 = OpPhi %bool %false %92 %true %99
               OpSelectionMerge %104 None
               OpBranchConditional %102 %103 %104
        %103 = OpLabel
        %105 = OpVectorShuffle %v2int %60 %60 0 1
        %106 = OpIEqual %v2bool %74 %105
        %107 = OpAll %bool %106
               OpBranch %104
        %104 = OpLabel
        %108 = OpPhi %bool %false %100 %107 %103
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %112 = OpVectorShuffle %v3int %60 %60 0 1 2
        %113 = OpIEqual %v3bool %111 %112
        %114 = OpAll %bool %113
               OpBranch %110
        %110 = OpLabel
        %115 = OpPhi %bool %false %104 %114 %109
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
               OpBranch %117
        %117 = OpLabel
        %118 = OpPhi %bool %false %110 %true %116
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
        %122 = OpCompositeExtract %int %55 0
        %121 = OpExtInst %int %1 SMax %67 %122
        %123 = OpIEqual %bool %121 %int_0
               OpBranch %120
        %120 = OpLabel
        %124 = OpPhi %bool %false %117 %123 %119
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
        %128 = OpVectorShuffle %v2int %41 %41 0 1
        %129 = OpVectorShuffle %v2int %55 %55 0 1
        %127 = OpExtInst %v2int %1 SMax %128 %129
        %130 = OpVectorShuffle %v2int %63 %63 0 1
        %131 = OpIEqual %v2bool %127 %130
        %132 = OpAll %bool %131
               OpBranch %126
        %126 = OpLabel
        %133 = OpPhi %bool %false %120 %132 %125
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %137 = OpVectorShuffle %v3int %41 %41 0 1 2
        %138 = OpVectorShuffle %v3int %55 %55 0 1 2
        %136 = OpExtInst %v3int %1 SMax %137 %138
        %139 = OpVectorShuffle %v3int %63 %63 0 1 2
        %140 = OpIEqual %v3bool %136 %139
        %141 = OpAll %bool %140
               OpBranch %135
        %135 = OpLabel
        %142 = OpPhi %bool %false %126 %141 %134
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
        %145 = OpExtInst %v4int %1 SMax %41 %55
        %146 = OpIEqual %v4bool %145 %63
        %147 = OpAll %bool %146
               OpBranch %144
        %144 = OpLabel
        %148 = OpPhi %bool %false %135 %147 %143
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
               OpBranch %150
        %150 = OpLabel
        %151 = OpPhi %bool %false %144 %true %149
               OpSelectionMerge %153 None
               OpBranchConditional %151 %152 %153
        %152 = OpLabel
        %155 = OpVectorShuffle %v2int %63 %63 0 1
        %156 = OpIEqual %v2bool %154 %155
        %157 = OpAll %bool %156
               OpBranch %153
        %153 = OpLabel
        %158 = OpPhi %bool %false %150 %157 %152
               OpSelectionMerge %160 None
               OpBranchConditional %158 %159 %160
        %159 = OpLabel
        %162 = OpVectorShuffle %v3int %63 %63 0 1 2
        %163 = OpIEqual %v3bool %161 %162
        %164 = OpAll %bool %163
               OpBranch %160
        %160 = OpLabel
        %165 = OpPhi %bool %false %153 %164 %159
               OpSelectionMerge %167 None
               OpBranchConditional %165 %166 %167
        %166 = OpLabel
               OpBranch %167
        %167 = OpLabel
        %168 = OpPhi %bool %false %160 %true %166
               OpSelectionMerge %173 None
               OpBranchConditional %168 %171 %172
        %171 = OpLabel
        %174 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %175 = OpLoad %v4float %174
               OpStore %169 %175
               OpBranch %173
        %172 = OpLabel
        %176 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %178 = OpLoad %v4float %176
               OpStore %169 %178
               OpBranch %173
        %173 = OpLabel
        %179 = OpLoad %v4float %169
               OpReturnValue %179
               OpFunctionEnd
