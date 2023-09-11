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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
   %int_n100 = OpConstant %int -100
     %int_75 = OpConstant %int 75
    %int_100 = OpConstant %int 100
         %46 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
     %int_50 = OpConstant %int 50
    %int_225 = OpConstant %int 225
         %50 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %61 = OpConstantComposite %v2int %int_n100 %int_n100
         %62 = OpConstantComposite %v2int %int_100 %int_100
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %73 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
         %74 = OpConstantComposite %v3int %int_100 %int_100 %int_100
     %v3bool = OpTypeVector %bool 3
         %83 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
         %84 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %95 = OpConstantComposite %v2int %int_n100 %int_0
        %102 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
   %int_n200 = OpConstant %int -200
        %120 = OpConstantComposite %v2int %int_n100 %int_n200
    %int_200 = OpConstant %int 200
        %122 = OpConstantComposite %v2int %int_100 %int_200
        %131 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
        %132 = OpConstantComposite %v3int %int_100 %int_200 %int_50
        %140 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
    %int_300 = OpConstant %int 300
        %142 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
        %157 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
  %intValues = OpVariable %_ptr_Function_v4int Function
  %expectedA = OpVariable %_ptr_Function_v4int Function
  %expectedB = OpVariable %_ptr_Function_v4int Function
        %165 = OpVariable %_ptr_Function_v4float Function
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
               OpStore %expectedA %46
               OpStore %expectedB %50
         %54 = OpCompositeExtract %int %41 0
         %53 = OpExtInst %int %1 SClamp %54 %int_n100 %int_100
         %55 = OpIEqual %bool %53 %int_n100
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %59 = OpVectorShuffle %v2int %41 %41 0 1
         %58 = OpExtInst %v2int %1 SClamp %59 %61 %62
         %63 = OpVectorShuffle %v2int %46 %46 0 1
         %64 = OpIEqual %v2bool %58 %63
         %66 = OpAll %bool %64
               OpBranch %57
         %57 = OpLabel
         %67 = OpPhi %bool %false %22 %66 %56
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpVectorShuffle %v3int %41 %41 0 1 2
         %70 = OpExtInst %v3int %1 SClamp %71 %73 %74
         %75 = OpVectorShuffle %v3int %46 %46 0 1 2
         %76 = OpIEqual %v3bool %70 %75
         %78 = OpAll %bool %76
               OpBranch %69
         %69 = OpLabel
         %79 = OpPhi %bool %false %57 %78 %68
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %82 = OpExtInst %v4int %1 SClamp %41 %83 %84
         %85 = OpIEqual %v4bool %82 %46
         %87 = OpAll %bool %85
               OpBranch %81
         %81 = OpLabel
         %88 = OpPhi %bool %false %69 %87 %80
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
               OpBranch %90
         %90 = OpLabel
         %92 = OpPhi %bool %false %81 %true %89
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %96 = OpVectorShuffle %v2int %46 %46 0 1
         %97 = OpIEqual %v2bool %95 %96
         %98 = OpAll %bool %97
               OpBranch %94
         %94 = OpLabel
         %99 = OpPhi %bool %false %90 %98 %93
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %103 = OpVectorShuffle %v3int %46 %46 0 1 2
        %104 = OpIEqual %v3bool %102 %103
        %105 = OpAll %bool %104
               OpBranch %101
        %101 = OpLabel
        %106 = OpPhi %bool %false %94 %105 %100
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
               OpBranch %108
        %108 = OpLabel
        %109 = OpPhi %bool %false %101 %true %107
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
        %112 = OpExtInst %int %1 SClamp %54 %int_n100 %int_100
        %113 = OpIEqual %bool %112 %int_n100
               OpBranch %111
        %111 = OpLabel
        %114 = OpPhi %bool %false %108 %113 %110
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %118 = OpVectorShuffle %v2int %41 %41 0 1
        %117 = OpExtInst %v2int %1 SClamp %118 %120 %122
        %123 = OpVectorShuffle %v2int %50 %50 0 1
        %124 = OpIEqual %v2bool %117 %123
        %125 = OpAll %bool %124
               OpBranch %116
        %116 = OpLabel
        %126 = OpPhi %bool %false %111 %125 %115
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpVectorShuffle %v3int %41 %41 0 1 2
        %129 = OpExtInst %v3int %1 SClamp %130 %131 %132
        %133 = OpVectorShuffle %v3int %50 %50 0 1 2
        %134 = OpIEqual %v3bool %129 %133
        %135 = OpAll %bool %134
               OpBranch %128
        %128 = OpLabel
        %136 = OpPhi %bool %false %116 %135 %127
               OpSelectionMerge %138 None
               OpBranchConditional %136 %137 %138
        %137 = OpLabel
        %139 = OpExtInst %v4int %1 SClamp %41 %140 %142
        %143 = OpIEqual %v4bool %139 %50
        %144 = OpAll %bool %143
               OpBranch %138
        %138 = OpLabel
        %145 = OpPhi %bool %false %128 %144 %137
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
               OpBranch %147
        %147 = OpLabel
        %148 = OpPhi %bool %false %138 %true %146
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
        %151 = OpVectorShuffle %v2int %50 %50 0 1
        %152 = OpIEqual %v2bool %95 %151
        %153 = OpAll %bool %152
               OpBranch %150
        %150 = OpLabel
        %154 = OpPhi %bool %false %147 %153 %149
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %158 = OpVectorShuffle %v3int %50 %50 0 1 2
        %159 = OpIEqual %v3bool %157 %158
        %160 = OpAll %bool %159
               OpBranch %156
        %156 = OpLabel
        %161 = OpPhi %bool %false %150 %160 %155
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %163
        %162 = OpLabel
               OpBranch %163
        %163 = OpLabel
        %164 = OpPhi %bool %false %156 %true %162
               OpSelectionMerge %169 None
               OpBranchConditional %164 %167 %168
        %167 = OpLabel
        %170 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %172 = OpLoad %v4float %170
               OpStore %165 %172
               OpBranch %169
        %168 = OpLabel
        %173 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %175 = OpLoad %v4float %173
               OpStore %165 %175
               OpBranch %169
        %169 = OpLabel
        %176 = OpLoad %v4float %165
               OpReturnValue %176
               OpFunctionEnd
