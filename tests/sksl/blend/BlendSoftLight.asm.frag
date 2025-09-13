               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"  ; id %8
               OpName %sk_FragColor "sk_FragColor"                      ; id %16
               OpName %_UniformBuffer "_UniformBuffer"                  ; id %20
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"    ; id %6
               OpName %DSqd "DSqd"                                                  ; id %85
               OpName %DCub "DCub"                                                  ; id %92
               OpName %DaSqd "DaSqd"                                                ; id %96
               OpName %DaCub "DaCub"                                                ; id %102
               OpName %main "main"                                                  ; id %7

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
               OpDecorate %19 Binding 0
               OpDecorate %19 DescriptorSet 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
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
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
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
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %DSqd RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %DCub RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %DaSqd RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %DaCub RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
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
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision

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
         %19 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
    %float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
   %float_16 = OpConstant %float 16
       %void = OpTypeVoid
        %187 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0


               ; Function soft_light_component_Qhh2h2
%soft_light_component_Qhh2h2 = OpFunction %float None %24   ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %27 = OpLabel
       %DSqd =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
       %DCub =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %DaSqd =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %DaCub =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %29 =   OpLoad %v2float %25                        ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 0            ; RelaxedPrecision
         %31 =   OpFMul %float %float_2 %30                 ; RelaxedPrecision
         %32 =   OpLoad %v2float %25                        ; RelaxedPrecision
         %33 =   OpCompositeExtract %float %32 1            ; RelaxedPrecision
         %34 =   OpFOrdLessThanEqual %bool %31 %33
                 OpSelectionMerge %37 None
                 OpBranchConditional %34 %35 %36

         %35 =     OpLabel
         %38 =       OpLoad %v2float %26            ; RelaxedPrecision
         %39 =       OpCompositeExtract %float %38 0    ; RelaxedPrecision
         %40 =       OpLoad %v2float %26                ; RelaxedPrecision
         %41 =       OpCompositeExtract %float %40 0    ; RelaxedPrecision
         %42 =       OpFMul %float %39 %41              ; RelaxedPrecision
         %43 =       OpLoad %v2float %25                ; RelaxedPrecision
         %44 =       OpCompositeExtract %float %43 1    ; RelaxedPrecision
         %45 =       OpLoad %v2float %25                ; RelaxedPrecision
         %46 =       OpCompositeExtract %float %45 0    ; RelaxedPrecision
         %47 =       OpFMul %float %float_2 %46         ; RelaxedPrecision
         %48 =       OpFSub %float %44 %47              ; RelaxedPrecision
         %49 =       OpFMul %float %42 %48              ; RelaxedPrecision
         %50 =       OpLoad %v2float %26                ; RelaxedPrecision
         %51 =       OpCompositeExtract %float %50 1    ; RelaxedPrecision
         %52 =       OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
         %53 =       OpFAdd %float %51 %52                  ; RelaxedPrecision
         %54 =       OpFDiv %float %49 %53                  ; RelaxedPrecision
         %56 =       OpLoad %v2float %26                    ; RelaxedPrecision
         %57 =       OpCompositeExtract %float %56 1        ; RelaxedPrecision
         %58 =       OpFSub %float %float_1 %57             ; RelaxedPrecision
         %59 =       OpLoad %v2float %25                    ; RelaxedPrecision
         %60 =       OpCompositeExtract %float %59 0        ; RelaxedPrecision
         %61 =       OpFMul %float %58 %60                  ; RelaxedPrecision
         %62 =       OpFAdd %float %54 %61                  ; RelaxedPrecision
         %63 =       OpLoad %v2float %26                    ; RelaxedPrecision
         %64 =       OpCompositeExtract %float %63 0        ; RelaxedPrecision
         %65 =       OpLoad %v2float %25                    ; RelaxedPrecision
         %66 =       OpCompositeExtract %float %65 1        ; RelaxedPrecision
         %67 =       OpFNegate %float %66                   ; RelaxedPrecision
         %68 =       OpLoad %v2float %25                    ; RelaxedPrecision
         %69 =       OpCompositeExtract %float %68 0        ; RelaxedPrecision
         %70 =       OpFMul %float %float_2 %69             ; RelaxedPrecision
         %71 =       OpFAdd %float %67 %70                  ; RelaxedPrecision
         %72 =       OpFAdd %float %71 %float_1             ; RelaxedPrecision
         %73 =       OpFMul %float %64 %72                  ; RelaxedPrecision
         %74 =       OpFAdd %float %62 %73                  ; RelaxedPrecision
                     OpReturnValue %74

         %36 =     OpLabel
         %76 =       OpLoad %v2float %26            ; RelaxedPrecision
         %77 =       OpCompositeExtract %float %76 0    ; RelaxedPrecision
         %78 =       OpFMul %float %float_4 %77         ; RelaxedPrecision
         %79 =       OpLoad %v2float %26                ; RelaxedPrecision
         %80 =       OpCompositeExtract %float %79 1    ; RelaxedPrecision
         %81 =       OpFOrdLessThanEqual %bool %78 %80
                     OpSelectionMerge %84 None
                     OpBranchConditional %81 %82 %83

         %82 =         OpLabel
         %87 =           OpLoad %v2float %26        ; RelaxedPrecision
         %88 =           OpCompositeExtract %float %87 0    ; RelaxedPrecision
         %89 =           OpLoad %v2float %26                ; RelaxedPrecision
         %90 =           OpCompositeExtract %float %89 0    ; RelaxedPrecision
         %91 =           OpFMul %float %88 %90              ; RelaxedPrecision
                         OpStore %DSqd %91
         %93 =           OpLoad %v2float %26        ; RelaxedPrecision
         %94 =           OpCompositeExtract %float %93 0    ; RelaxedPrecision
         %95 =           OpFMul %float %91 %94              ; RelaxedPrecision
                         OpStore %DCub %95
         %97 =           OpLoad %v2float %26        ; RelaxedPrecision
         %98 =           OpCompositeExtract %float %97 1    ; RelaxedPrecision
         %99 =           OpLoad %v2float %26                ; RelaxedPrecision
        %100 =           OpCompositeExtract %float %99 1    ; RelaxedPrecision
        %101 =           OpFMul %float %98 %100             ; RelaxedPrecision
                         OpStore %DaSqd %101
        %103 =           OpLoad %v2float %26        ; RelaxedPrecision
        %104 =           OpCompositeExtract %float %103 1   ; RelaxedPrecision
        %105 =           OpFMul %float %101 %104            ; RelaxedPrecision
                         OpStore %DaCub %105
        %106 =           OpLoad %v2float %25        ; RelaxedPrecision
        %107 =           OpCompositeExtract %float %106 0   ; RelaxedPrecision
        %108 =           OpLoad %v2float %26                ; RelaxedPrecision
        %109 =           OpCompositeExtract %float %108 0   ; RelaxedPrecision
        %111 =           OpLoad %v2float %25                ; RelaxedPrecision
        %112 =           OpCompositeExtract %float %111 1   ; RelaxedPrecision
        %113 =           OpFMul %float %float_3 %112        ; RelaxedPrecision
        %115 =           OpLoad %v2float %25                ; RelaxedPrecision
        %116 =           OpCompositeExtract %float %115 0   ; RelaxedPrecision
        %117 =           OpFMul %float %float_6 %116        ; RelaxedPrecision
        %118 =           OpFSub %float %113 %117            ; RelaxedPrecision
        %119 =           OpFSub %float %118 %float_1        ; RelaxedPrecision
        %120 =           OpFMul %float %109 %119            ; RelaxedPrecision
        %121 =           OpFSub %float %107 %120            ; RelaxedPrecision
        %122 =           OpFMul %float %101 %121            ; RelaxedPrecision
        %124 =           OpLoad %v2float %26                ; RelaxedPrecision
        %125 =           OpCompositeExtract %float %124 1   ; RelaxedPrecision
        %126 =           OpFMul %float %float_12 %125       ; RelaxedPrecision
        %127 =           OpFMul %float %126 %91             ; RelaxedPrecision
        %128 =           OpLoad %v2float %25                ; RelaxedPrecision
        %129 =           OpCompositeExtract %float %128 1   ; RelaxedPrecision
        %130 =           OpLoad %v2float %25                ; RelaxedPrecision
        %131 =           OpCompositeExtract %float %130 0   ; RelaxedPrecision
        %132 =           OpFMul %float %float_2 %131        ; RelaxedPrecision
        %133 =           OpFSub %float %129 %132            ; RelaxedPrecision
        %134 =           OpFMul %float %127 %133            ; RelaxedPrecision
        %135 =           OpFAdd %float %122 %134            ; RelaxedPrecision
        %137 =           OpFMul %float %float_16 %95        ; RelaxedPrecision
        %138 =           OpLoad %v2float %25                ; RelaxedPrecision
        %139 =           OpCompositeExtract %float %138 1   ; RelaxedPrecision
        %140 =           OpLoad %v2float %25                ; RelaxedPrecision
        %141 =           OpCompositeExtract %float %140 0   ; RelaxedPrecision
        %142 =           OpFMul %float %float_2 %141        ; RelaxedPrecision
        %143 =           OpFSub %float %139 %142            ; RelaxedPrecision
        %144 =           OpFMul %float %137 %143            ; RelaxedPrecision
        %145 =           OpFSub %float %135 %144            ; RelaxedPrecision
        %146 =           OpLoad %v2float %25                ; RelaxedPrecision
        %147 =           OpCompositeExtract %float %146 0   ; RelaxedPrecision
        %148 =           OpFMul %float %105 %147            ; RelaxedPrecision
        %149 =           OpFSub %float %145 %148            ; RelaxedPrecision
        %150 =           OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
        %151 =           OpFAdd %float %101 %150                ; RelaxedPrecision
        %152 =           OpFDiv %float %149 %151                ; RelaxedPrecision
                         OpReturnValue %152

         %83 =         OpLabel
        %153 =           OpLoad %v2float %26        ; RelaxedPrecision
        %154 =           OpCompositeExtract %float %153 0   ; RelaxedPrecision
        %155 =           OpLoad %v2float %25                ; RelaxedPrecision
        %156 =           OpCompositeExtract %float %155 1   ; RelaxedPrecision
        %157 =           OpLoad %v2float %25                ; RelaxedPrecision
        %158 =           OpCompositeExtract %float %157 0   ; RelaxedPrecision
        %159 =           OpFMul %float %float_2 %158        ; RelaxedPrecision
        %160 =           OpFSub %float %156 %159            ; RelaxedPrecision
        %161 =           OpFAdd %float %160 %float_1        ; RelaxedPrecision
        %162 =           OpFMul %float %154 %161            ; RelaxedPrecision
        %163 =           OpLoad %v2float %25                ; RelaxedPrecision
        %164 =           OpCompositeExtract %float %163 0   ; RelaxedPrecision
        %165 =           OpFAdd %float %162 %164            ; RelaxedPrecision
        %167 =           OpLoad %v2float %26                ; RelaxedPrecision
        %168 =           OpCompositeExtract %float %167 1   ; RelaxedPrecision
        %169 =           OpLoad %v2float %26                ; RelaxedPrecision
        %170 =           OpCompositeExtract %float %169 0   ; RelaxedPrecision
        %171 =           OpFMul %float %168 %170            ; RelaxedPrecision
        %166 =           OpExtInst %float %5 Sqrt %171      ; RelaxedPrecision
        %172 =           OpLoad %v2float %25                ; RelaxedPrecision
        %173 =           OpCompositeExtract %float %172 1   ; RelaxedPrecision
        %174 =           OpLoad %v2float %25                ; RelaxedPrecision
        %175 =           OpCompositeExtract %float %174 0   ; RelaxedPrecision
        %176 =           OpFMul %float %float_2 %175        ; RelaxedPrecision
        %177 =           OpFSub %float %173 %176            ; RelaxedPrecision
        %178 =           OpFMul %float %166 %177            ; RelaxedPrecision
        %179 =           OpFSub %float %165 %178            ; RelaxedPrecision
        %180 =           OpLoad %v2float %26                ; RelaxedPrecision
        %181 =           OpCompositeExtract %float %180 1   ; RelaxedPrecision
        %182 =           OpLoad %v2float %25                ; RelaxedPrecision
        %183 =           OpCompositeExtract %float %182 0   ; RelaxedPrecision
        %184 =           OpFMul %float %181 %183            ; RelaxedPrecision
        %185 =           OpFSub %float %179 %184            ; RelaxedPrecision
                         OpReturnValue %185

         %84 =     OpLabel
                     OpBranch %37

         %37 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %187

        %188 = OpLabel
        %195 =   OpVariable %_ptr_Function_v4float Function
        %206 =   OpVariable %_ptr_Function_v2float Function
        %210 =   OpVariable %_ptr_Function_v2float Function
        %215 =   OpVariable %_ptr_Function_v2float Function
        %219 =   OpVariable %_ptr_Function_v2float Function
        %224 =   OpVariable %_ptr_Function_v2float Function
        %228 =   OpVariable %_ptr_Function_v2float Function
         %13 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %13
        %189 =   OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %192 =   OpLoad %v4float %189               ; RelaxedPrecision
        %193 =   OpCompositeExtract %float %192 3   ; RelaxedPrecision
        %194 =   OpFOrdEqual %bool %193 %float_0
                 OpSelectionMerge %199 None
                 OpBranchConditional %194 %197 %198

        %197 =     OpLabel
        %200 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %202 =       OpLoad %v4float %200           ; RelaxedPrecision
                     OpStore %195 %202
                     OpBranch %199

        %198 =     OpLabel
        %203 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %204 =       OpLoad %v4float %203           ; RelaxedPrecision
        %205 =       OpVectorShuffle %v2float %204 %204 0 3     ; RelaxedPrecision
                     OpStore %206 %205
        %207 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %208 =       OpLoad %v4float %207           ; RelaxedPrecision
        %209 =       OpVectorShuffle %v2float %208 %208 0 3     ; RelaxedPrecision
                     OpStore %210 %209
        %211 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %206 %210
        %212 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %213 =       OpLoad %v4float %212           ; RelaxedPrecision
        %214 =       OpVectorShuffle %v2float %213 %213 1 3     ; RelaxedPrecision
                     OpStore %215 %214
        %216 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %217 =       OpLoad %v4float %216           ; RelaxedPrecision
        %218 =       OpVectorShuffle %v2float %217 %217 1 3     ; RelaxedPrecision
                     OpStore %219 %218
        %220 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %215 %219
        %221 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %222 =       OpLoad %v4float %221           ; RelaxedPrecision
        %223 =       OpVectorShuffle %v2float %222 %222 2 3     ; RelaxedPrecision
                     OpStore %224 %223
        %225 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %226 =       OpLoad %v4float %225           ; RelaxedPrecision
        %227 =       OpVectorShuffle %v2float %226 %226 2 3     ; RelaxedPrecision
                     OpStore %228 %227
        %229 =       OpFunctionCall %float %soft_light_component_Qhh2h2 %224 %228
        %230 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %231 =       OpLoad %v4float %230           ; RelaxedPrecision
        %232 =       OpCompositeExtract %float %231 3   ; RelaxedPrecision
        %233 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_0
        %234 =       OpLoad %v4float %233           ; RelaxedPrecision
        %235 =       OpCompositeExtract %float %234 3   ; RelaxedPrecision
        %236 =       OpFSub %float %float_1 %235        ; RelaxedPrecision
        %237 =       OpAccessChain %_ptr_Uniform_v4float %19 %int_1
        %238 =       OpLoad %v4float %237           ; RelaxedPrecision
        %239 =       OpCompositeExtract %float %238 3   ; RelaxedPrecision
        %240 =       OpFMul %float %236 %239            ; RelaxedPrecision
        %241 =       OpFAdd %float %232 %240            ; RelaxedPrecision
        %242 =       OpCompositeConstruct %v4float %211 %220 %229 %241  ; RelaxedPrecision
                     OpStore %195 %242
                     OpBranch %199

        %199 = OpLabel
        %243 =   OpLoad %v4float %195               ; RelaxedPrecision
                 OpStore %sk_FragColor %243
                 OpReturn
               OpFunctionEnd
