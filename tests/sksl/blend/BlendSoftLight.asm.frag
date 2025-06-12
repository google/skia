               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"  ; id %4
               OpName %sk_FragColor "sk_FragColor"                      ; id %12
               OpName %_UniformBuffer "_UniformBuffer"                  ; id %16
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"    ; id %2
               OpName %DSqd "DSqd"                                                  ; id %81
               OpName %DCub "DCub"                                                  ; id %88
               OpName %DaSqd "DaSqd"                                                ; id %92
               OpName %DaCub "DaCub"                                                ; id %98
               OpName %main "main"                                                  ; id %3

               ; Annotations
               OpDecorate %soft_light_component_Qhh2h2 RelaxedPrecision
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
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
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %DSqd RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %DCub RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %DaSqd RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %DaCub RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
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
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision

               ; Types, variables and constants
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
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
    %float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
   %float_16 = OpConstant %float 16
       %void = OpTypeVoid
        %183 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0


               ; Function soft_light_component_Qhh2h2
%soft_light_component_Qhh2h2 = OpFunction %float None %20   ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %22 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %23 = OpLabel
       %DSqd =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %DCub =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %DaSqd =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %DaCub =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %25 =   OpLoad %v2float %21                        ; RelaxedPrecision
         %26 =   OpCompositeExtract %float %25 0            ; RelaxedPrecision
         %27 =   OpFMul %float %float_2 %26                 ; RelaxedPrecision
         %28 =   OpLoad %v2float %21                        ; RelaxedPrecision
         %29 =   OpCompositeExtract %float %28 1            ; RelaxedPrecision
         %30 =   OpFOrdLessThanEqual %bool %27 %29
                 OpSelectionMerge %33 None
                 OpBranchConditional %30 %31 %32

         %31 =     OpLabel
         %34 =       OpLoad %v2float %22            ; RelaxedPrecision
         %35 =       OpCompositeExtract %float %34 0    ; RelaxedPrecision
         %36 =       OpLoad %v2float %22                ; RelaxedPrecision
         %37 =       OpCompositeExtract %float %36 0    ; RelaxedPrecision
         %38 =       OpFMul %float %35 %37              ; RelaxedPrecision
         %39 =       OpLoad %v2float %21                ; RelaxedPrecision
         %40 =       OpCompositeExtract %float %39 1    ; RelaxedPrecision
         %41 =       OpLoad %v2float %21                ; RelaxedPrecision
         %42 =       OpCompositeExtract %float %41 0    ; RelaxedPrecision
         %43 =       OpFMul %float %float_2 %42         ; RelaxedPrecision
         %44 =       OpFSub %float %40 %43              ; RelaxedPrecision
         %45 =       OpFMul %float %38 %44              ; RelaxedPrecision
         %46 =       OpLoad %v2float %22                ; RelaxedPrecision
         %47 =       OpCompositeExtract %float %46 1    ; RelaxedPrecision
         %48 =       OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
         %49 =       OpFAdd %float %47 %48                  ; RelaxedPrecision
         %50 =       OpFDiv %float %45 %49                  ; RelaxedPrecision
         %52 =       OpLoad %v2float %22                    ; RelaxedPrecision
         %53 =       OpCompositeExtract %float %52 1        ; RelaxedPrecision
         %54 =       OpFSub %float %float_1 %53             ; RelaxedPrecision
         %55 =       OpLoad %v2float %21                    ; RelaxedPrecision
         %56 =       OpCompositeExtract %float %55 0        ; RelaxedPrecision
         %57 =       OpFMul %float %54 %56                  ; RelaxedPrecision
         %58 =       OpFAdd %float %50 %57                  ; RelaxedPrecision
         %59 =       OpLoad %v2float %22                    ; RelaxedPrecision
         %60 =       OpCompositeExtract %float %59 0        ; RelaxedPrecision
         %61 =       OpLoad %v2float %21                    ; RelaxedPrecision
         %62 =       OpCompositeExtract %float %61 1        ; RelaxedPrecision
         %63 =       OpFNegate %float %62                   ; RelaxedPrecision
         %64 =       OpLoad %v2float %21                    ; RelaxedPrecision
         %65 =       OpCompositeExtract %float %64 0        ; RelaxedPrecision
         %66 =       OpFMul %float %float_2 %65             ; RelaxedPrecision
         %67 =       OpFAdd %float %63 %66                  ; RelaxedPrecision
         %68 =       OpFAdd %float %67 %float_1             ; RelaxedPrecision
         %69 =       OpFMul %float %60 %68                  ; RelaxedPrecision
         %70 =       OpFAdd %float %58 %69                  ; RelaxedPrecision
                     OpReturnValue %70

         %32 =     OpLabel
         %72 =       OpLoad %v2float %22            ; RelaxedPrecision
         %73 =       OpCompositeExtract %float %72 0    ; RelaxedPrecision
         %74 =       OpFMul %float %float_4 %73         ; RelaxedPrecision
         %75 =       OpLoad %v2float %22                ; RelaxedPrecision
         %76 =       OpCompositeExtract %float %75 1    ; RelaxedPrecision
         %77 =       OpFOrdLessThanEqual %bool %74 %76
                     OpSelectionMerge %80 None
                     OpBranchConditional %77 %78 %79

         %78 =         OpLabel
         %83 =           OpLoad %v2float %22        ; RelaxedPrecision
         %84 =           OpCompositeExtract %float %83 0    ; RelaxedPrecision
         %85 =           OpLoad %v2float %22                ; RelaxedPrecision
         %86 =           OpCompositeExtract %float %85 0    ; RelaxedPrecision
         %87 =           OpFMul %float %84 %86              ; RelaxedPrecision
                         OpStore %DSqd %87
         %89 =           OpLoad %v2float %22        ; RelaxedPrecision
         %90 =           OpCompositeExtract %float %89 0    ; RelaxedPrecision
         %91 =           OpFMul %float %87 %90              ; RelaxedPrecision
                         OpStore %DCub %91
         %93 =           OpLoad %v2float %22        ; RelaxedPrecision
         %94 =           OpCompositeExtract %float %93 1    ; RelaxedPrecision
         %95 =           OpLoad %v2float %22                ; RelaxedPrecision
         %96 =           OpCompositeExtract %float %95 1    ; RelaxedPrecision
         %97 =           OpFMul %float %94 %96              ; RelaxedPrecision
                         OpStore %DaSqd %97
         %99 =           OpLoad %v2float %22        ; RelaxedPrecision
        %100 =           OpCompositeExtract %float %99 1    ; RelaxedPrecision
        %101 =           OpFMul %float %97 %100             ; RelaxedPrecision
                         OpStore %DaCub %101
        %102 =           OpLoad %v2float %21        ; RelaxedPrecision
        %103 =           OpCompositeExtract %float %102 0   ; RelaxedPrecision
        %104 =           OpLoad %v2float %22                ; RelaxedPrecision
        %105 =           OpCompositeExtract %float %104 0   ; RelaxedPrecision
        %107 =           OpLoad %v2float %21                ; RelaxedPrecision
        %108 =           OpCompositeExtract %float %107 1   ; RelaxedPrecision
        %109 =           OpFMul %float %float_3 %108        ; RelaxedPrecision
        %111 =           OpLoad %v2float %21                ; RelaxedPrecision
        %112 =           OpCompositeExtract %float %111 0   ; RelaxedPrecision
        %113 =           OpFMul %float %float_6 %112        ; RelaxedPrecision
        %114 =           OpFSub %float %109 %113            ; RelaxedPrecision
        %115 =           OpFSub %float %114 %float_1        ; RelaxedPrecision
        %116 =           OpFMul %float %105 %115            ; RelaxedPrecision
        %117 =           OpFSub %float %103 %116            ; RelaxedPrecision
        %118 =           OpFMul %float %97 %117             ; RelaxedPrecision
        %120 =           OpLoad %v2float %22                ; RelaxedPrecision
        %121 =           OpCompositeExtract %float %120 1   ; RelaxedPrecision
        %122 =           OpFMul %float %float_12 %121       ; RelaxedPrecision
        %123 =           OpFMul %float %122 %87             ; RelaxedPrecision
        %124 =           OpLoad %v2float %21                ; RelaxedPrecision
        %125 =           OpCompositeExtract %float %124 1   ; RelaxedPrecision
        %126 =           OpLoad %v2float %21                ; RelaxedPrecision
        %127 =           OpCompositeExtract %float %126 0   ; RelaxedPrecision
        %128 =           OpFMul %float %float_2 %127        ; RelaxedPrecision
        %129 =           OpFSub %float %125 %128            ; RelaxedPrecision
        %130 =           OpFMul %float %123 %129            ; RelaxedPrecision
        %131 =           OpFAdd %float %118 %130            ; RelaxedPrecision
        %133 =           OpFMul %float %float_16 %91        ; RelaxedPrecision
        %134 =           OpLoad %v2float %21                ; RelaxedPrecision
        %135 =           OpCompositeExtract %float %134 1   ; RelaxedPrecision
        %136 =           OpLoad %v2float %21                ; RelaxedPrecision
        %137 =           OpCompositeExtract %float %136 0   ; RelaxedPrecision
        %138 =           OpFMul %float %float_2 %137        ; RelaxedPrecision
        %139 =           OpFSub %float %135 %138            ; RelaxedPrecision
        %140 =           OpFMul %float %133 %139            ; RelaxedPrecision
        %141 =           OpFSub %float %131 %140            ; RelaxedPrecision
        %142 =           OpLoad %v2float %21                ; RelaxedPrecision
        %143 =           OpCompositeExtract %float %142 0   ; RelaxedPrecision
        %144 =           OpFMul %float %101 %143            ; RelaxedPrecision
        %145 =           OpFSub %float %141 %144            ; RelaxedPrecision
        %146 =           OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
        %147 =           OpFAdd %float %97 %146                 ; RelaxedPrecision
        %148 =           OpFDiv %float %145 %147                ; RelaxedPrecision
                         OpReturnValue %148

         %79 =         OpLabel
        %149 =           OpLoad %v2float %22        ; RelaxedPrecision
        %150 =           OpCompositeExtract %float %149 0   ; RelaxedPrecision
        %151 =           OpLoad %v2float %21                ; RelaxedPrecision
        %152 =           OpCompositeExtract %float %151 1   ; RelaxedPrecision
        %153 =           OpLoad %v2float %21                ; RelaxedPrecision
        %154 =           OpCompositeExtract %float %153 0   ; RelaxedPrecision
        %155 =           OpFMul %float %float_2 %154        ; RelaxedPrecision
        %156 =           OpFSub %float %152 %155            ; RelaxedPrecision
        %157 =           OpFAdd %float %156 %float_1        ; RelaxedPrecision
        %158 =           OpFMul %float %150 %157            ; RelaxedPrecision
        %159 =           OpLoad %v2float %21                ; RelaxedPrecision
        %160 =           OpCompositeExtract %float %159 0   ; RelaxedPrecision
        %161 =           OpFAdd %float %158 %160            ; RelaxedPrecision
        %163 =           OpLoad %v2float %22                ; RelaxedPrecision
        %164 =           OpCompositeExtract %float %163 1   ; RelaxedPrecision
        %165 =           OpLoad %v2float %22                ; RelaxedPrecision
        %166 =           OpCompositeExtract %float %165 0   ; RelaxedPrecision
        %167 =           OpFMul %float %164 %166            ; RelaxedPrecision
        %162 =           OpExtInst %float %1 Sqrt %167      ; RelaxedPrecision
        %168 =           OpLoad %v2float %21                ; RelaxedPrecision
        %169 =           OpCompositeExtract %float %168 1   ; RelaxedPrecision
        %170 =           OpLoad %v2float %21                ; RelaxedPrecision
        %171 =           OpCompositeExtract %float %170 0   ; RelaxedPrecision
        %172 =           OpFMul %float %float_2 %171        ; RelaxedPrecision
        %173 =           OpFSub %float %169 %172            ; RelaxedPrecision
        %174 =           OpFMul %float %162 %173            ; RelaxedPrecision
        %175 =           OpFSub %float %161 %174            ; RelaxedPrecision
        %176 =           OpLoad %v2float %22                ; RelaxedPrecision
        %177 =           OpCompositeExtract %float %176 1   ; RelaxedPrecision
        %178 =           OpLoad %v2float %21                ; RelaxedPrecision
        %179 =           OpCompositeExtract %float %178 0   ; RelaxedPrecision
        %180 =           OpFMul %float %177 %179            ; RelaxedPrecision
        %181 =           OpFSub %float %175 %180            ; RelaxedPrecision
                         OpReturnValue %181

         %80 =     OpLabel
                     OpBranch %33

         %33 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %183

        %184 = OpLabel
        %192 =   OpVariable %_ptr_Function_v4float Function
        %203 =   OpVariable %_ptr_Function_v2float Function
        %207 =   OpVariable %_ptr_Function_v2float Function
        %212 =   OpVariable %_ptr_Function_v2float Function
        %216 =   OpVariable %_ptr_Function_v2float Function
        %221 =   OpVariable %_ptr_Function_v2float Function
        %225 =   OpVariable %_ptr_Function_v2float Function
          %9 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %9
        %185 =   OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %189 =   OpLoad %v4float %185               ; RelaxedPrecision
        %190 =   OpCompositeExtract %float %189 3   ; RelaxedPrecision
        %191 =   OpFOrdEqual %bool %190 %float_0
                 OpSelectionMerge %196 None
                 OpBranchConditional %191 %194 %195

        %194 =     OpLabel
        %197 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %199 =       OpLoad %v4float %197           ; RelaxedPrecision
                     OpStore %192 %199
                     OpBranch %196

        %195 =     OpLabel
        %200 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %201 =       OpLoad %v4float %200           ; RelaxedPrecision
        %202 =       OpVectorShuffle %v2float %201 %201 0 3     ; RelaxedPrecision
                     OpStore %203 %202
        %204 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %205 =       OpLoad %v4float %204           ; RelaxedPrecision
        %206 =       OpVectorShuffle %v2float %205 %205 0 3     ; RelaxedPrecision
                     OpStore %207 %206
        %208 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %203 %207
        %209 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %210 =       OpLoad %v4float %209           ; RelaxedPrecision
        %211 =       OpVectorShuffle %v2float %210 %210 1 3     ; RelaxedPrecision
                     OpStore %212 %211
        %213 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %214 =       OpLoad %v4float %213           ; RelaxedPrecision
        %215 =       OpVectorShuffle %v2float %214 %214 1 3     ; RelaxedPrecision
                     OpStore %216 %215
        %217 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %212 %216
        %218 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %219 =       OpLoad %v4float %218           ; RelaxedPrecision
        %220 =       OpVectorShuffle %v2float %219 %219 2 3     ; RelaxedPrecision
                     OpStore %221 %220
        %222 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %223 =       OpLoad %v4float %222           ; RelaxedPrecision
        %224 =       OpVectorShuffle %v2float %223 %223 2 3     ; RelaxedPrecision
                     OpStore %225 %224
        %226 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %221 %225
        %227 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %228 =       OpLoad %v4float %227           ; RelaxedPrecision
        %229 =       OpCompositeExtract %float %228 3   ; RelaxedPrecision
        %230 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %231 =       OpLoad %v4float %230           ; RelaxedPrecision
        %232 =       OpCompositeExtract %float %231 3   ; RelaxedPrecision
        %233 =       OpFSub %float %float_1 %232        ; RelaxedPrecision
        %234 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %235 =       OpLoad %v4float %234           ; RelaxedPrecision
        %236 =       OpCompositeExtract %float %235 3   ; RelaxedPrecision
        %237 =       OpFMul %float %233 %236            ; RelaxedPrecision
        %238 =       OpFAdd %float %229 %237            ; RelaxedPrecision
        %239 =       OpCompositeConstruct %v4float %208 %217 %226 %238  ; RelaxedPrecision
                     OpStore %192 %239
                     OpBranch %196

        %196 = OpLabel
        %240 =   OpLoad %v4float %192               ; RelaxedPrecision
                 OpStore %sk_FragColor %240
                 OpReturn
               OpFunctionEnd
