               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
               OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"
               OpName %alpha "alpha"
               OpName %sda "sda"
               OpName %dsa "dsa"
               OpName %l "l"
               OpName %r "r"
               OpName %_2_mn "_2_mn"
               OpName %_3_mx "_3_mx"
               OpName %_4_lum "_4_lum"
               OpName %_5_result "_5_result"
               OpName %_6_minComp "_6_minComp"
               OpName %_7_maxComp "_7_maxComp"
               OpName %main "main"
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %18 Binding 0
               OpDecorate %18 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %sda RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %dsa RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %_2_mn RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %_3_mx RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %_4_lum RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %_5_result RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %_6_minComp RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %_7_maxComp RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %23 = OpTypeFunction %float %_ptr_Function_v3float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %46 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
        %116 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %123 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
       %void = OpTypeVoid
        %194 = OpTypeFunction %void
        %196 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%blend_color_saturation_Qhh3 = OpFunction %float None %23
         %24 = OpFunctionParameter %_ptr_Function_v3float
         %25 = OpLabel
         %28 = OpLoad %v3float %24
         %29 = OpCompositeExtract %float %28 0
         %30 = OpLoad %v3float %24
         %31 = OpCompositeExtract %float %30 1
         %27 = OpExtInst %float %1 FMax %29 %31
         %32 = OpLoad %v3float %24
         %33 = OpCompositeExtract %float %32 2
         %26 = OpExtInst %float %1 FMax %27 %33
         %36 = OpLoad %v3float %24
         %37 = OpCompositeExtract %float %36 0
         %38 = OpLoad %v3float %24
         %39 = OpCompositeExtract %float %38 1
         %35 = OpExtInst %float %1 FMin %37 %39
         %40 = OpLoad %v3float %24
         %41 = OpCompositeExtract %float %40 2
         %34 = OpExtInst %float %1 FMin %35 %41
         %42 = OpFSub %float %26 %34
               OpReturnValue %42
               OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %46
         %47 = OpFunctionParameter %_ptr_Function_v2float
         %48 = OpFunctionParameter %_ptr_Function_v4float
         %49 = OpFunctionParameter %_ptr_Function_v4float
         %50 = OpLabel
      %alpha = OpVariable %_ptr_Function_float Function
        %sda = OpVariable %_ptr_Function_v3float Function
        %dsa = OpVariable %_ptr_Function_v3float Function
          %l = OpVariable %_ptr_Function_v3float Function
         %74 = OpVariable %_ptr_Function_v3float Function
          %r = OpVariable %_ptr_Function_v3float Function
         %83 = OpVariable %_ptr_Function_v3float Function
      %_2_mn = OpVariable %_ptr_Function_float Function
      %_3_mx = OpVariable %_ptr_Function_float Function
        %103 = OpVariable %_ptr_Function_v3float Function
        %109 = OpVariable %_ptr_Function_v3float Function
     %_4_lum = OpVariable %_ptr_Function_float Function
  %_5_result = OpVariable %_ptr_Function_v3float Function
 %_6_minComp = OpVariable %_ptr_Function_float Function
 %_7_maxComp = OpVariable %_ptr_Function_float Function
         %53 = OpLoad %v4float %49
         %54 = OpCompositeExtract %float %53 3
         %55 = OpLoad %v4float %48
         %56 = OpCompositeExtract %float %55 3
         %57 = OpFMul %float %54 %56
               OpStore %alpha %57
         %59 = OpLoad %v4float %48
         %60 = OpVectorShuffle %v3float %59 %59 0 1 2
         %61 = OpLoad %v4float %49
         %62 = OpCompositeExtract %float %61 3
         %63 = OpVectorTimesScalar %v3float %60 %62
               OpStore %sda %63
         %65 = OpLoad %v4float %49
         %66 = OpVectorShuffle %v3float %65 %65 0 1 2
         %67 = OpLoad %v4float %48
         %68 = OpCompositeExtract %float %67 3
         %69 = OpVectorTimesScalar %v3float %66 %68
               OpStore %dsa %69
         %71 = OpLoad %v2float %47
         %72 = OpCompositeExtract %float %71 0
         %73 = OpFUnordNotEqual %bool %72 %float_0
               OpSelectionMerge %77 None
               OpBranchConditional %73 %75 %76
         %75 = OpLabel
               OpStore %74 %69
               OpBranch %77
         %76 = OpLabel
               OpStore %74 %63
               OpBranch %77
         %77 = OpLabel
         %78 = OpLoad %v3float %74
               OpStore %l %78
         %80 = OpLoad %v2float %47
         %81 = OpCompositeExtract %float %80 0
         %82 = OpFUnordNotEqual %bool %81 %float_0
               OpSelectionMerge %86 None
               OpBranchConditional %82 %84 %85
         %84 = OpLabel
               OpStore %83 %63
               OpBranch %86
         %85 = OpLabel
               OpStore %83 %69
               OpBranch %86
         %86 = OpLabel
         %87 = OpLoad %v3float %83
               OpStore %r %87
         %88 = OpLoad %v2float %47
         %89 = OpCompositeExtract %float %88 1
         %90 = OpFUnordNotEqual %bool %89 %float_0
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %96 = OpCompositeExtract %float %78 0
         %97 = OpCompositeExtract %float %78 1
         %95 = OpExtInst %float %1 FMin %96 %97
         %98 = OpCompositeExtract %float %78 2
         %94 = OpExtInst %float %1 FMin %95 %98
               OpStore %_2_mn %94
        %101 = OpExtInst %float %1 FMax %96 %97
        %100 = OpExtInst %float %1 FMax %101 %98
               OpStore %_3_mx %100
        %102 = OpFOrdGreaterThan %bool %100 %94
               OpSelectionMerge %106 None
               OpBranchConditional %102 %104 %105
        %104 = OpLabel
        %107 = OpCompositeConstruct %v3float %94 %94 %94
        %108 = OpFSub %v3float %78 %107
               OpStore %109 %87
        %110 = OpFunctionCall %float %blend_color_saturation_Qhh3 %109
        %111 = OpVectorTimesScalar %v3float %108 %110
        %112 = OpFSub %float %100 %94
        %114 = OpFDiv %float %float_1 %112
        %115 = OpVectorTimesScalar %v3float %111 %114
               OpStore %103 %115
               OpBranch %106
        %105 = OpLabel
               OpStore %103 %116
               OpBranch %106
        %106 = OpLabel
        %117 = OpLoad %v3float %103
               OpStore %l %117
               OpStore %r %69
               OpBranch %92
         %92 = OpLabel
        %124 = OpLoad %v3float %r
        %119 = OpDot %float %123 %124
               OpStore %_4_lum %119
        %127 = OpLoad %v3float %l
        %126 = OpDot %float %123 %127
        %128 = OpFSub %float %119 %126
        %129 = OpLoad %v3float %l
        %130 = OpCompositeConstruct %v3float %128 %128 %128
        %131 = OpFAdd %v3float %130 %129
               OpStore %_5_result %131
        %135 = OpCompositeExtract %float %131 0
        %136 = OpCompositeExtract %float %131 1
        %134 = OpExtInst %float %1 FMin %135 %136
        %137 = OpCompositeExtract %float %131 2
        %133 = OpExtInst %float %1 FMin %134 %137
               OpStore %_6_minComp %133
        %140 = OpExtInst %float %1 FMax %135 %136
        %139 = OpExtInst %float %1 FMax %140 %137
               OpStore %_7_maxComp %139
        %141 = OpFOrdLessThan %bool %133 %float_0
               OpSelectionMerge %143 None
               OpBranchConditional %141 %142 %143
        %142 = OpLabel
        %144 = OpFUnordNotEqual %bool %119 %133
               OpBranch %143
        %143 = OpLabel
        %145 = OpPhi %bool %false %92 %144 %142
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
        %148 = OpCompositeConstruct %v3float %119 %119 %119
        %149 = OpFSub %v3float %131 %148
        %150 = OpFSub %float %119 %133
        %151 = OpLoad %float %_kGuardedDivideEpsilon
        %152 = OpFAdd %float %150 %151
        %153 = OpFDiv %float %119 %152
        %154 = OpVectorTimesScalar %v3float %149 %153
        %155 = OpFAdd %v3float %148 %154
               OpStore %_5_result %155
               OpBranch %147
        %147 = OpLabel
        %156 = OpFOrdGreaterThan %bool %139 %57
               OpSelectionMerge %158 None
               OpBranchConditional %156 %157 %158
        %157 = OpLabel
        %159 = OpFUnordNotEqual %bool %139 %119
               OpBranch %158
        %158 = OpLabel
        %160 = OpPhi %bool %false %147 %159 %157
               OpSelectionMerge %162 None
               OpBranchConditional %160 %161 %162
        %161 = OpLabel
        %163 = OpLoad %v3float %_5_result
        %164 = OpCompositeConstruct %v3float %119 %119 %119
        %165 = OpFSub %v3float %163 %164
        %166 = OpFSub %float %57 %119
        %167 = OpVectorTimesScalar %v3float %165 %166
        %168 = OpFSub %float %139 %119
        %169 = OpLoad %float %_kGuardedDivideEpsilon
        %170 = OpFAdd %float %168 %169
        %171 = OpFDiv %float %float_1 %170
        %172 = OpVectorTimesScalar %v3float %167 %171
        %173 = OpFAdd %v3float %164 %172
               OpStore %_5_result %173
               OpBranch %162
        %162 = OpLabel
        %174 = OpLoad %v3float %_5_result
        %175 = OpLoad %v4float %49
        %176 = OpVectorShuffle %v3float %175 %175 0 1 2
        %177 = OpFAdd %v3float %174 %176
        %178 = OpFSub %v3float %177 %69
        %179 = OpLoad %v4float %48
        %180 = OpVectorShuffle %v3float %179 %179 0 1 2
        %181 = OpFAdd %v3float %178 %180
        %182 = OpFSub %v3float %181 %63
        %183 = OpCompositeExtract %float %182 0
        %184 = OpCompositeExtract %float %182 1
        %185 = OpCompositeExtract %float %182 2
        %186 = OpLoad %v4float %48
        %187 = OpCompositeExtract %float %186 3
        %188 = OpLoad %v4float %49
        %189 = OpCompositeExtract %float %188 3
        %190 = OpFAdd %float %187 %189
        %191 = OpFSub %float %190 %57
        %192 = OpCompositeConstruct %v4float %183 %184 %185 %191
               OpReturnValue %192
               OpFunctionEnd
       %main = OpFunction %void None %194
        %195 = OpLabel
        %197 = OpVariable %_ptr_Function_v2float Function
        %203 = OpVariable %_ptr_Function_v4float Function
        %207 = OpVariable %_ptr_Function_v4float Function
         %10 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %10
               OpStore %197 %196
        %198 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
        %202 = OpLoad %v4float %198
               OpStore %203 %202
        %204 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
        %206 = OpLoad %v4float %204
               OpStore %207 %206
        %208 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %197 %203 %207
               OpStore %sk_FragColor %208
               OpReturn
               OpFunctionEnd
