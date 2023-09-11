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
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
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
  %float_200 = OpConstant %float 200
         %35 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
   %uint_100 = OpConstant %uint 100
   %uint_200 = OpConstant %uint 200
   %uint_275 = OpConstant %uint 275
   %uint_300 = OpConstant %uint 300
         %51 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
   %uint_250 = OpConstant %uint 250
   %uint_425 = OpConstant %uint 425
         %55 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %66 = OpConstantComposite %v2uint %uint_100 %uint_100
         %67 = OpConstantComposite %v2uint %uint_300 %uint_300
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %78 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
         %79 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
     %v3bool = OpTypeVector %bool 3
         %88 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
         %89 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %100 = OpConstantComposite %v2uint %uint_100 %uint_200
        %107 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
     %uint_0 = OpConstant %uint 0
        %125 = OpConstantComposite %v2uint %uint_100 %uint_0
   %uint_400 = OpConstant %uint 400
        %127 = OpConstantComposite %v2uint %uint_300 %uint_400
        %136 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
        %137 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
        %145 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
   %uint_500 = OpConstant %uint 500
        %147 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
        %162 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
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
 %uintValues = OpVariable %_ptr_Function_v4uint Function
  %expectedA = OpVariable %_ptr_Function_v4uint Function
  %expectedB = OpVariable %_ptr_Function_v4uint Function
        %170 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 = OpLoad %v4float %27
         %33 = OpVectorTimesScalar %v4float %31 %float_100
         %36 = OpFAdd %v4float %33 %35
         %37 = OpCompositeExtract %float %36 0
         %38 = OpConvertFToU %uint %37
         %39 = OpCompositeExtract %float %36 1
         %40 = OpConvertFToU %uint %39
         %41 = OpCompositeExtract %float %36 2
         %42 = OpConvertFToU %uint %41
         %43 = OpCompositeExtract %float %36 3
         %44 = OpConvertFToU %uint %43
         %45 = OpCompositeConstruct %v4uint %38 %40 %42 %44
               OpStore %uintValues %45
               OpStore %expectedA %51
               OpStore %expectedB %55
         %59 = OpCompositeExtract %uint %45 0
         %58 = OpExtInst %uint %1 UClamp %59 %uint_100 %uint_300
         %60 = OpIEqual %bool %58 %uint_100
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpVectorShuffle %v2uint %45 %45 0 1
         %63 = OpExtInst %v2uint %1 UClamp %64 %66 %67
         %68 = OpVectorShuffle %v2uint %51 %51 0 1
         %69 = OpIEqual %v2bool %63 %68
         %71 = OpAll %bool %69
               OpBranch %62
         %62 = OpLabel
         %72 = OpPhi %bool %false %22 %71 %61
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpVectorShuffle %v3uint %45 %45 0 1 2
         %75 = OpExtInst %v3uint %1 UClamp %76 %78 %79
         %80 = OpVectorShuffle %v3uint %51 %51 0 1 2
         %81 = OpIEqual %v3bool %75 %80
         %83 = OpAll %bool %81
               OpBranch %74
         %74 = OpLabel
         %84 = OpPhi %bool %false %62 %83 %73
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %87 = OpExtInst %v4uint %1 UClamp %45 %88 %89
         %90 = OpIEqual %v4bool %87 %51
         %92 = OpAll %bool %90
               OpBranch %86
         %86 = OpLabel
         %93 = OpPhi %bool %false %74 %92 %85
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
               OpBranch %95
         %95 = OpLabel
         %97 = OpPhi %bool %false %86 %true %94
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
        %101 = OpVectorShuffle %v2uint %51 %51 0 1
        %102 = OpIEqual %v2bool %100 %101
        %103 = OpAll %bool %102
               OpBranch %99
         %99 = OpLabel
        %104 = OpPhi %bool %false %95 %103 %98
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %108 = OpVectorShuffle %v3uint %51 %51 0 1 2
        %109 = OpIEqual %v3bool %107 %108
        %110 = OpAll %bool %109
               OpBranch %106
        %106 = OpLabel
        %111 = OpPhi %bool %false %99 %110 %105
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
               OpBranch %113
        %113 = OpLabel
        %114 = OpPhi %bool %false %106 %true %112
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %117 = OpExtInst %uint %1 UClamp %59 %uint_100 %uint_300
        %118 = OpIEqual %bool %117 %uint_100
               OpBranch %116
        %116 = OpLabel
        %119 = OpPhi %bool %false %113 %118 %115
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %121
        %120 = OpLabel
        %123 = OpVectorShuffle %v2uint %45 %45 0 1
        %122 = OpExtInst %v2uint %1 UClamp %123 %125 %127
        %128 = OpVectorShuffle %v2uint %55 %55 0 1
        %129 = OpIEqual %v2bool %122 %128
        %130 = OpAll %bool %129
               OpBranch %121
        %121 = OpLabel
        %131 = OpPhi %bool %false %116 %130 %120
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %135 = OpVectorShuffle %v3uint %45 %45 0 1 2
        %134 = OpExtInst %v3uint %1 UClamp %135 %136 %137
        %138 = OpVectorShuffle %v3uint %55 %55 0 1 2
        %139 = OpIEqual %v3bool %134 %138
        %140 = OpAll %bool %139
               OpBranch %133
        %133 = OpLabel
        %141 = OpPhi %bool %false %121 %140 %132
               OpSelectionMerge %143 None
               OpBranchConditional %141 %142 %143
        %142 = OpLabel
        %144 = OpExtInst %v4uint %1 UClamp %45 %145 %147
        %148 = OpIEqual %v4bool %144 %55
        %149 = OpAll %bool %148
               OpBranch %143
        %143 = OpLabel
        %150 = OpPhi %bool %false %133 %149 %142
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
               OpBranch %152
        %152 = OpLabel
        %153 = OpPhi %bool %false %143 %true %151
               OpSelectionMerge %155 None
               OpBranchConditional %153 %154 %155
        %154 = OpLabel
        %156 = OpVectorShuffle %v2uint %55 %55 0 1
        %157 = OpIEqual %v2bool %100 %156
        %158 = OpAll %bool %157
               OpBranch %155
        %155 = OpLabel
        %159 = OpPhi %bool %false %152 %158 %154
               OpSelectionMerge %161 None
               OpBranchConditional %159 %160 %161
        %160 = OpLabel
        %163 = OpVectorShuffle %v3uint %55 %55 0 1 2
        %164 = OpIEqual %v3bool %162 %163
        %165 = OpAll %bool %164
               OpBranch %161
        %161 = OpLabel
        %166 = OpPhi %bool %false %155 %165 %160
               OpSelectionMerge %168 None
               OpBranchConditional %166 %167 %168
        %167 = OpLabel
               OpBranch %168
        %168 = OpLabel
        %169 = OpPhi %bool %false %161 %true %167
               OpSelectionMerge %174 None
               OpBranchConditional %169 %172 %173
        %172 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %177 = OpLoad %v4float %175
               OpStore %170 %177
               OpBranch %174
        %173 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %180 = OpLoad %v4float %178
               OpStore %170 %180
               OpBranch %174
        %174 = OpLabel
        %181 = OpLoad %v4float %170
               OpReturnValue %181
               OpFunctionEnd
