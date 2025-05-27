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
               OpDecorate %174 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
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
   %int_n125 = OpConstant %int -125
     %int_50 = OpConstant %int 50
         %59 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
    %int_100 = OpConstant %int 100
         %62 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %73 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %84 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %93 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %104 = OpConstantComposite %v2int %int_n125 %int_0
        %111 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
        %160 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
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
        %168 = OpVariable %_ptr_Function_v4float Function
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
               OpStore %expectedA %59
               OpStore %expectedB %62
         %66 = OpCompositeExtract %int %41 0
         %65 = OpExtInst %int %1 SMin %66 %int_50
         %67 = OpIEqual %bool %65 %int_n125
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %71 = OpVectorShuffle %v2int %41 %41 0 1
         %70 = OpExtInst %v2int %1 SMin %71 %73
         %74 = OpVectorShuffle %v2int %59 %59 0 1
         %75 = OpIEqual %v2bool %70 %74
         %77 = OpAll %bool %75
               OpBranch %69
         %69 = OpLabel
         %78 = OpPhi %bool %false %22 %77 %68
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %82 = OpVectorShuffle %v3int %41 %41 0 1 2
         %81 = OpExtInst %v3int %1 SMin %82 %84
         %85 = OpVectorShuffle %v3int %59 %59 0 1 2
         %86 = OpIEqual %v3bool %81 %85
         %88 = OpAll %bool %86
               OpBranch %80
         %80 = OpLabel
         %89 = OpPhi %bool %false %69 %88 %79
               OpSelectionMerge %91 None
               OpBranchConditional %89 %90 %91
         %90 = OpLabel
         %92 = OpExtInst %v4int %1 SMin %41 %93
         %94 = OpIEqual %v4bool %92 %59
         %96 = OpAll %bool %94
               OpBranch %91
         %91 = OpLabel
         %97 = OpPhi %bool %false %80 %96 %90
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
               OpBranch %99
         %99 = OpLabel
        %101 = OpPhi %bool %false %91 %true %98
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %105 = OpVectorShuffle %v2int %59 %59 0 1
        %106 = OpIEqual %v2bool %104 %105
        %107 = OpAll %bool %106
               OpBranch %103
        %103 = OpLabel
        %108 = OpPhi %bool %false %99 %107 %102
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %112 = OpVectorShuffle %v3int %59 %59 0 1 2
        %113 = OpIEqual %v3bool %111 %112
        %114 = OpAll %bool %113
               OpBranch %110
        %110 = OpLabel
        %115 = OpPhi %bool %false %103 %114 %109
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
        %121 = OpExtInst %int %1 SMin %66 %122
        %123 = OpIEqual %bool %121 %int_n125
               OpBranch %120
        %120 = OpLabel
        %124 = OpPhi %bool %false %117 %123 %119
               OpSelectionMerge %126 None
               OpBranchConditional %124 %125 %126
        %125 = OpLabel
        %128 = OpVectorShuffle %v2int %41 %41 0 1
        %129 = OpVectorShuffle %v2int %55 %55 0 1
        %127 = OpExtInst %v2int %1 SMin %128 %129
        %130 = OpVectorShuffle %v2int %62 %62 0 1
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
        %136 = OpExtInst %v3int %1 SMin %137 %138
        %139 = OpVectorShuffle %v3int %62 %62 0 1 2
        %140 = OpIEqual %v3bool %136 %139
        %141 = OpAll %bool %140
               OpBranch %135
        %135 = OpLabel
        %142 = OpPhi %bool %false %126 %141 %134
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
        %145 = OpExtInst %v4int %1 SMin %41 %55
        %146 = OpIEqual %v4bool %145 %62
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
        %154 = OpVectorShuffle %v2int %62 %62 0 1
        %155 = OpIEqual %v2bool %104 %154
        %156 = OpAll %bool %155
               OpBranch %153
        %153 = OpLabel
        %157 = OpPhi %bool %false %150 %156 %152
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %161 = OpVectorShuffle %v3int %62 %62 0 1 2
        %162 = OpIEqual %v3bool %160 %161
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
               OpSelectionMerge %172 None
               OpBranchConditional %167 %170 %171
        %170 = OpLabel
        %173 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %174 = OpLoad %v4float %173
               OpStore %168 %174
               OpBranch %172
        %171 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %177 = OpLoad %v4float %175
               OpStore %168 %177
               OpBranch %172
        %172 = OpLabel
        %178 = OpLoad %v4float %168
               OpReturnValue %178
               OpFunctionEnd
