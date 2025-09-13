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
               OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"    ; id %6
               OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"  ; id %7
               OpName %dxScale "dxScale"                                            ; id %39
               OpName %delta "delta"                                                ; id %47
               OpName %main "main"                                                  ; id %8

               ; Annotations
               OpDecorate %guarded_divide_Qhhh RelaxedPrecision
               OpDecorate %color_dodge_component_Qhh2h2 RelaxedPrecision
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
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %dxScale RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision

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
%_ptr_Function_float = OpTypePointer Function %float
         %24 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %35 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
         %99 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


               ; Function guarded_divide_Qhhh
%guarded_divide_Qhhh = OpFunction %float None %24   ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %27 = OpLabel
         %28 =   OpLoad %float %25                  ; RelaxedPrecision
         %29 =   OpLoad %float %26                  ; RelaxedPrecision
         %30 =   OpLoad %float %_kGuardedDivideEpsilon  ; RelaxedPrecision
         %31 =   OpFAdd %float %29 %30                  ; RelaxedPrecision
         %32 =   OpFDiv %float %28 %31                  ; RelaxedPrecision
                 OpReturnValue %32
               OpFunctionEnd


               ; Function color_dodge_component_Qhh2h2
%color_dodge_component_Qhh2h2 = OpFunction %float None %35  ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %37 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %38 = OpLabel
    %dxScale =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %delta =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %59 =   OpVariable %_ptr_Function_float Function
         %68 =   OpVariable %_ptr_Function_float Function
         %74 =   OpVariable %_ptr_Function_float Function
         %40 =   OpLoad %v2float %37                ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %40 0    ; RelaxedPrecision
         %42 =   OpFOrdEqual %bool %41 %float_0
         %43 =   OpSelect %int %42 %int_0 %int_1
         %46 =   OpConvertSToF %float %43           ; RelaxedPrecision
                 OpStore %dxScale %46
         %49 =   OpLoad %v2float %37                ; RelaxedPrecision
         %50 =   OpCompositeExtract %float %49 1    ; RelaxedPrecision
         %52 =   OpLoad %v2float %36                ; RelaxedPrecision
         %53 =   OpCompositeExtract %float %52 1    ; RelaxedPrecision
         %54 =   OpLoad %v2float %36                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 0    ; RelaxedPrecision
         %56 =   OpFSub %float %53 %55              ; RelaxedPrecision
         %51 =   OpExtInst %float %5 FAbs %56       ; RelaxedPrecision
         %58 =   OpFOrdGreaterThanEqual %bool %51 %float_6_10351562en05
                 OpSelectionMerge %62 None
                 OpBranchConditional %58 %60 %61

         %60 =     OpLabel
         %63 =       OpLoad %v2float %37            ; RelaxedPrecision
         %64 =       OpCompositeExtract %float %63 0    ; RelaxedPrecision
         %65 =       OpLoad %v2float %36                ; RelaxedPrecision
         %66 =       OpCompositeExtract %float %65 1    ; RelaxedPrecision
         %67 =       OpFMul %float %64 %66              ; RelaxedPrecision
                     OpStore %68 %67
         %69 =       OpLoad %v2float %36            ; RelaxedPrecision
         %70 =       OpCompositeExtract %float %69 1    ; RelaxedPrecision
         %71 =       OpLoad %v2float %36                ; RelaxedPrecision
         %72 =       OpCompositeExtract %float %71 0    ; RelaxedPrecision
         %73 =       OpFSub %float %70 %72              ; RelaxedPrecision
                     OpStore %74 %73
         %75 =       OpFunctionCall %float %guarded_divide_Qhhh %68 %74
                     OpStore %59 %75
                     OpBranch %62

         %61 =     OpLabel
         %76 =       OpLoad %v2float %37            ; RelaxedPrecision
         %77 =       OpCompositeExtract %float %76 1    ; RelaxedPrecision
                     OpStore %59 %77
                     OpBranch %62

         %62 = OpLabel
         %78 =   OpLoad %float %59                  ; RelaxedPrecision
         %48 =   OpExtInst %float %5 FMin %50 %78   ; RelaxedPrecision
         %79 =   OpFMul %float %46 %48              ; RelaxedPrecision
                 OpStore %delta %79
         %80 =   OpLoad %v2float %36                ; RelaxedPrecision
         %81 =   OpCompositeExtract %float %80 1    ; RelaxedPrecision
         %82 =   OpFMul %float %79 %81              ; RelaxedPrecision
         %83 =   OpLoad %v2float %36                ; RelaxedPrecision
         %84 =   OpCompositeExtract %float %83 0    ; RelaxedPrecision
         %86 =   OpLoad %v2float %37                ; RelaxedPrecision
         %87 =   OpCompositeExtract %float %86 1    ; RelaxedPrecision
         %88 =   OpFSub %float %float_1 %87         ; RelaxedPrecision
         %89 =   OpFMul %float %84 %88              ; RelaxedPrecision
         %90 =   OpFAdd %float %82 %89              ; RelaxedPrecision
         %91 =   OpLoad %v2float %37                ; RelaxedPrecision
         %92 =   OpCompositeExtract %float %91 0    ; RelaxedPrecision
         %93 =   OpLoad %v2float %36                ; RelaxedPrecision
         %94 =   OpCompositeExtract %float %93 1    ; RelaxedPrecision
         %95 =   OpFSub %float %float_1 %94         ; RelaxedPrecision
         %96 =   OpFMul %float %92 %95              ; RelaxedPrecision
         %97 =   OpFAdd %float %90 %96              ; RelaxedPrecision
                 OpReturnValue %97
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %99

        %100 = OpLabel
        %105 =   OpVariable %_ptr_Function_v2float Function
        %109 =   OpVariable %_ptr_Function_v2float Function
        %114 =   OpVariable %_ptr_Function_v2float Function
        %118 =   OpVariable %_ptr_Function_v2float Function
        %123 =   OpVariable %_ptr_Function_v2float Function
        %127 =   OpVariable %_ptr_Function_v2float Function
         %14 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %14
        %101 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %103 =   OpLoad %v4float %101               ; RelaxedPrecision
        %104 =   OpVectorShuffle %v2float %103 %103 0 3     ; RelaxedPrecision
                 OpStore %105 %104
        %106 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %107 =   OpLoad %v4float %106               ; RelaxedPrecision
        %108 =   OpVectorShuffle %v2float %107 %107 0 3     ; RelaxedPrecision
                 OpStore %109 %108
        %110 =   OpFunctionCall %float %color_dodge_component_Qhh2h2 %105 %109
        %111 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %112 =   OpLoad %v4float %111               ; RelaxedPrecision
        %113 =   OpVectorShuffle %v2float %112 %112 1 3     ; RelaxedPrecision
                 OpStore %114 %113
        %115 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %116 =   OpLoad %v4float %115               ; RelaxedPrecision
        %117 =   OpVectorShuffle %v2float %116 %116 1 3     ; RelaxedPrecision
                 OpStore %118 %117
        %119 =   OpFunctionCall %float %color_dodge_component_Qhh2h2 %114 %118
        %120 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %121 =   OpLoad %v4float %120               ; RelaxedPrecision
        %122 =   OpVectorShuffle %v2float %121 %121 2 3     ; RelaxedPrecision
                 OpStore %123 %122
        %124 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %125 =   OpLoad %v4float %124               ; RelaxedPrecision
        %126 =   OpVectorShuffle %v2float %125 %125 2 3     ; RelaxedPrecision
                 OpStore %127 %126
        %128 =   OpFunctionCall %float %color_dodge_component_Qhh2h2 %123 %127
        %129 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %130 =   OpLoad %v4float %129               ; RelaxedPrecision
        %131 =   OpCompositeExtract %float %130 3   ; RelaxedPrecision
        %132 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %133 =   OpLoad %v4float %132               ; RelaxedPrecision
        %134 =   OpCompositeExtract %float %133 3   ; RelaxedPrecision
        %135 =   OpFSub %float %float_1 %134        ; RelaxedPrecision
        %136 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %137 =   OpLoad %v4float %136               ; RelaxedPrecision
        %138 =   OpCompositeExtract %float %137 3   ; RelaxedPrecision
        %139 =   OpFMul %float %135 %138            ; RelaxedPrecision
        %140 =   OpFAdd %float %131 %139            ; RelaxedPrecision
        %141 =   OpCompositeConstruct %v4float %110 %119 %128 %140  ; RelaxedPrecision
                 OpStore %sk_FragColor %141
                 OpReturn
               OpFunctionEnd
