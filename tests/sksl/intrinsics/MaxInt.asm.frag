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
               OpName %intGreen "intGreen"
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
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
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
      %int_1 = OpConstant %int 1
     %int_50 = OpConstant %int 50
     %int_75 = OpConstant %int 75
    %int_225 = OpConstant %int 225
         %63 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
    %int_100 = OpConstant %int 100
         %66 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %76 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %87 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %96 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %113 = OpConstantComposite %v3int %int_50 %int_50 %int_75
        %156 = OpConstantComposite %v2int %int_0 %int_100
        %163 = OpConstantComposite %v3int %int_0 %int_100 %int_75
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %intGreen = OpVariable %_ptr_Function_v4int Function
  %expectedA = OpVariable %_ptr_Function_v4int Function
  %expectedB = OpVariable %_ptr_Function_v4int Function
        %171 = OpVariable %_ptr_Function_v4float Function
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
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %48 = OpLoad %v4float %46
         %49 = OpVectorTimesScalar %v4float %48 %float_100
         %50 = OpCompositeExtract %float %49 0
         %51 = OpConvertFToS %int %50
         %52 = OpCompositeExtract %float %49 1
         %53 = OpConvertFToS %int %52
         %54 = OpCompositeExtract %float %49 2
         %55 = OpConvertFToS %int %54
         %56 = OpCompositeExtract %float %49 3
         %57 = OpConvertFToS %int %56
         %58 = OpCompositeConstruct %v4int %51 %53 %55 %57
               OpStore %intGreen %58
               OpStore %expectedA %63
               OpStore %expectedB %66
         %69 = OpCompositeExtract %int %44 0
         %68 = OpExtInst %int %1 SMax %69 %int_50
         %70 = OpIEqual %bool %68 %int_50
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
         %74 = OpVectorShuffle %v2int %44 %44 0 1
         %73 = OpExtInst %v2int %1 SMax %74 %76
         %77 = OpVectorShuffle %v2int %63 %63 0 1
         %78 = OpIEqual %v2bool %73 %77
         %80 = OpAll %bool %78
               OpBranch %72
         %72 = OpLabel
         %81 = OpPhi %bool %false %25 %80 %71
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %85 = OpVectorShuffle %v3int %44 %44 0 1 2
         %84 = OpExtInst %v3int %1 SMax %85 %87
         %88 = OpVectorShuffle %v3int %63 %63 0 1 2
         %89 = OpIEqual %v3bool %84 %88
         %91 = OpAll %bool %89
               OpBranch %83
         %83 = OpLabel
         %92 = OpPhi %bool %false %72 %91 %82
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %95 = OpExtInst %v4int %1 SMax %44 %96
         %97 = OpIEqual %v4bool %95 %63
         %99 = OpAll %bool %97
               OpBranch %94
         %94 = OpLabel
        %100 = OpPhi %bool %false %83 %99 %93
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
               OpBranch %102
        %102 = OpLabel
        %104 = OpPhi %bool %false %94 %true %101
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %107 = OpVectorShuffle %v2int %63 %63 0 1
        %108 = OpIEqual %v2bool %76 %107
        %109 = OpAll %bool %108
               OpBranch %106
        %106 = OpLabel
        %110 = OpPhi %bool %false %102 %109 %105
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %114 = OpVectorShuffle %v3int %63 %63 0 1 2
        %115 = OpIEqual %v3bool %113 %114
        %116 = OpAll %bool %115
               OpBranch %112
        %112 = OpLabel
        %117 = OpPhi %bool %false %106 %116 %111
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
               OpBranch %119
        %119 = OpLabel
        %120 = OpPhi %bool %false %112 %true %118
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %124 = OpCompositeExtract %int %58 0
        %123 = OpExtInst %int %1 SMax %69 %124
        %125 = OpIEqual %bool %123 %int_0
               OpBranch %122
        %122 = OpLabel
        %126 = OpPhi %bool %false %119 %125 %121
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpVectorShuffle %v2int %44 %44 0 1
        %131 = OpVectorShuffle %v2int %58 %58 0 1
        %129 = OpExtInst %v2int %1 SMax %130 %131
        %132 = OpVectorShuffle %v2int %66 %66 0 1
        %133 = OpIEqual %v2bool %129 %132
        %134 = OpAll %bool %133
               OpBranch %128
        %128 = OpLabel
        %135 = OpPhi %bool %false %122 %134 %127
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %139 = OpVectorShuffle %v3int %44 %44 0 1 2
        %140 = OpVectorShuffle %v3int %58 %58 0 1 2
        %138 = OpExtInst %v3int %1 SMax %139 %140
        %141 = OpVectorShuffle %v3int %66 %66 0 1 2
        %142 = OpIEqual %v3bool %138 %141
        %143 = OpAll %bool %142
               OpBranch %137
        %137 = OpLabel
        %144 = OpPhi %bool %false %128 %143 %136
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpExtInst %v4int %1 SMax %44 %58
        %148 = OpIEqual %v4bool %147 %66
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
        %157 = OpVectorShuffle %v2int %66 %66 0 1
        %158 = OpIEqual %v2bool %156 %157
        %159 = OpAll %bool %158
               OpBranch %155
        %155 = OpLabel
        %160 = OpPhi %bool %false %152 %159 %154
               OpSelectionMerge %162 None
               OpBranchConditional %160 %161 %162
        %161 = OpLabel
        %164 = OpVectorShuffle %v3int %66 %66 0 1 2
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
        %176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %177 = OpLoad %v4float %176
               OpStore %171 %177
               OpBranch %175
        %174 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %180 = OpLoad %v4float %178
               OpStore %171 %180
               OpBranch %175
        %175 = OpLabel
        %181 = OpLoad %v4float %171
               OpReturnValue %181
               OpFunctionEnd
