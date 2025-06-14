               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"  ; id %9
               OpName %sk_FragColor "sk_FragColor"                      ; id %17
               OpName %_UniformBuffer "_UniformBuffer"                  ; id %21
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"    ; id %6
               OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"                    ; id %7
               OpName %alpha "alpha"                                                ; id %53
               OpName %sda "sda"                                                    ; id %60
               OpName %dsa "dsa"                                                    ; id %66
               OpName %l "l"                                                        ; id %72
               OpName %r "r"                                                        ; id %81
               OpName %_2_mn "_2_mn"                                                ; id %95
               OpName %_3_mx "_3_mx"                                                ; id %101
               OpName %_4_lum "_4_lum"                                              ; id %120
               OpName %_5_result "_5_result"                                        ; id %127
               OpName %_6_minComp "_6_minComp"                                      ; id %134
               OpName %_7_maxComp "_7_maxComp"                                      ; id %140
               OpName %main "main"                                                  ; id %8

               ; Annotations
               OpDecorate %blend_color_saturation_Qhh3 RelaxedPrecision
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
               OpDecorate %20 Binding 0
               OpDecorate %20 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
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
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %sda RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %dsa RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %_2_mn RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %_3_mx RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %_4_lum RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %_5_result RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %_6_minComp RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %_7_maxComp RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
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
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision

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
         %20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %25 = OpTypeFunction %float %_ptr_Function_v3float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %48 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
        %118 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %125 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
       %void = OpTypeVoid
        %199 = OpTypeFunction %void
        %201 = OpConstantComposite %v2float %float_0 %float_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function blend_color_saturation_Qhh3
