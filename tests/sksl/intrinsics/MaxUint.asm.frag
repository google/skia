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
               OpDecorate %179 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
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
   %uint_125 = OpConstant %uint 125
    %uint_80 = OpConstant %uint 80
   %uint_225 = OpConstant %uint 225
         %62 = OpConstantComposite %v4uint %uint_125 %uint_80 %uint_80 %uint_225
   %uint_100 = OpConstant %uint 100
    %uint_75 = OpConstant %uint 75
         %66 = OpConstantComposite %v4uint %uint_125 %uint_100 %uint_75 %uint_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %77 = OpConstantComposite %v2uint %uint_80 %uint_80
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %88 = OpConstantComposite %v3uint %uint_80 %uint_80 %uint_80
     %v3bool = OpTypeVector %bool 3
         %97 = OpConstantComposite %v4uint %uint_80 %uint_80 %uint_80 %uint_80
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %108 = OpConstantComposite %v2uint %uint_125 %uint_80
        %115 = OpConstantComposite %v3uint %uint_125 %uint_80 %uint_80
        %158 = OpConstantComposite %v2uint %uint_125 %uint_100
        %165 = OpConstantComposite %v3uint %uint_125 %uint_100 %uint_75
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
        %173 = OpVariable %_ptr_Function_v4float Function
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
               OpStore %expectedA %62
               OpStore %expectedB %66
         %70 = OpCompositeExtract %uint %43 0
         %69 = OpExtInst %uint %1 UMax %70 %uint_80
         %71 = OpIEqual %bool %69 %uint_125
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %75 = OpVectorShuffle %v2uint %43 %43 0 1
         %74 = OpExtInst %v2uint %1 UMax %75 %77
         %78 = OpVectorShuffle %v2uint %62 %62 0 1
         %79 = OpIEqual %v2bool %74 %78
         %81 = OpAll %bool %79
               OpBranch %73
         %73 = OpLabel
         %82 = OpPhi %bool %false %22 %81 %72
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %86 = OpVectorShuffle %v3uint %43 %43 0 1 2
         %85 = OpExtInst %v3uint %1 UMax %86 %88
         %89 = OpVectorShuffle %v3uint %62 %62 0 1 2
         %90 = OpIEqual %v3bool %85 %89
         %92 = OpAll %bool %90
               OpBranch %84
         %84 = OpLabel
         %93 = OpPhi %bool %false %73 %92 %83
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %96 = OpExtInst %v4uint %1 UMax %43 %97
         %98 = OpIEqual %v4bool %96 %62
        %100 = OpAll %bool %98
               OpBranch %95
         %95 = OpLabel
        %101 = OpPhi %bool %false %84 %100 %94
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %105 = OpPhi %bool %false %95 %true %102
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %109 = OpVectorShuffle %v2uint %62 %62 0 1
        %110 = OpIEqual %v2bool %108 %109
        %111 = OpAll %bool %110
               OpBranch %107
        %107 = OpLabel
        %112 = OpPhi %bool %false %103 %111 %106
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %116 = OpVectorShuffle %v3uint %62 %62 0 1 2
        %117 = OpIEqual %v3bool %115 %116
        %118 = OpAll %bool %117
               OpBranch %114
        %114 = OpLabel
        %119 = OpPhi %bool %false %107 %118 %113
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %121
        %120 = OpLabel
               OpBranch %121
        %121 = OpLabel
        %122 = OpPhi %bool %false %114 %true %120
               OpSelectionMerge %124 None
               OpBranchConditional %122 %123 %124
        %123 = OpLabel
        %126 = OpCompositeExtract %uint %57 0
        %125 = OpExtInst %uint %1 UMax %70 %126
        %127 = OpIEqual %bool %125 %uint_125
               OpBranch %124
        %124 = OpLabel
        %128 = OpPhi %bool %false %121 %127 %123
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpVectorShuffle %v2uint %43 %43 0 1
        %133 = OpVectorShuffle %v2uint %57 %57 0 1
        %131 = OpExtInst %v2uint %1 UMax %132 %133
        %134 = OpVectorShuffle %v2uint %66 %66 0 1
        %135 = OpIEqual %v2bool %131 %134
        %136 = OpAll %bool %135
               OpBranch %130
        %130 = OpLabel
        %137 = OpPhi %bool %false %124 %136 %129
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
        %141 = OpVectorShuffle %v3uint %43 %43 0 1 2
        %142 = OpVectorShuffle %v3uint %57 %57 0 1 2
        %140 = OpExtInst %v3uint %1 UMax %141 %142
        %143 = OpVectorShuffle %v3uint %66 %66 0 1 2
        %144 = OpIEqual %v3bool %140 %143
        %145 = OpAll %bool %144
               OpBranch %139
        %139 = OpLabel
        %146 = OpPhi %bool %false %130 %145 %138
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpExtInst %v4uint %1 UMax %43 %57
        %150 = OpIEqual %v4bool %149 %66
        %151 = OpAll %bool %150
               OpBranch %148
        %148 = OpLabel
        %152 = OpPhi %bool %false %139 %151 %147
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %bool %false %148 %true %153
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpVectorShuffle %v2uint %66 %66 0 1
        %160 = OpIEqual %v2bool %158 %159
        %161 = OpAll %bool %160
               OpBranch %157
        %157 = OpLabel
        %162 = OpPhi %bool %false %154 %161 %156
               OpSelectionMerge %164 None
               OpBranchConditional %162 %163 %164
        %163 = OpLabel
        %166 = OpVectorShuffle %v3uint %66 %66 0 1 2
        %167 = OpIEqual %v3bool %165 %166
        %168 = OpAll %bool %167
               OpBranch %164
        %164 = OpLabel
        %169 = OpPhi %bool %false %157 %168 %163
               OpSelectionMerge %171 None
               OpBranchConditional %169 %170 %171
        %170 = OpLabel
               OpBranch %171
        %171 = OpLabel
        %172 = OpPhi %bool %false %164 %true %170
               OpSelectionMerge %177 None
               OpBranchConditional %172 %175 %176
        %175 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %179 = OpLoad %v4float %178
               OpStore %173 %179
               OpBranch %177
        %176 = OpLabel
        %180 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %182 = OpLoad %v4float %180
               OpStore %173 %182
               OpBranch %177
        %177 = OpLabel
        %183 = OpLoad %v4float %173
               OpReturnValue %183
               OpFunctionEnd
