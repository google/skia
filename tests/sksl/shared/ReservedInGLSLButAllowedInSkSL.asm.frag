               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %active "active"                  ; id %27
               OpName %centroid "centroid"              ; id %33
               OpName %coherent "coherent"              ; id %36
               OpName %common "common"                  ; id %39
               OpName %filter "filter"                  ; id %42
               OpName %partition "partition"            ; id %45
               OpName %patch "patch"                    ; id %48
               OpName %precise "precise"                ; id %51
               OpName %resource "resource"              ; id %54
               OpName %restrict "restrict"              ; id %57
               OpName %shared "shared"                  ; id %60
               OpName %smooth "smooth"                  ; id %63
               OpName %subroutine "subroutine"          ; id %66

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %active RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %centroid RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %coherent RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %common RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %filter RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %partition RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %patch RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %precise RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %resource RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %restrict RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %shared RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %smooth RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %subroutine RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
     %active =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %centroid =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %coherent =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %common =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %filter =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
  %partition =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
      %patch =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
    %precise =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %resource =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %restrict =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %shared =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %smooth =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %subroutine =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %29 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
                 OpStore %active %32
         %34 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %35 =   OpLoad %v4float %34                ; RelaxedPrecision
                 OpStore %centroid %35
         %37 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %38 =   OpLoad %v4float %37                ; RelaxedPrecision
                 OpStore %coherent %38
         %40 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
                 OpStore %common %41
         %43 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =   OpLoad %v4float %43                ; RelaxedPrecision
                 OpStore %filter %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpStore %partition %47
         %49 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
                 OpStore %patch %50
         %52 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %53 =   OpLoad %v4float %52                ; RelaxedPrecision
                 OpStore %precise %53
         %55 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %56 =   OpLoad %v4float %55                ; RelaxedPrecision
                 OpStore %resource %56
         %58 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %59 =   OpLoad %v4float %58                ; RelaxedPrecision
                 OpStore %restrict %59
         %61 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %62 =   OpLoad %v4float %61                ; RelaxedPrecision
                 OpStore %shared %62
         %64 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %65 =   OpLoad %v4float %64                ; RelaxedPrecision
                 OpStore %smooth %65
         %67 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =   OpLoad %v4float %67                ; RelaxedPrecision
                 OpStore %subroutine %68
         %69 =   OpFMul %v4float %32 %35            ; RelaxedPrecision
         %70 =   OpFMul %v4float %69 %38            ; RelaxedPrecision
         %71 =   OpFMul %v4float %70 %41            ; RelaxedPrecision
         %72 =   OpFMul %v4float %71 %44            ; RelaxedPrecision
         %73 =   OpFMul %v4float %72 %47            ; RelaxedPrecision
         %74 =   OpFMul %v4float %73 %50            ; RelaxedPrecision
         %75 =   OpFMul %v4float %74 %53            ; RelaxedPrecision
         %76 =   OpFMul %v4float %75 %56            ; RelaxedPrecision
         %77 =   OpFMul %v4float %76 %59            ; RelaxedPrecision
         %78 =   OpFMul %v4float %77 %62            ; RelaxedPrecision
         %79 =   OpFMul %v4float %78 %65            ; RelaxedPrecision
         %80 =   OpFMul %v4float %79 %68            ; RelaxedPrecision
                 OpReturnValue %80
               OpFunctionEnd
