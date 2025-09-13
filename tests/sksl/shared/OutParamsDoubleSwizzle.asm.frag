               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %swizzle_lvalue_h2hhh2h "swizzle_lvalue_h2hhh2h"  ; id %6
               OpName %func_vh4 "func_vh4"                              ; id %7
               OpName %t "t"                                            ; id %45
               OpName %main "main"                                      ; id %8
               OpName %result "result"                                  ; id %64

               ; Annotations
               OpDecorate %swizzle_lvalue_h2hhh2h RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %t RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %27 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %42 = OpTypeFunction %void %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_5 = OpConstant %float 5
         %61 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %66 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
         %70 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function swizzle_lvalue_h2hhh2h
%swizzle_lvalue_h2hhh2h = OpFunction %v2float None %27  ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision
         %30 = OpFunctionParameter %_ptr_Function_v2float   ; RelaxedPrecision
         %31 = OpFunctionParameter %_ptr_Function_float     ; RelaxedPrecision

         %32 = OpLabel
         %33 =   OpLoad %v2float %30                ; RelaxedPrecision
         %34 =   OpLoad %v2float %30                ; RelaxedPrecision
         %35 =   OpVectorShuffle %v2float %34 %33 3 2   ; RelaxedPrecision
                 OpStore %30 %35
         %36 =   OpLoad %float %28                  ; RelaxedPrecision
         %37 =   OpLoad %float %29                  ; RelaxedPrecision
         %38 =   OpFAdd %float %36 %37              ; RelaxedPrecision
         %39 =   OpLoad %float %31                  ; RelaxedPrecision
         %40 =   OpCompositeConstruct %v2float %38 %39  ; RelaxedPrecision
                 OpReturnValue %40
               OpFunctionEnd


               ; Function func_vh4
   %func_vh4 = OpFunction %void None %42
         %43 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %44 = OpLabel
          %t =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
         %47 =   OpVariable %_ptr_Function_float Function
         %49 =   OpVariable %_ptr_Function_float Function
         %52 =   OpVariable %_ptr_Function_v2float Function     ; RelaxedPrecision
         %54 =   OpVariable %_ptr_Function_float Function
                 OpStore %47 %float_1
                 OpStore %49 %float_2
         %50 =   OpLoad %v4float %43                ; RelaxedPrecision
         %51 =   OpVectorShuffle %v2float %50 %50 0 2   ; RelaxedPrecision
                 OpStore %52 %51
                 OpStore %54 %float_5
         %55 =   OpFunctionCall %v2float %swizzle_lvalue_h2hhh2h %47 %49 %52 %54
         %56 =   OpLoad %v2float %52                ; RelaxedPrecision
         %57 =   OpLoad %v4float %43                ; RelaxedPrecision
         %58 =   OpVectorShuffle %v4float %57 %56 4 1 5 3   ; RelaxedPrecision
                 OpStore %43 %58
                 OpStore %t %55
         %59 =   OpLoad %v4float %43                ; RelaxedPrecision
         %60 =   OpVectorShuffle %v4float %59 %55 0 4 2 5   ; RelaxedPrecision
                 OpStore %43 %60
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %61         ; RelaxedPrecision
         %62 = OpFunctionParameter %_ptr_Function_v2float

         %63 = OpLabel
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %67 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %75 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %result %66
                 OpStore %67 %66
         %68 =   OpFunctionCall %void %func_vh4 %67
         %69 =   OpLoad %v4float %67                ; RelaxedPrecision
                 OpStore %result %69
         %71 =   OpFOrdEqual %v4bool %69 %70
         %74 =   OpAll %bool %71
                 OpSelectionMerge %78 None
                 OpBranchConditional %74 %76 %77

         %76 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %82 =       OpLoad %v4float %79            ; RelaxedPrecision
                     OpStore %75 %82
                     OpBranch %78

         %77 =     OpLabel
         %83 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %85 =       OpLoad %v4float %83            ; RelaxedPrecision
                     OpStore %75 %85
                     OpBranch %78

         %78 = OpLabel
         %86 =   OpLoad %v4float %75                ; RelaxedPrecision
                 OpReturnValue %86
               OpFunctionEnd