%blend_color_saturation_Qhh3 = OpFunction %float None %25   ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v3float   ; RelaxedPrecision

         %27 = OpLabel
         %30 =   OpLoad %v3float %26                ; RelaxedPrecision
         %31 =   OpCompositeExtract %float %30 0    ; RelaxedPrecision
         %32 =   OpLoad %v3float %26                ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1    ; RelaxedPrecision
         %29 =   OpExtInst %float %5 FMax %31 %33   ; RelaxedPrecision
         %34 =   OpLoad %v3float %26                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 2    ; RelaxedPrecision
         %28 =   OpExtInst %float %5 FMax %29 %35   ; RelaxedPrecision
         %38 =   OpLoad %v3float %26                ; RelaxedPrecision
         %39 =   OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %40 =   OpLoad %v3float %26                ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %40 1    ; RelaxedPrecision
         %37 =   OpExtInst %float %5 FMin %39 %41   ; RelaxedPrecision
         %42 =   OpLoad %v3float %26                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 2    ; RelaxedPrecision
         %36 =   OpExtInst %float %5 FMin %37 %43   ; RelaxedPrecision
         %44 =   OpFSub %float %28 %36              ; RelaxedPrecision
                 OpReturnValue %44
               OpFunctionEnd


               ; Function blend_hslc_h4h2h4h4
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %48     ; RelaxedPrecision
         %49 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %50 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %51 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %52 = OpLabel
      %alpha =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
        %sda =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
        %dsa =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
          %l =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %76 =   OpVariable %_ptr_Function_v3float Function
          %r =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
         %85 =   OpVariable %_ptr_Function_v3float Function
      %_2_mn =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %_3_mx =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
        %105 =   OpVariable %_ptr_Function_v3float Function
        %111 =   OpVariable %_ptr_Function_v3float Function
     %_4_lum =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
  %_5_result =   OpVariable %_ptr_Function_v3float Function     ; RelaxedPrecision
 %_6_minComp =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
 %_7_maxComp =   OpVariable %_ptr_Function_float Function       ; RelaxedPrecision
         %55 =   OpLoad %v4float %51                            ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %55 3                ; RelaxedPrecision
         %57 =   OpLoad %v4float %50                            ; RelaxedPrecision
         %58 =   OpCompositeExtract %float %57 3                ; RelaxedPrecision
         %59 =   OpFMul %float %56 %58                          ; RelaxedPrecision
                 OpStore %alpha %59
         %61 =   OpLoad %v4float %50                ; RelaxedPrecision
         %62 =   OpVectorShuffle %v3float %61 %61 0 1 2     ; RelaxedPrecision
         %63 =   OpLoad %v4float %51                        ; RelaxedPrecision
         %64 =   OpCompositeExtract %float %63 3            ; RelaxedPrecision
         %65 =   OpVectorTimesScalar %v3float %62 %64       ; RelaxedPrecision
                 OpStore %sda %65
         %67 =   OpLoad %v4float %51                ; RelaxedPrecision
         %68 =   OpVectorShuffle %v3float %67 %67 0 1 2     ; RelaxedPrecision
         %69 =   OpLoad %v4float %50                        ; RelaxedPrecision
         %70 =   OpCompositeExtract %float %69 3            ; RelaxedPrecision
         %71 =   OpVectorTimesScalar %v3float %68 %70       ; RelaxedPrecision
                 OpStore %dsa %71
         %73 =   OpLoad %v2float %49                ; RelaxedPrecision
         %74 =   OpCompositeExtract %float %73 0    ; RelaxedPrecision
         %75 =   OpFUnordNotEqual %bool %74 %float_0
                 OpSelectionMerge %79 None
                 OpBranchConditional %75 %77 %78

         %77 =     OpLabel
                     OpStore %76 %71
                     OpBranch %79

         %78 =     OpLabel
                     OpStore %76 %65
                     OpBranch %79

         %79 = OpLabel
         %80 =   OpLoad %v3float %76                ; RelaxedPrecision
                 OpStore %l %80
         %82 =   OpLoad %v2float %49                ; RelaxedPrecision
         %83 =   OpCompositeExtract %float %82 0    ; RelaxedPrecision
         %84 =   OpFUnordNotEqual %bool %83 %float_0
                 OpSelectionMerge %88 None
                 OpBranchConditional %84 %86 %87

         %86 =     OpLabel
                     OpStore %85 %65
                     OpBranch %88

         %87 =     OpLabel
                     OpStore %85 %71
                     OpBranch %88

         %88 = OpLabel
         %89 =   OpLoad %v3float %85                ; RelaxedPrecision
                 OpStore %r %89
         %90 =   OpLoad %v2float %49                ; RelaxedPrecision
         %91 =   OpCompositeExtract %float %90 1    ; RelaxedPrecision
         %92 =   OpFUnordNotEqual %bool %91 %float_0
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %98 =       OpCompositeExtract %float %80 0    ; RelaxedPrecision
         %99 =       OpCompositeExtract %float %80 1    ; RelaxedPrecision
         %97 =       OpExtInst %float %5 FMin %98 %99   ; RelaxedPrecision
        %100 =       OpCompositeExtract %float %80 2    ; RelaxedPrecision
         %96 =       OpExtInst %float %5 FMin %97 %100  ; RelaxedPrecision
                     OpStore %_2_mn %96
        %103 =       OpExtInst %float %5 FMax %98 %99   ; RelaxedPrecision
        %102 =       OpExtInst %float %5 FMax %103 %100     ; RelaxedPrecision
                     OpStore %_3_mx %102
        %104 =       OpFOrdGreaterThan %bool %102 %96
                     OpSelectionMerge %108 None
                     OpBranchConditional %104 %106 %107

        %106 =         OpLabel
        %109 =           OpCompositeConstruct %v3float %96 %96 %96  ; RelaxedPrecision
        %110 =           OpFSub %v3float %80 %109                   ; RelaxedPrecision
                         OpStore %111 %89
        %112 =           OpFunctionCall %float %blend_color_saturation_Qhh3 %111
        %113 =           OpVectorTimesScalar %v3float %110 %112     ; RelaxedPrecision
        %114 =           OpFSub %float %102 %96                     ; RelaxedPrecision
        %116 =           OpFDiv %float %float_1 %114                ; RelaxedPrecision
        %117 =           OpVectorTimesScalar %v3float %113 %116     ; RelaxedPrecision
                         OpStore %105 %117
                         OpBranch %108

        %107 =         OpLabel
                         OpStore %105 %118
                         OpBranch %108

        %108 =     OpLabel
        %119 =       OpLoad %v3float %105           ; RelaxedPrecision
                     OpStore %l %119
                     OpStore %r %71
                     OpBranch %94

         %94 = OpLabel
        %126 =   OpLoad %v3float %r                 ; RelaxedPrecision
        %121 =   OpDot %float %125 %126             ; RelaxedPrecision
                 OpStore %_4_lum %121
        %129 =   OpLoad %v3float %l                 ; RelaxedPrecision
        %128 =   OpDot %float %125 %129             ; RelaxedPrecision
        %130 =   OpFSub %float %121 %128            ; RelaxedPrecision
        %131 =   OpLoad %v3float %l                 ; RelaxedPrecision
        %132 =   OpCompositeConstruct %v3float %130 %130 %130   ; RelaxedPrecision
        %133 =   OpFAdd %v3float %132 %131                      ; RelaxedPrecision
                 OpStore %_5_result %133
        %137 =   OpCompositeExtract %float %133 0   ; RelaxedPrecision
        %138 =   OpCompositeExtract %float %133 1   ; RelaxedPrecision
        %136 =   OpExtInst %float %5 FMin %137 %138     ; RelaxedPrecision
        %139 =   OpCompositeExtract %float %133 2       ; RelaxedPrecision
        %135 =   OpExtInst %float %5 FMin %136 %139     ; RelaxedPrecision
                 OpStore %_6_minComp %135
        %142 =   OpExtInst %float %5 FMax %137 %138     ; RelaxedPrecision
        %141 =   OpExtInst %float %5 FMax %142 %139     ; RelaxedPrecision
                 OpStore %_7_maxComp %141
        %143 =   OpFOrdLessThan %bool %135 %float_0
                 OpSelectionMerge %145 None
                 OpBranchConditional %143 %144 %145

        %144 =     OpLabel
        %146 =       OpFUnordNotEqual %bool %121 %135
                     OpBranch %145

        %145 = OpLabel
        %147 =   OpPhi %bool %false %94 %146 %144
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %150 =       OpCompositeConstruct %v3float %121 %121 %121   ; RelaxedPrecision
        %151 =       OpFSub %v3float %133 %150                      ; RelaxedPrecision
        %152 =       OpFSub %float %121 %135                        ; RelaxedPrecision
        %154 =       OpFAdd %float %152 %float_6_10351562en05       ; RelaxedPrecision
        %155 =       OpLoad %float %_kGuardedDivideEpsilon          ; RelaxedPrecision
        %156 =       OpFAdd %float %154 %155                        ; RelaxedPrecision
        %157 =       OpFDiv %float %121 %156                        ; RelaxedPrecision
        %158 =       OpVectorTimesScalar %v3float %151 %157         ; RelaxedPrecision
        %159 =       OpFAdd %v3float %150 %158                      ; RelaxedPrecision
                     OpStore %_5_result %159
                     OpBranch %149

        %149 = OpLabel
        %160 =   OpFOrdGreaterThan %bool %141 %59
                 OpSelectionMerge %162 None
                 OpBranchConditional %160 %161 %162

        %161 =     OpLabel
        %163 =       OpFUnordNotEqual %bool %141 %121
                     OpBranch %162

        %162 = OpLabel
        %164 =   OpPhi %bool %false %149 %163 %161
                 OpSelectionMerge %166 None
                 OpBranchConditional %164 %165 %166

        %165 =     OpLabel
        %167 =       OpLoad %v3float %_5_result     ; RelaxedPrecision
        %168 =       OpCompositeConstruct %v3float %121 %121 %121   ; RelaxedPrecision
        %169 =       OpFSub %v3float %167 %168                      ; RelaxedPrecision
        %170 =       OpFSub %float %59 %121                         ; RelaxedPrecision
        %171 =       OpVectorTimesScalar %v3float %169 %170         ; RelaxedPrecision
        %172 =       OpFSub %float %141 %121                        ; RelaxedPrecision
        %173 =       OpFAdd %float %172 %float_6_10351562en05       ; RelaxedPrecision
        %174 =       OpLoad %float %_kGuardedDivideEpsilon          ; RelaxedPrecision
        %175 =       OpFAdd %float %173 %174                        ; RelaxedPrecision
        %176 =       OpFDiv %float %float_1 %175                    ; RelaxedPrecision
        %177 =       OpVectorTimesScalar %v3float %171 %176         ; RelaxedPrecision
        %178 =       OpFAdd %v3float %168 %177                      ; RelaxedPrecision
                     OpStore %_5_result %178
                     OpBranch %166

        %166 = OpLabel
        %179 =   OpLoad %v3float %_5_result         ; RelaxedPrecision
        %180 =   OpLoad %v4float %51                ; RelaxedPrecision
        %181 =   OpVectorShuffle %v3float %180 %180 0 1 2   ; RelaxedPrecision
        %182 =   OpFAdd %v3float %179 %181                  ; RelaxedPrecision
        %183 =   OpFSub %v3float %182 %71                   ; RelaxedPrecision
        %184 =   OpLoad %v4float %50                        ; RelaxedPrecision
        %185 =   OpVectorShuffle %v3float %184 %184 0 1 2   ; RelaxedPrecision
        %186 =   OpFAdd %v3float %183 %185                  ; RelaxedPrecision
        %187 =   OpFSub %v3float %186 %65                   ; RelaxedPrecision
        %188 =   OpCompositeExtract %float %187 0           ; RelaxedPrecision
        %189 =   OpCompositeExtract %float %187 1           ; RelaxedPrecision
        %190 =   OpCompositeExtract %float %187 2           ; RelaxedPrecision
        %191 =   OpLoad %v4float %50                        ; RelaxedPrecision
        %192 =   OpCompositeExtract %float %191 3           ; RelaxedPrecision
        %193 =   OpLoad %v4float %51                        ; RelaxedPrecision
        %194 =   OpCompositeExtract %float %193 3           ; RelaxedPrecision
        %195 =   OpFAdd %float %192 %194                    ; RelaxedPrecision
        %196 =   OpFSub %float %195 %59                     ; RelaxedPrecision
        %197 =   OpCompositeConstruct %v4float %188 %189 %190 %196  ; RelaxedPrecision
                 OpReturnValue %197
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %199

        %200 = OpLabel
        %202 =   OpVariable %_ptr_Function_v2float Function
        %207 =   OpVariable %_ptr_Function_v4float Function
        %211 =   OpVariable %_ptr_Function_v4float Function
         %14 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %14
                 OpStore %202 %201
        %203 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %206 =   OpLoad %v4float %203               ; RelaxedPrecision
                 OpStore %207 %206
        %208 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %210 =   OpLoad %v4float %208               ; RelaxedPrecision
                 OpStore %211 %210
        %212 =   OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %202 %207 %211
                 OpStore %sk_FragColor %212
                 OpReturn
               OpFunctionEnd
