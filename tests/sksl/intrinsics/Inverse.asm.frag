               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %matrix2x2 "matrix2x2"
               OpName %inv2x2 "inv2x2"
               OpName %inv3x3 "inv3x3"
               OpName %inv4x4 "inv4x4"
               OpName %Zero "Zero"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %33 = OpConstantComposite %v2float %float_1 %float_2
         %34 = OpConstantComposite %v2float %float_3 %float_4
         %35 = OpConstantComposite %mat2v2float %33 %34
   %float_n2 = OpConstant %float -2
  %float_1_5 = OpConstant %float 1.5
 %float_n0_5 = OpConstant %float -0.5
         %40 = OpConstantComposite %v2float %float_n2 %float_1
         %41 = OpConstantComposite %v2float %float_1_5 %float_n0_5
         %42 = OpConstantComposite %mat2v2float %40 %41
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
  %float_n24 = OpConstant %float -24
   %float_18 = OpConstant %float 18
    %float_5 = OpConstant %float 5
   %float_20 = OpConstant %float 20
  %float_n15 = OpConstant %float -15
   %float_n4 = OpConstant %float -4
   %float_n5 = OpConstant %float -5
         %54 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
         %55 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
         %56 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
         %57 = OpConstantComposite %mat3v3float %54 %55 %56
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_0_5 = OpConstant %float 0.5
   %float_n8 = OpConstant %float -8
   %float_n1 = OpConstant %float -1
         %64 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
         %65 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
         %66 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
         %67 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
         %68 = OpConstantComposite %mat4v4float %64 %65 %66 %67
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
        %118 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %119 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %120 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %121 = OpConstantComposite %mat3v3float %118 %119 %120
        %153 = OpConstantComposite %v3float %float_0 %float_1 %float_4
        %154 = OpConstantComposite %v3float %float_5 %float_6 %float_0
        %155 = OpConstantComposite %mat3v3float %118 %153 %154
        %177 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %178 = OpConstantComposite %v4float %float_0 %float_2 %float_1 %float_2
        %179 = OpConstantComposite %v4float %float_2 %float_1 %float_0 %float_1
        %180 = OpConstantComposite %v4float %float_2 %float_0 %float_1 %float_4
        %181 = OpConstantComposite %mat4v4float %177 %178 %179 %180
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
  %matrix2x2 = OpVariable %_ptr_Function_mat2v2float Function
     %inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
     %inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
     %inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
       %Zero = OpVariable %_ptr_Function_float Function
        %205 = OpVariable %_ptr_Function_v4float Function
               OpStore %matrix2x2 %35
               OpStore %inv2x2 %42
               OpStore %inv3x3 %57
               OpStore %inv4x4 %68
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %75 = OpLoad %v4float %71
         %76 = OpCompositeExtract %float %75 2
               OpStore %Zero %76
         %79 = OpFOrdEqual %v2bool %40 %40
         %80 = OpAll %bool %79
         %81 = OpFOrdEqual %v2bool %41 %41
         %82 = OpAll %bool %81
         %83 = OpLogicalAnd %bool %80 %82
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %87 = OpFOrdEqual %v3bool %54 %54
         %88 = OpAll %bool %87
         %89 = OpFOrdEqual %v3bool %55 %55
         %90 = OpAll %bool %89
         %91 = OpLogicalAnd %bool %88 %90
         %92 = OpFOrdEqual %v3bool %56 %56
         %93 = OpAll %bool %92
         %94 = OpLogicalAnd %bool %91 %93
               OpBranch %85
         %85 = OpLabel
         %95 = OpPhi %bool %false %25 %94 %84
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
         %99 = OpFOrdEqual %v4bool %64 %64
        %100 = OpAll %bool %99
        %101 = OpFOrdEqual %v4bool %65 %65
        %102 = OpAll %bool %101
        %103 = OpLogicalAnd %bool %100 %102
        %104 = OpFOrdEqual %v4bool %66 %66
        %105 = OpAll %bool %104
        %106 = OpLogicalAnd %bool %103 %105
        %107 = OpFOrdEqual %v4bool %67 %67
        %108 = OpAll %bool %107
        %109 = OpLogicalAnd %bool %106 %108
               OpBranch %97
         %97 = OpLabel
        %110 = OpPhi %bool %false %85 %109 %96
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpExtInst %mat3v3float %1 MatrixInverse %121
        %122 = OpCompositeExtract %v3float %113 0
        %123 = OpFUnordNotEqual %v3bool %122 %54
        %124 = OpAny %bool %123
        %125 = OpCompositeExtract %v3float %113 1
        %126 = OpFUnordNotEqual %v3bool %125 %55
        %127 = OpAny %bool %126
        %128 = OpLogicalOr %bool %124 %127
        %129 = OpCompositeExtract %v3float %113 2
        %130 = OpFUnordNotEqual %v3bool %129 %56
        %131 = OpAny %bool %130
        %132 = OpLogicalOr %bool %128 %131
               OpBranch %112
        %112 = OpLabel
        %133 = OpPhi %bool %false %97 %132 %111
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %137 = OpCompositeConstruct %v2float %76 %76
        %138 = OpCompositeConstruct %mat2v2float %137 %137
        %139 = OpFAdd %v2float %33 %137
        %140 = OpFAdd %v2float %34 %137
        %141 = OpCompositeConstruct %mat2v2float %139 %140
        %136 = OpExtInst %mat2v2float %1 MatrixInverse %141
        %142 = OpCompositeExtract %v2float %136 0
        %143 = OpFOrdEqual %v2bool %142 %40
        %144 = OpAll %bool %143
        %145 = OpCompositeExtract %v2float %136 1
        %146 = OpFOrdEqual %v2bool %145 %41
        %147 = OpAll %bool %146
        %148 = OpLogicalAnd %bool %144 %147
               OpBranch %135
        %135 = OpLabel
        %149 = OpPhi %bool %false %112 %148 %134
               OpSelectionMerge %151 None
               OpBranchConditional %149 %150 %151
        %150 = OpLabel
        %156 = OpCompositeConstruct %v3float %76 %76 %76
        %157 = OpCompositeConstruct %mat3v3float %156 %156 %156
        %158 = OpFAdd %v3float %118 %156
        %159 = OpFAdd %v3float %153 %156
        %160 = OpFAdd %v3float %154 %156
        %161 = OpCompositeConstruct %mat3v3float %158 %159 %160
        %152 = OpExtInst %mat3v3float %1 MatrixInverse %161
        %162 = OpCompositeExtract %v3float %152 0
        %163 = OpFOrdEqual %v3bool %162 %54
        %164 = OpAll %bool %163
        %165 = OpCompositeExtract %v3float %152 1
        %166 = OpFOrdEqual %v3bool %165 %55
        %167 = OpAll %bool %166
        %168 = OpLogicalAnd %bool %164 %167
        %169 = OpCompositeExtract %v3float %152 2
        %170 = OpFOrdEqual %v3bool %169 %56
        %171 = OpAll %bool %170
        %172 = OpLogicalAnd %bool %168 %171
               OpBranch %151
        %151 = OpLabel
        %173 = OpPhi %bool %false %135 %172 %150
               OpSelectionMerge %175 None
               OpBranchConditional %173 %174 %175
        %174 = OpLabel
        %182 = OpCompositeConstruct %v4float %76 %76 %76 %76
        %183 = OpCompositeConstruct %mat4v4float %182 %182 %182 %182
        %184 = OpFAdd %v4float %177 %182
        %185 = OpFAdd %v4float %178 %182
        %186 = OpFAdd %v4float %179 %182
        %187 = OpFAdd %v4float %180 %182
        %188 = OpCompositeConstruct %mat4v4float %184 %185 %186 %187
        %176 = OpExtInst %mat4v4float %1 MatrixInverse %188
        %189 = OpCompositeExtract %v4float %176 0
        %190 = OpFOrdEqual %v4bool %189 %64
        %191 = OpAll %bool %190
        %192 = OpCompositeExtract %v4float %176 1
        %193 = OpFOrdEqual %v4bool %192 %65
        %194 = OpAll %bool %193
        %195 = OpLogicalAnd %bool %191 %194
        %196 = OpCompositeExtract %v4float %176 2
        %197 = OpFOrdEqual %v4bool %196 %66
        %198 = OpAll %bool %197
        %199 = OpLogicalAnd %bool %195 %198
        %200 = OpCompositeExtract %v4float %176 3
        %201 = OpFOrdEqual %v4bool %200 %67
        %202 = OpAll %bool %201
        %203 = OpLogicalAnd %bool %199 %202
               OpBranch %175
        %175 = OpLabel
        %204 = OpPhi %bool %false %151 %203 %174
               OpSelectionMerge %209 None
               OpBranchConditional %204 %207 %208
        %207 = OpLabel
        %210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %211 = OpLoad %v4float %210
               OpStore %205 %211
               OpBranch %209
        %208 = OpLabel
        %212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %214 = OpLoad %v4float %212
               OpStore %205 %214
               OpBranch %209
        %209 = OpLabel
        %215 = OpLoad %v4float %205
               OpReturnValue %215
               OpFunctionEnd
