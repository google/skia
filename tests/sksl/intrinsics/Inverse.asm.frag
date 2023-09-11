               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %30 = OpConstantComposite %v2float %float_1 %float_2
         %31 = OpConstantComposite %v2float %float_3 %float_4
         %32 = OpConstantComposite %mat2v2float %30 %31
   %float_n2 = OpConstant %float -2
  %float_1_5 = OpConstant %float 1.5
 %float_n0_5 = OpConstant %float -0.5
         %37 = OpConstantComposite %v2float %float_n2 %float_1
         %38 = OpConstantComposite %v2float %float_1_5 %float_n0_5
         %39 = OpConstantComposite %mat2v2float %37 %38
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
         %51 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
         %52 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
         %53 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
         %54 = OpConstantComposite %mat3v3float %51 %52 %53
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_0_5 = OpConstant %float 0.5
   %float_n8 = OpConstant %float -8
   %float_n1 = OpConstant %float -1
         %61 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
         %62 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
         %63 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
         %64 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
         %65 = OpConstantComposite %mat4v4float %61 %62 %63 %64
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
    %float_9 = OpConstant %float 9
        %116 = OpConstantComposite %v3float %float_1 %float_2 %float_3
        %117 = OpConstantComposite %v3float %float_4 %float_5 %float_6
        %118 = OpConstantComposite %v3float %float_7 %float_8 %float_9
        %119 = OpConstantComposite %mat3v3float %116 %117 %118
        %151 = OpConstantComposite %v3float %float_0 %float_1 %float_4
        %152 = OpConstantComposite %v3float %float_5 %float_6 %float_0
        %153 = OpConstantComposite %mat3v3float %116 %151 %152
        %175 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
        %176 = OpConstantComposite %v4float %float_0 %float_2 %float_1 %float_2
        %177 = OpConstantComposite %v4float %float_2 %float_1 %float_0 %float_1
        %178 = OpConstantComposite %v4float %float_2 %float_0 %float_1 %float_4
        %179 = OpConstantComposite %mat4v4float %175 %176 %177 %178
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
  %matrix2x2 = OpVariable %_ptr_Function_mat2v2float Function
     %inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
     %inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
     %inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
       %Zero = OpVariable %_ptr_Function_float Function
        %203 = OpVariable %_ptr_Function_v4float Function
               OpStore %matrix2x2 %32
               OpStore %inv2x2 %39
               OpStore %inv3x3 %54
               OpStore %inv4x4 %65
         %68 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %72 = OpLoad %v4float %68
         %73 = OpCompositeExtract %float %72 2
               OpStore %Zero %73
         %77 = OpFOrdEqual %v2bool %37 %37
         %78 = OpAll %bool %77
         %79 = OpFOrdEqual %v2bool %38 %38
         %80 = OpAll %bool %79
         %81 = OpLogicalAnd %bool %78 %80
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %85 = OpFOrdEqual %v3bool %51 %51
         %86 = OpAll %bool %85
         %87 = OpFOrdEqual %v3bool %52 %52
         %88 = OpAll %bool %87
         %89 = OpLogicalAnd %bool %86 %88
         %90 = OpFOrdEqual %v3bool %53 %53
         %91 = OpAll %bool %90
         %92 = OpLogicalAnd %bool %89 %91
               OpBranch %83
         %83 = OpLabel
         %93 = OpPhi %bool %false %22 %92 %82
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %97 = OpFOrdEqual %v4bool %61 %61
         %98 = OpAll %bool %97
         %99 = OpFOrdEqual %v4bool %62 %62
        %100 = OpAll %bool %99
        %101 = OpLogicalAnd %bool %98 %100
        %102 = OpFOrdEqual %v4bool %63 %63
        %103 = OpAll %bool %102
        %104 = OpLogicalAnd %bool %101 %103
        %105 = OpFOrdEqual %v4bool %64 %64
        %106 = OpAll %bool %105
        %107 = OpLogicalAnd %bool %104 %106
               OpBranch %95
         %95 = OpLabel
        %108 = OpPhi %bool %false %83 %107 %94
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %111 = OpExtInst %mat3v3float %1 MatrixInverse %119
        %120 = OpCompositeExtract %v3float %111 0
        %121 = OpFUnordNotEqual %v3bool %120 %51
        %122 = OpAny %bool %121
        %123 = OpCompositeExtract %v3float %111 1
        %124 = OpFUnordNotEqual %v3bool %123 %52
        %125 = OpAny %bool %124
        %126 = OpLogicalOr %bool %122 %125
        %127 = OpCompositeExtract %v3float %111 2
        %128 = OpFUnordNotEqual %v3bool %127 %53
        %129 = OpAny %bool %128
        %130 = OpLogicalOr %bool %126 %129
               OpBranch %110
        %110 = OpLabel
        %131 = OpPhi %bool %false %95 %130 %109
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %135 = OpCompositeConstruct %v2float %73 %73
        %136 = OpCompositeConstruct %mat2v2float %135 %135
        %137 = OpFAdd %v2float %30 %135
        %138 = OpFAdd %v2float %31 %135
        %139 = OpCompositeConstruct %mat2v2float %137 %138
        %134 = OpExtInst %mat2v2float %1 MatrixInverse %139
        %140 = OpCompositeExtract %v2float %134 0
        %141 = OpFOrdEqual %v2bool %140 %37
        %142 = OpAll %bool %141
        %143 = OpCompositeExtract %v2float %134 1
        %144 = OpFOrdEqual %v2bool %143 %38
        %145 = OpAll %bool %144
        %146 = OpLogicalAnd %bool %142 %145
               OpBranch %133
        %133 = OpLabel
        %147 = OpPhi %bool %false %110 %146 %132
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
        %154 = OpCompositeConstruct %v3float %73 %73 %73
        %155 = OpCompositeConstruct %mat3v3float %154 %154 %154
        %156 = OpFAdd %v3float %116 %154
        %157 = OpFAdd %v3float %151 %154
        %158 = OpFAdd %v3float %152 %154
        %159 = OpCompositeConstruct %mat3v3float %156 %157 %158
        %150 = OpExtInst %mat3v3float %1 MatrixInverse %159
        %160 = OpCompositeExtract %v3float %150 0
        %161 = OpFOrdEqual %v3bool %160 %51
        %162 = OpAll %bool %161
        %163 = OpCompositeExtract %v3float %150 1
        %164 = OpFOrdEqual %v3bool %163 %52
        %165 = OpAll %bool %164
        %166 = OpLogicalAnd %bool %162 %165
        %167 = OpCompositeExtract %v3float %150 2
        %168 = OpFOrdEqual %v3bool %167 %53
        %169 = OpAll %bool %168
        %170 = OpLogicalAnd %bool %166 %169
               OpBranch %149
        %149 = OpLabel
        %171 = OpPhi %bool %false %133 %170 %148
               OpSelectionMerge %173 None
               OpBranchConditional %171 %172 %173
        %172 = OpLabel
        %180 = OpCompositeConstruct %v4float %73 %73 %73 %73
        %181 = OpCompositeConstruct %mat4v4float %180 %180 %180 %180
        %182 = OpFAdd %v4float %175 %180
        %183 = OpFAdd %v4float %176 %180
        %184 = OpFAdd %v4float %177 %180
        %185 = OpFAdd %v4float %178 %180
        %186 = OpCompositeConstruct %mat4v4float %182 %183 %184 %185
        %174 = OpExtInst %mat4v4float %1 MatrixInverse %186
        %187 = OpCompositeExtract %v4float %174 0
        %188 = OpFOrdEqual %v4bool %187 %61
        %189 = OpAll %bool %188
        %190 = OpCompositeExtract %v4float %174 1
        %191 = OpFOrdEqual %v4bool %190 %62
        %192 = OpAll %bool %191
        %193 = OpLogicalAnd %bool %189 %192
        %194 = OpCompositeExtract %v4float %174 2
        %195 = OpFOrdEqual %v4bool %194 %63
        %196 = OpAll %bool %195
        %197 = OpLogicalAnd %bool %193 %196
        %198 = OpCompositeExtract %v4float %174 3
        %199 = OpFOrdEqual %v4bool %198 %64
        %200 = OpAll %bool %199
        %201 = OpLogicalAnd %bool %197 %200
               OpBranch %173
        %173 = OpLabel
        %202 = OpPhi %bool %false %149 %201 %172
               OpSelectionMerge %207 None
               OpBranchConditional %202 %205 %206
        %205 = OpLabel
        %208 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %209 = OpLoad %v4float %208
               OpStore %203 %209
               OpBranch %207
        %206 = OpLabel
        %210 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %212 = OpLoad %v4float %210
               OpStore %203 %212
               OpBranch %207
        %207 = OpLabel
        %213 = OpLoad %v4float %203
               OpReturnValue %213
               OpFunctionEnd
