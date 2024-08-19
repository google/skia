               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %16 Binding 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %sda RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %dsa RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %_2_mn RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %_3_mx RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %_4_lum RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %_5_result RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %_6_minComp RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %_7_maxComp RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
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
               OpDecorate %200 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %21 = OpTypeFunction %float %_ptr_Function_v3float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %44 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
        %114 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %121 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
       %void = OpTypeVoid
        %192 = OpTypeFunction %void
        %194 = OpConstantComposite %v2float %float_0 %float_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%blend_color_saturation_Qhh3 = OpFunction %float None %21
         %22 = OpFunctionParameter %_ptr_Function_v3float
         %23 = OpLabel
         %26 = OpLoad %v3float %22
         %27 = OpCompositeExtract %float %26 0
         %28 = OpLoad %v3float %22
         %29 = OpCompositeExtract %float %28 1
         %25 = OpExtInst %float %1 FMax %27 %29
         %30 = OpLoad %v3float %22
         %31 = OpCompositeExtract %float %30 2
         %24 = OpExtInst %float %1 FMax %25 %31
         %34 = OpLoad %v3float %22
         %35 = OpCompositeExtract %float %34 0
         %36 = OpLoad %v3float %22
         %37 = OpCompositeExtract %float %36 1
         %33 = OpExtInst %float %1 FMin %35 %37
         %38 = OpLoad %v3float %22
         %39 = OpCompositeExtract %float %38 2
         %32 = OpExtInst %float %1 FMin %33 %39
         %40 = OpFSub %float %24 %32
               OpReturnValue %40
               OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %44
         %45 = OpFunctionParameter %_ptr_Function_v2float
         %46 = OpFunctionParameter %_ptr_Function_v4float
         %47 = OpFunctionParameter %_ptr_Function_v4float
         %48 = OpLabel
      %alpha = OpVariable %_ptr_Function_float Function
        %sda = OpVariable %_ptr_Function_v3float Function
        %dsa = OpVariable %_ptr_Function_v3float Function
          %l = OpVariable %_ptr_Function_v3float Function
         %72 = OpVariable %_ptr_Function_v3float Function
          %r = OpVariable %_ptr_Function_v3float Function
         %81 = OpVariable %_ptr_Function_v3float Function
      %_2_mn = OpVariable %_ptr_Function_float Function
      %_3_mx = OpVariable %_ptr_Function_float Function
        %101 = OpVariable %_ptr_Function_v3float Function
        %107 = OpVariable %_ptr_Function_v3float Function
     %_4_lum = OpVariable %_ptr_Function_float Function
  %_5_result = OpVariable %_ptr_Function_v3float Function
 %_6_minComp = OpVariable %_ptr_Function_float Function
 %_7_maxComp = OpVariable %_ptr_Function_float Function
         %51 = OpLoad %v4float %47
         %52 = OpCompositeExtract %float %51 3
         %53 = OpLoad %v4float %46
         %54 = OpCompositeExtract %float %53 3
         %55 = OpFMul %float %52 %54
               OpStore %alpha %55
         %57 = OpLoad %v4float %46
         %58 = OpVectorShuffle %v3float %57 %57 0 1 2
         %59 = OpLoad %v4float %47
         %60 = OpCompositeExtract %float %59 3
         %61 = OpVectorTimesScalar %v3float %58 %60
               OpStore %sda %61
         %63 = OpLoad %v4float %47
         %64 = OpVectorShuffle %v3float %63 %63 0 1 2
         %65 = OpLoad %v4float %46
         %66 = OpCompositeExtract %float %65 3
         %67 = OpVectorTimesScalar %v3float %64 %66
               OpStore %dsa %67
         %69 = OpLoad %v2float %45
         %70 = OpCompositeExtract %float %69 0
         %71 = OpFUnordNotEqual %bool %70 %float_0
               OpSelectionMerge %75 None
               OpBranchConditional %71 %73 %74
         %73 = OpLabel
               OpStore %72 %67
               OpBranch %75
         %74 = OpLabel
               OpStore %72 %61
               OpBranch %75
         %75 = OpLabel
         %76 = OpLoad %v3float %72
               OpStore %l %76
         %78 = OpLoad %v2float %45
         %79 = OpCompositeExtract %float %78 0
         %80 = OpFUnordNotEqual %bool %79 %float_0
               OpSelectionMerge %84 None
               OpBranchConditional %80 %82 %83
         %82 = OpLabel
               OpStore %81 %61
               OpBranch %84
         %83 = OpLabel
               OpStore %81 %67
               OpBranch %84
         %84 = OpLabel
         %85 = OpLoad %v3float %81
               OpStore %r %85
         %86 = OpLoad %v2float %45
         %87 = OpCompositeExtract %float %86 1
         %88 = OpFUnordNotEqual %bool %87 %float_0
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %94 = OpCompositeExtract %float %76 0
         %95 = OpCompositeExtract %float %76 1
         %93 = OpExtInst %float %1 FMin %94 %95
         %96 = OpCompositeExtract %float %76 2
         %92 = OpExtInst %float %1 FMin %93 %96
               OpStore %_2_mn %92
         %99 = OpExtInst %float %1 FMax %94 %95
         %98 = OpExtInst %float %1 FMax %99 %96
               OpStore %_3_mx %98
        %100 = OpFOrdGreaterThan %bool %98 %92
               OpSelectionMerge %104 None
               OpBranchConditional %100 %102 %103
        %102 = OpLabel
        %105 = OpCompositeConstruct %v3float %92 %92 %92
        %106 = OpFSub %v3float %76 %105
               OpStore %107 %85
        %108 = OpFunctionCall %float %blend_color_saturation_Qhh3 %107
        %109 = OpVectorTimesScalar %v3float %106 %108
        %110 = OpFSub %float %98 %92
        %112 = OpFDiv %float %float_1 %110
        %113 = OpVectorTimesScalar %v3float %109 %112
               OpStore %101 %113
               OpBranch %104
        %103 = OpLabel
               OpStore %101 %114
               OpBranch %104
        %104 = OpLabel
        %115 = OpLoad %v3float %101
               OpStore %l %115
               OpStore %r %67
               OpBranch %90
         %90 = OpLabel
        %122 = OpLoad %v3float %r
        %117 = OpDot %float %121 %122
               OpStore %_4_lum %117
        %125 = OpLoad %v3float %l
        %124 = OpDot %float %121 %125
        %126 = OpFSub %float %117 %124
        %127 = OpLoad %v3float %l
        %128 = OpCompositeConstruct %v3float %126 %126 %126
        %129 = OpFAdd %v3float %128 %127
               OpStore %_5_result %129
        %133 = OpCompositeExtract %float %129 0
        %134 = OpCompositeExtract %float %129 1
        %132 = OpExtInst %float %1 FMin %133 %134
        %135 = OpCompositeExtract %float %129 2
        %131 = OpExtInst %float %1 FMin %132 %135
               OpStore %_6_minComp %131
        %138 = OpExtInst %float %1 FMax %133 %134
        %137 = OpExtInst %float %1 FMax %138 %135
               OpStore %_7_maxComp %137
        %139 = OpFOrdLessThan %bool %131 %float_0
               OpSelectionMerge %141 None
               OpBranchConditional %139 %140 %141
        %140 = OpLabel
        %142 = OpFUnordNotEqual %bool %117 %131
               OpBranch %141
        %141 = OpLabel
        %143 = OpPhi %bool %false %90 %142 %140
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
        %146 = OpCompositeConstruct %v3float %117 %117 %117
        %147 = OpFSub %v3float %129 %146
        %148 = OpFSub %float %117 %131
        %149 = OpLoad %float %_kGuardedDivideEpsilon
        %150 = OpFAdd %float %148 %149
        %151 = OpFDiv %float %117 %150
        %152 = OpVectorTimesScalar %v3float %147 %151
        %153 = OpFAdd %v3float %146 %152
               OpStore %_5_result %153
               OpBranch %145
        %145 = OpLabel
        %154 = OpFOrdGreaterThan %bool %137 %55
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %157 = OpFUnordNotEqual %bool %137 %117
               OpBranch %156
        %156 = OpLabel
        %158 = OpPhi %bool %false %145 %157 %155
               OpSelectionMerge %160 None
               OpBranchConditional %158 %159 %160
        %159 = OpLabel
        %161 = OpLoad %v3float %_5_result
        %162 = OpCompositeConstruct %v3float %117 %117 %117
        %163 = OpFSub %v3float %161 %162
        %164 = OpFSub %float %55 %117
        %165 = OpVectorTimesScalar %v3float %163 %164
        %166 = OpFSub %float %137 %117
        %167 = OpLoad %float %_kGuardedDivideEpsilon
        %168 = OpFAdd %float %166 %167
        %169 = OpFDiv %float %float_1 %168
        %170 = OpVectorTimesScalar %v3float %165 %169
        %171 = OpFAdd %v3float %162 %170
               OpStore %_5_result %171
               OpBranch %160
        %160 = OpLabel
        %172 = OpLoad %v3float %_5_result
        %173 = OpLoad %v4float %47
        %174 = OpVectorShuffle %v3float %173 %173 0 1 2
        %175 = OpFAdd %v3float %172 %174
        %176 = OpFSub %v3float %175 %67
        %177 = OpLoad %v4float %46
        %178 = OpVectorShuffle %v3float %177 %177 0 1 2
        %179 = OpFAdd %v3float %176 %178
        %180 = OpFSub %v3float %179 %61
        %181 = OpCompositeExtract %float %180 0
        %182 = OpCompositeExtract %float %180 1
        %183 = OpCompositeExtract %float %180 2
        %184 = OpLoad %v4float %46
        %185 = OpCompositeExtract %float %184 3
        %186 = OpLoad %v4float %47
        %187 = OpCompositeExtract %float %186 3
        %188 = OpFAdd %float %185 %187
        %189 = OpFSub %float %188 %55
        %190 = OpCompositeConstruct %v4float %181 %182 %183 %189
               OpReturnValue %190
               OpFunctionEnd
       %main = OpFunction %void None %192
        %193 = OpLabel
        %195 = OpVariable %_ptr_Function_v2float Function
        %201 = OpVariable %_ptr_Function_v4float Function
        %205 = OpVariable %_ptr_Function_v4float Function
         %10 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %10
               OpStore %195 %194
        %196 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
        %200 = OpLoad %v4float %196
               OpStore %201 %200
        %202 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
        %204 = OpLoad %v4float %202
               OpStore %205 %204
        %206 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %195 %201 %205
               OpStore %sk_FragColor %206
               OpReturn
               OpFunctionEnd
