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
               OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"    ; id %7
               OpName %dyTerm "dyTerm"                                              ; id %39
               OpName %delta "delta"                                                ; id %52
               OpName %main "main"                                                  ; id %8

               ; Annotations
               OpDecorate %guarded_divide_Qhhh RelaxedPrecision
               OpDecorate %color_burn_component_Qhh2h2 RelaxedPrecision
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
               OpDecorate %dyTerm RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %delta RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
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
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
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
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision

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
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
       %void = OpTypeVoid
        %101 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


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


               ; Function color_burn_component_Qhh2h2
%color_burn_component_Qhh2h2 = OpFunction %float None %35   ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %37 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision

         %38 = OpLabel
     %dyTerm =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %45 =   OpVariable %_ptr_Function_float Function
      %delta =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
         %58 =   OpVariable %_ptr_Function_float Function
         %75 =   OpVariable %_ptr_Function_float Function
         %78 =   OpVariable %_ptr_Function_float Function
         %40 =   OpLoad %v2float %37                ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %40 1    ; RelaxedPrecision
         %42 =   OpLoad %v2float %37                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 0    ; RelaxedPrecision
         %44 =   OpFOrdEqual %bool %41 %43
                 OpSelectionMerge %48 None
                 OpBranchConditional %44 %46 %47

         %46 =     OpLabel
         %49 =       OpLoad %v2float %37            ; RelaxedPrecision
         %50 =       OpCompositeExtract %float %49 1    ; RelaxedPrecision
                     OpStore %45 %50
                     OpBranch %48

         %47 =     OpLabel
                     OpStore %45 %float_0
                     OpBranch %48

         %48 = OpLabel
         %51 =   OpLoad %float %45                  ; RelaxedPrecision
                 OpStore %dyTerm %51
         %54 =   OpLoad %v2float %36                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 0    ; RelaxedPrecision
         %53 =   OpExtInst %float %5 FAbs %55       ; RelaxedPrecision
         %57 =   OpFOrdGreaterThanEqual %bool %53 %float_6_10351562en05
                 OpSelectionMerge %61 None
                 OpBranchConditional %57 %59 %60

         %59 =     OpLabel
         %62 =       OpLoad %v2float %37            ; RelaxedPrecision
         %63 =       OpCompositeExtract %float %62 1    ; RelaxedPrecision
         %65 =       OpLoad %v2float %37                ; RelaxedPrecision
         %66 =       OpCompositeExtract %float %65 1    ; RelaxedPrecision
         %67 =       OpLoad %v2float %37                ; RelaxedPrecision
         %68 =       OpCompositeExtract %float %67 1    ; RelaxedPrecision
         %69 =       OpLoad %v2float %37                ; RelaxedPrecision
         %70 =       OpCompositeExtract %float %69 0    ; RelaxedPrecision
         %71 =       OpFSub %float %68 %70              ; RelaxedPrecision
         %72 =       OpLoad %v2float %36                ; RelaxedPrecision
         %73 =       OpCompositeExtract %float %72 1    ; RelaxedPrecision
         %74 =       OpFMul %float %71 %73              ; RelaxedPrecision
                     OpStore %75 %74
         %76 =       OpLoad %v2float %36            ; RelaxedPrecision
         %77 =       OpCompositeExtract %float %76 0    ; RelaxedPrecision
                     OpStore %78 %77
         %79 =       OpFunctionCall %float %guarded_divide_Qhhh %75 %78
         %64 =       OpExtInst %float %5 FMin %66 %79   ; RelaxedPrecision
         %80 =       OpFSub %float %63 %64              ; RelaxedPrecision
                     OpStore %58 %80
                     OpBranch %61

         %60 =     OpLabel
                     OpStore %58 %51
                     OpBranch %61

         %61 = OpLabel
         %81 =   OpLoad %float %58                  ; RelaxedPrecision
                 OpStore %delta %81
         %82 =   OpLoad %v2float %36                ; RelaxedPrecision
         %83 =   OpCompositeExtract %float %82 1    ; RelaxedPrecision
         %84 =   OpFMul %float %81 %83              ; RelaxedPrecision
         %85 =   OpLoad %v2float %36                ; RelaxedPrecision
         %86 =   OpCompositeExtract %float %85 0    ; RelaxedPrecision
         %88 =   OpLoad %v2float %37                ; RelaxedPrecision
         %89 =   OpCompositeExtract %float %88 1    ; RelaxedPrecision
         %90 =   OpFSub %float %float_1 %89         ; RelaxedPrecision
         %91 =   OpFMul %float %86 %90              ; RelaxedPrecision
         %92 =   OpFAdd %float %84 %91              ; RelaxedPrecision
         %93 =   OpLoad %v2float %37                ; RelaxedPrecision
         %94 =   OpCompositeExtract %float %93 0    ; RelaxedPrecision
         %95 =   OpLoad %v2float %36                ; RelaxedPrecision
         %96 =   OpCompositeExtract %float %95 1    ; RelaxedPrecision
         %97 =   OpFSub %float %float_1 %96         ; RelaxedPrecision
         %98 =   OpFMul %float %94 %97              ; RelaxedPrecision
         %99 =   OpFAdd %float %92 %98              ; RelaxedPrecision
                 OpReturnValue %99
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %101

        %102 = OpLabel
        %108 =   OpVariable %_ptr_Function_v2float Function
        %113 =   OpVariable %_ptr_Function_v2float Function
        %118 =   OpVariable %_ptr_Function_v2float Function
        %122 =   OpVariable %_ptr_Function_v2float Function
        %127 =   OpVariable %_ptr_Function_v2float Function
        %131 =   OpVariable %_ptr_Function_v2float Function
         %14 =   OpSelect %float %false %float_9_99999994en09 %float_0
                 OpStore %_kGuardedDivideEpsilon %14
        %103 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %106 =   OpLoad %v4float %103               ; RelaxedPrecision
        %107 =   OpVectorShuffle %v2float %106 %106 0 3     ; RelaxedPrecision
                 OpStore %108 %107
        %109 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %111 =   OpLoad %v4float %109               ; RelaxedPrecision
        %112 =   OpVectorShuffle %v2float %111 %111 0 3     ; RelaxedPrecision
                 OpStore %113 %112
        %114 =   OpFunctionCall %float %color_burn_component_Qhh2h2 %108 %113
        %115 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %116 =   OpLoad %v4float %115               ; RelaxedPrecision
        %117 =   OpVectorShuffle %v2float %116 %116 1 3     ; RelaxedPrecision
                 OpStore %118 %117
        %119 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %120 =   OpLoad %v4float %119               ; RelaxedPrecision
        %121 =   OpVectorShuffle %v2float %120 %120 1 3     ; RelaxedPrecision
                 OpStore %122 %121
        %123 =   OpFunctionCall %float %color_burn_component_Qhh2h2 %118 %122
        %124 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %125 =   OpLoad %v4float %124               ; RelaxedPrecision
        %126 =   OpVectorShuffle %v2float %125 %125 2 3     ; RelaxedPrecision
                 OpStore %127 %126
        %128 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %129 =   OpLoad %v4float %128               ; RelaxedPrecision
        %130 =   OpVectorShuffle %v2float %129 %129 2 3     ; RelaxedPrecision
                 OpStore %131 %130
        %132 =   OpFunctionCall %float %color_burn_component_Qhh2h2 %127 %131
        %133 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %134 =   OpLoad %v4float %133               ; RelaxedPrecision
        %135 =   OpCompositeExtract %float %134 3   ; RelaxedPrecision
        %136 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_0
        %137 =   OpLoad %v4float %136               ; RelaxedPrecision
        %138 =   OpCompositeExtract %float %137 3   ; RelaxedPrecision
        %139 =   OpFSub %float %float_1 %138        ; RelaxedPrecision
        %140 =   OpAccessChain %_ptr_Uniform_v4float %20 %int_1
        %141 =   OpLoad %v4float %140               ; RelaxedPrecision
        %142 =   OpCompositeExtract %float %141 3   ; RelaxedPrecision
        %143 =   OpFMul %float %139 %142            ; RelaxedPrecision
        %144 =   OpFAdd %float %135 %143            ; RelaxedPrecision
        %145 =   OpCompositeConstruct %v4float %114 %123 %132 %144  ; RelaxedPrecision
                 OpStore %sk_FragColor %145
                 OpReturn
               OpFunctionEnd
