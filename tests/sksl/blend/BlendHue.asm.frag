               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"  ; id %10
               OpName %sk_FragColor "sk_FragColor"                      ; id %18
               OpName %_UniformBuffer "_UniformBuffer"                  ; id %22
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"    ; id %6
               OpName %guarded_divide_Qh3h3h "guarded_divide_Qh3h3h"                ; id %7
               OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"                    ; id %8
               OpName %alpha "alpha"                                                ; id %66
               OpName %sda "sda"                                                    ; id %72
               OpName %dsa "dsa"                                                    ; id %78
               OpName %l "l"                                                        ; id %84
               OpName %r "r"                                                        ; id %93
               OpName %_2_mn "_2_mn"                                                ; id %107
               OpName %_3_mx "_3_mx"                                                ; id %113
               OpName %_4_diff "_4_diff"                                            ; id %116
               OpName %_5_lum "_5_lum"                                              ; id %134
               OpName %_6_result "_6_result"                                        ; id %141
               OpName %_7_minComp "_7_minComp"                                      ; id %148
               OpName %_8_maxComp "_8_maxComp"                                      ; id %154
               OpName %main "main"                                                  ; id %9

               ; Annotations
               OpDecorate %blend_color_saturation_Qhh3 RelaxedPrecision
               OpDecorate %guarded_divide_Qh3h3h RelaxedPrecision
               OpDecorate %blend_hslc_h4h2h4h4 RelaxedPrecision
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %21 Binding 0
               OpDecorate %21 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %sda RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %dsa RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %_2_mn RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %_3_mx RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %_4_diff RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %_5_lum RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %_6_result RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %_7_minComp RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %_8_maxComp RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
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
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private    ; RelaxedPrecision
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %21 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %26 = OpTypeFunction %float %_ptr_Function_v3float
%_ptr_Function_float = OpTypePointer Function %float
         %47 = OpTypeFunction %v3float %_ptr_Function_v3float %_ptr_Function_float
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %61 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%float_0_000244140625 = OpConstant %float 0.000244140625
        %132 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %139 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
       %void = OpTypeVoid
        %213 = OpTypeFunction %void
        %215 = OpConstantComposite %v2float %float_0 %float_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function blend_color_saturation_Qhh3
%blend_color_saturation_Qhh3 = OpFunction %float None %26   ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v3float   ; RelaxedPrecision

         %28 = OpLabel
         %31 =   OpLoad %v3float %27                ; RelaxedPrecision
         %32 =   OpCompositeExtract %float %31 0    ; RelaxedPrecision
         %33 =   OpLoad %v3float %27                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 1    ; RelaxedPrecision
         %30 =   OpExtInst %float %5 FMax %32 %34   ; RelaxedPrecision
         %35 =   OpLoad %v3float %27                ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %35 2    ; RelaxedPrecision
         %29 =   OpExtInst %float %5 FMax %30 %36   ; RelaxedPrecision
         %39 =   OpLoad %v3float %27                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0    ; RelaxedPrecision
         %41 =   OpLoad %v3float %27                ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %41 1    ; RelaxedPrecision
         %38 =   OpExtInst %float %5 FMin %40 %42   ; RelaxedPrecision
         %43 =   OpLoad %v3float %27                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 2    ; RelaxedPrecision
         %37 =   OpExtInst %float %5 FMin %38 %44   ; RelaxedPrecision
         %45 =   OpFSub %float %29 %37              ; RelaxedPrecision
                 OpReturnValue %45
               OpFunctionEnd


               ; Function guarded_divide_Qh3h3h
