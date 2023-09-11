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
               OpName %uintValues "uintValues"
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
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
  %float_200 = OpConstant %float 200
         %38 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
   %uint_100 = OpConstant %uint 100
   %uint_200 = OpConstant %uint 200
   %uint_275 = OpConstant %uint 275
   %uint_300 = OpConstant %uint 300
         %54 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
   %uint_250 = OpConstant %uint 250
   %uint_425 = OpConstant %uint 425
         %58 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %68 = OpConstantComposite %v2uint %uint_100 %uint_100
         %69 = OpConstantComposite %v2uint %uint_300 %uint_300
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %80 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
         %81 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
     %v3bool = OpTypeVector %bool 3
         %90 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
         %91 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %102 = OpConstantComposite %v2uint %uint_100 %uint_200
        %109 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
     %uint_0 = OpConstant %uint 0
        %127 = OpConstantComposite %v2uint %uint_100 %uint_0
   %uint_400 = OpConstant %uint 400
        %129 = OpConstantComposite %v2uint %uint_300 %uint_400
        %138 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
        %139 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
        %147 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
   %uint_500 = OpConstant %uint 500
        %149 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
        %164 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
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
 %uintValues = OpVariable %_ptr_Function_v4uint Function
  %expectedA = OpVariable %_ptr_Function_v4uint Function
  %expectedB = OpVariable %_ptr_Function_v4uint Function
        %172 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %34 = OpLoad %v4float %30
         %36 = OpVectorTimesScalar %v4float %34 %float_100
         %39 = OpFAdd %v4float %36 %38
         %40 = OpCompositeExtract %float %39 0
         %41 = OpConvertFToU %uint %40
         %42 = OpCompositeExtract %float %39 1
         %43 = OpConvertFToU %uint %42
         %44 = OpCompositeExtract %float %39 2
         %45 = OpConvertFToU %uint %44
         %46 = OpCompositeExtract %float %39 3
         %47 = OpConvertFToU %uint %46
         %48 = OpCompositeConstruct %v4uint %41 %43 %45 %47
               OpStore %uintValues %48
               OpStore %expectedA %54
               OpStore %expectedB %58
         %61 = OpCompositeExtract %uint %48 0
         %60 = OpExtInst %uint %1 UClamp %61 %uint_100 %uint_300
         %62 = OpIEqual %bool %60 %uint_100
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %66 = OpVectorShuffle %v2uint %48 %48 0 1
         %65 = OpExtInst %v2uint %1 UClamp %66 %68 %69
         %70 = OpVectorShuffle %v2uint %54 %54 0 1
         %71 = OpIEqual %v2bool %65 %70
         %73 = OpAll %bool %71
               OpBranch %64
         %64 = OpLabel
         %74 = OpPhi %bool %false %25 %73 %63
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpVectorShuffle %v3uint %48 %48 0 1 2
         %77 = OpExtInst %v3uint %1 UClamp %78 %80 %81
         %82 = OpVectorShuffle %v3uint %54 %54 0 1 2
         %83 = OpIEqual %v3bool %77 %82
         %85 = OpAll %bool %83
               OpBranch %76
         %76 = OpLabel
         %86 = OpPhi %bool %false %64 %85 %75
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %89 = OpExtInst %v4uint %1 UClamp %48 %90 %91
         %92 = OpIEqual %v4bool %89 %54
         %94 = OpAll %bool %92
               OpBranch %88
         %88 = OpLabel
         %95 = OpPhi %bool %false %76 %94 %87
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
               OpBranch %97
         %97 = OpLabel
         %99 = OpPhi %bool %false %88 %true %96
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %103 = OpVectorShuffle %v2uint %54 %54 0 1
        %104 = OpIEqual %v2bool %102 %103
        %105 = OpAll %bool %104
               OpBranch %101
        %101 = OpLabel
        %106 = OpPhi %bool %false %97 %105 %100
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %110 = OpVectorShuffle %v3uint %54 %54 0 1 2
        %111 = OpIEqual %v3bool %109 %110
        %112 = OpAll %bool %111
               OpBranch %108
        %108 = OpLabel
        %113 = OpPhi %bool %false %101 %112 %107
               OpSelectionMerge %115 None
               OpBranchConditional %113 %114 %115
        %114 = OpLabel
               OpBranch %115
        %115 = OpLabel
        %116 = OpPhi %bool %false %108 %true %114
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %119 = OpExtInst %uint %1 UClamp %61 %uint_100 %uint_300
        %120 = OpIEqual %bool %119 %uint_100
               OpBranch %118
        %118 = OpLabel
        %121 = OpPhi %bool %false %115 %120 %117
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %125 = OpVectorShuffle %v2uint %48 %48 0 1
        %124 = OpExtInst %v2uint %1 UClamp %125 %127 %129
        %130 = OpVectorShuffle %v2uint %58 %58 0 1
        %131 = OpIEqual %v2bool %124 %130
        %132 = OpAll %bool %131
               OpBranch %123
        %123 = OpLabel
        %133 = OpPhi %bool %false %118 %132 %122
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %137 = OpVectorShuffle %v3uint %48 %48 0 1 2
        %136 = OpExtInst %v3uint %1 UClamp %137 %138 %139
        %140 = OpVectorShuffle %v3uint %58 %58 0 1 2
        %141 = OpIEqual %v3bool %136 %140
        %142 = OpAll %bool %141
               OpBranch %135
        %135 = OpLabel
        %143 = OpPhi %bool %false %123 %142 %134
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
        %146 = OpExtInst %v4uint %1 UClamp %48 %147 %149
        %150 = OpIEqual %v4bool %146 %58
        %151 = OpAll %bool %150
               OpBranch %145
        %145 = OpLabel
        %152 = OpPhi %bool %false %135 %151 %144
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %bool %false %145 %true %153
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %158 = OpVectorShuffle %v2uint %58 %58 0 1
        %159 = OpIEqual %v2bool %102 %158
        %160 = OpAll %bool %159
               OpBranch %157
        %157 = OpLabel
        %161 = OpPhi %bool %false %154 %160 %156
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %163
        %162 = OpLabel
        %165 = OpVectorShuffle %v3uint %58 %58 0 1 2
        %166 = OpIEqual %v3bool %164 %165
        %167 = OpAll %bool %166
               OpBranch %163
        %163 = OpLabel
        %168 = OpPhi %bool %false %157 %167 %162
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
               OpBranch %170
        %170 = OpLabel
        %171 = OpPhi %bool %false %163 %true %169
               OpSelectionMerge %176 None
               OpBranchConditional %171 %174 %175
        %174 = OpLabel
        %177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %179 = OpLoad %v4float %177
               OpStore %172 %179
               OpBranch %176
        %175 = OpLabel
        %180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %182 = OpLoad %v4float %180
               OpStore %172 %182
               OpBranch %176
        %176 = OpLabel
        %183 = OpLoad %v4float %172
               OpReturnValue %183
               OpFunctionEnd