%guarded_divide_Qh3h3h = OpFunction %v3float None %47   ; RelaxedPrecision
         %48 = OpFunctionParameter %_ptr_Function_v3float   ; RelaxedPrecision
         %49 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %50 = OpLabel
         %51 =   OpLoad %v3float %48                ; RelaxedPrecision
         %52 =   OpLoad %float %49                  ; RelaxedPrecision
         %53 =   OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
         %54 =   OpFAdd %float %52 %53                  ; RelaxedPrecision
         %56 =   OpFDiv %float %float_1 %54             ; RelaxedPrecision
         %57 =   OpVectorTimesScalar %v3float %51 %56   ; RelaxedPrecision
                 OpReturnValue %57
               OpFunctionEnd


               ; Function blend_hslc_h4h2h4h4
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %61     ; RelaxedPrecision
         %62 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %63 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %64 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %65 = OpLabel
      %alpha =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
        %sda =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
        %dsa =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
          %l =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %88 =   OpVariable %_ptr_Function_v3float Function
          %r =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %97 =   OpVariable %_ptr_Function_v3float Function
      %_2_mn =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %_3_mx =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
    %_4_diff =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
        %120 =   OpVariable %_ptr_Function_v3float Function
        %126 =   OpVariable %_ptr_Function_v3float Function
        %129 =   OpVariable %_ptr_Function_v3float Function
        %130 =   OpVariable %_ptr_Function_float Function
     %_5_lum =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
  %_6_result =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
 %_7_minComp =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
 %_8_maxComp =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
         %67 =   OpLoad %v4float %64                            ; RelaxedPrecision
         %68 =   OpCompositeExtract %float %67 3                ; RelaxedPrecision
         %69 =   OpLoad %v4float %63                            ; RelaxedPrecision
         %70 =   OpCompositeExtract %float %69 3                ; RelaxedPrecision
         %71 =   OpFMul %float %68 %70                          ; RelaxedPrecision
                 OpStore %alpha %71
         %73 =   OpLoad %v4float %63                ; RelaxedPrecision
         %74 =   OpVectorShuffle %v3float %73 %73 0 1 2     ; RelaxedPrecision
         %75 =   OpLoad %v4float %64                        ; RelaxedPrecision
         %76 =   OpCompositeExtract %float %75 3            ; RelaxedPrecision
         %77 =   OpVectorTimesScalar %v3float %74 %76       ; RelaxedPrecision
                 OpStore %sda %77
         %79 =   OpLoad %v4float %64                ; RelaxedPrecision
         %80 =   OpVectorShuffle %v3float %79 %79 0 1 2     ; RelaxedPrecision
         %81 =   OpLoad %v4float %63                        ; RelaxedPrecision
         %82 =   OpCompositeExtract %float %81 3            ; RelaxedPrecision
         %83 =   OpVectorTimesScalar %v3float %80 %82       ; RelaxedPrecision
                 OpStore %dsa %83
         %85 =   OpLoad %v2float %62                ; RelaxedPrecision
         %86 =   OpCompositeExtract %float %85 0    ; RelaxedPrecision
         %87 =   OpFUnordNotEqual %bool %86 %float_0
                 OpSelectionMerge %91 None
                 OpBranchConditional %87 %89 %90

         %89 =     OpLabel
                     OpStore %88 %83
                     OpBranch %91

         %90 =     OpLabel
                     OpStore %88 %77
                     OpBranch %91

         %91 = OpLabel
         %92 =   OpLoad %v3float %88                ; RelaxedPrecision
                 OpStore %l %92
         %94 =   OpLoad %v2float %62                ; RelaxedPrecision
         %95 =   OpCompositeExtract %float %94 0    ; RelaxedPrecision
         %96 =   OpFUnordNotEqual %bool %95 %float_0
                 OpSelectionMerge %100 None
                 OpBranchConditional %96 %98 %99

         %98 =     OpLabel
                     OpStore %97 %77
                     OpBranch %100

         %99 =     OpLabel
                     OpStore %97 %83
                     OpBranch %100

        %100 = OpLabel
        %101 =   OpLoad %v3float %97                ; RelaxedPrecision
                 OpStore %r %101
        %102 =   OpLoad %v2float %62                ; RelaxedPrecision
        %103 =   OpCompositeExtract %float %102 1   ; RelaxedPrecision
        %104 =   OpFUnordNotEqual %bool %103 %float_0
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
        %110 =       OpCompositeExtract %float %92 0    ; RelaxedPrecision
        %111 =       OpCompositeExtract %float %92 1    ; RelaxedPrecision
        %109 =       OpExtInst %float %5 FMin %110 %111     ; RelaxedPrecision
        %112 =       OpCompositeExtract %float %92 2        ; RelaxedPrecision
        %108 =       OpExtInst %float %5 FMin %109 %112     ; RelaxedPrecision
                     OpStore %_2_mn %108
        %115 =       OpExtInst %float %5 FMax %110 %111     ; RelaxedPrecision
        %114 =       OpExtInst %float %5 FMax %115 %112     ; RelaxedPrecision
                     OpStore %_3_mx %114
        %117 =       OpFSub %float %114 %108        ; RelaxedPrecision
                     OpStore %_4_diff %117
        %119 =       OpFOrdGreaterThanEqual %bool %117 %float_0_000244140625
                     OpSelectionMerge %123 None
                     OpBranchConditional %119 %121 %122

        %121 =         OpLabel
        %124 =           OpCompositeConstruct %v3float %108 %108 %108   ; RelaxedPrecision
        %125 =           OpFSub %v3float %92 %124                       ; RelaxedPrecision
                         OpStore %126 %101
        %127 =           OpFunctionCall %float %blend_color_saturation_Qhh3 %126
        %128 =           OpVectorTimesScalar %v3float %125 %127     ; RelaxedPrecision
                         OpStore %129 %128
                         OpStore %130 %117
        %131 =           OpFunctionCall %v3float %guarded_divide_Qh3h3h %129 %130
                         OpStore %120 %131
                         OpBranch %123

        %122 =         OpLabel
                         OpStore %120 %132
                         OpBranch %123

        %123 =     OpLabel
        %133 =       OpLoad %v3float %120           ; RelaxedPrecision
                     OpStore %l %133
                     OpStore %r %83
                     OpBranch %106

        %106 = OpLabel
        %140 =   OpLoad %v3float %r                 ; RelaxedPrecision
        %135 =   OpDot %float %139 %140             ; RelaxedPrecision
                 OpStore %_5_lum %135
        %143 =   OpLoad %v3float %l                 ; RelaxedPrecision
        %142 =   OpDot %float %139 %143             ; RelaxedPrecision
        %144 =   OpFSub %float %135 %142            ; RelaxedPrecision
        %145 =   OpLoad %v3float %l                 ; RelaxedPrecision
        %146 =   OpCompositeConstruct %v3float %144 %144 %144   ; RelaxedPrecision
        %147 =   OpFAdd %v3float %146 %145                      ; RelaxedPrecision
                 OpStore %_6_result %147
        %151 =   OpCompositeExtract %float %147 0   ; RelaxedPrecision
        %152 =   OpCompositeExtract %float %147 1   ; RelaxedPrecision
        %150 =   OpExtInst %float %5 FMin %151 %152     ; RelaxedPrecision
        %153 =   OpCompositeExtract %float %147 2       ; RelaxedPrecision
        %149 =   OpExtInst %float %5 FMin %150 %153     ; RelaxedPrecision
                 OpStore %_7_minComp %149
        %156 =   OpExtInst %float %5 FMax %151 %152     ; RelaxedPrecision
        %155 =   OpExtInst %float %5 FMax %156 %153     ; RelaxedPrecision
                 OpStore %_8_maxComp %155
        %157 =   OpFOrdLessThan %bool %149 %float_0
                 OpSelectionMerge %159 None
                 OpBranchConditional %157 %158 %159

        %158 =     OpLabel
        %160 =       OpFUnordNotEqual %bool %135 %149
                     OpBranch %159

        %159 = OpLabel
        %161 =   OpPhi %bool %false %106 %160 %158
                 OpSelectionMerge %163 None
                 OpBranchConditional %161 %162 %163

        %162 =     OpLabel
        %164 =       OpCompositeConstruct %v3float %135 %135 %135   ; RelaxedPrecision
        %165 =       OpFSub %v3float %147 %164                      ; RelaxedPrecision
        %166 =       OpFSub %float %135 %149                        ; RelaxedPrecision
        %168 =       OpFAdd %float %166 %float_6_10351562en05       ; RelaxedPrecision
        %169 =       OpLoad %float %_kGuardedDivideEpsilon          ; RelaxedPrecision
        %170 =       OpFAdd %float %168 %169                        ; RelaxedPrecision
        %171 =       OpFDiv %float %135 %170                        ; RelaxedPrecision
        %172 =       OpVectorTimesScalar %v3float %165 %171         ; RelaxedPrecision
        %173 =       OpFAdd %v3float %164 %172                      ; RelaxedPrecision
                     OpStore %_6_result %173
                     OpBranch %163

        %163 = OpLabel
        %174 =   OpFOrdGreaterThan %bool %155 %71
                 OpSelectionMerge %176 None
                 OpBranchConditional %174 %175 %176

        %175 =     OpLabel
        %177 =       OpFUnordNotEqual %bool %155 %135
                     OpBranch %176

        %176 = OpLabel
        %178 =   OpPhi %bool %false %163 %177 %175
                 OpSelectionMerge %180 None
                 OpBranchConditional %178 %179 %180

        %179 =     OpLabel
        %181 =       OpLoad %v3float %_6_result     ; RelaxedPrecision
        %182 =       OpCompositeConstruct %v3float %135 %135 %135   ; RelaxedPrecision
        %183 =       OpFSub %v3float %181 %182                      ; RelaxedPrecision
        %184 =       OpFSub %float %71 %135                         ; RelaxedPrecision
        %185 =       OpVectorTimesScalar %v3float %183 %184         ; RelaxedPrecision
        %186 =       OpFSub %float %155 %135                        ; RelaxedPrecision
        %187 =       OpFAdd %float %186 %float_6_10351562en05       ; RelaxedPrecision
        %188 =       OpLoad %float %_kGuardedDivideEpsilon          ; RelaxedPrecision
        %189 =       OpFAdd %float %187 %188                        ; RelaxedPrecision
        %190 =       OpFDiv %float %float_1 %189                    ; RelaxedPrecision
        %191 =       OpVectorTimesScalar %v3float %185 %190         ; RelaxedPrecision
        %192 =       OpFAdd %v3float %182 %191                      ; RelaxedPrecision
                     OpStore %_6_result %192
                     OpBranch %180

        %180 = OpLabel
        %193 =   OpLoad %v3float %_6_result         ; RelaxedPrecision
        %194 =   OpLoad %v4float %64                ; RelaxedPrecision
        %195 =   OpVectorShuffle %v3float %194 %194 0 1 2   ; RelaxedPrecision
        %196 =   OpFAdd %v3float %193 %195                  ; RelaxedPrecision
        %197 =   OpFSub %v3float %196 %83                   ; RelaxedPrecision
        %198 =   OpLoad %v4float %63                        ; RelaxedPrecision
        %199 =   OpVectorShuffle %v3float %198 %198 0 1 2   ; RelaxedPrecision
        %200 =   OpFAdd %v3float %197 %199                  ; RelaxedPrecision
        %201 =   OpFSub %v3float %200 %77                   ; RelaxedPrecision
        %202 =   OpCompositeExtract %float %201 0           ; RelaxedPrecision
        %203 =   OpCompositeExtract %float %201 1           ; RelaxedPrecision
        %204 =   OpCompositeExtract %float %201 2           ; RelaxedPrecision
        %205 =   OpLoad %v4float %63                        ; RelaxedPrecision
        %206 =   OpCompositeExtract %float %205 3           ; RelaxedPrecision
        %207 =   OpLoad %v4float %64                        ; RelaxedPrecision
        %208 =   OpCompositeExtract %float %207 3           ; RelaxedPrecision
        %209 =   OpFAdd %float %206 %208                    ; RelaxedPrecision
        %210 =   OpFSub %float %209 %71                     ; RelaxedPrecision
        %211 =   OpCompositeConstruct %v4float %202 %203 %204 %210  ; RelaxedPrecision
                 OpReturnValue %211
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %213

        %214 = OpLabel
        %216 =   OpVariable %_ptr_Function_v2float Function
        %221 =   OpVariable %_ptr_Function_v4float Function
        %225 =   OpVariable %_ptr_Function_v4float Function
         %15 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %15
                 OpStore %216 %215
        %217 =   OpAccessChain %_ptr_Uniform_v4float %21 %int_0
        %220 =   OpLoad %v4float %217               ; RelaxedPrecision
                 OpStore %221 %220
        %222 =   OpAccessChain %_ptr_Uniform_v4float %21 %int_1
        %224 =   OpLoad %v4float %222               ; RelaxedPrecision
                 OpStore %225 %224
        %226 =   OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %216 %221 %225
                 OpStore %sk_FragColor %226
                 OpReturn
               OpFunctionEnd
