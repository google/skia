               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %active "active"                  ; id %23
               OpName %centroid "centroid"              ; id %30
               OpName %coherent "coherent"              ; id %33
               OpName %common "common"                  ; id %36
               OpName %filter "filter"                  ; id %39
               OpName %partition "partition"            ; id %42
               OpName %patch "patch"                    ; id %45
               OpName %precise "precise"                ; id %48
               OpName %resource "resource"              ; id %51
               OpName %restrict "restrict"              ; id %54
               OpName %shared "shared"                  ; id %57
               OpName %smooth "smooth"                  ; id %60
               OpName %subroutine "subroutine"          ; id %63

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %active RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %centroid RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %coherent RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %common RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %filter RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %partition RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %patch RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %precise RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %resource RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %restrict RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %shared RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %smooth RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %subroutine RelaxedPrecision
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
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
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
         %25 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 =   OpLoad %v4float %25                ; RelaxedPrecision
                 OpStore %active %29
         %31 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 =   OpLoad %v4float %31                ; RelaxedPrecision
                 OpStore %centroid %32
         %34 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %35 =   OpLoad %v4float %34                ; RelaxedPrecision
                 OpStore %coherent %35
         %37 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 =   OpLoad %v4float %37                ; RelaxedPrecision
                 OpStore %common %38
         %40 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
                 OpStore %filter %41
         %43 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 =   OpLoad %v4float %43                ; RelaxedPrecision
                 OpStore %partition %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
                 OpStore %patch %47
         %49 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
                 OpStore %precise %50
         %52 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %53 =   OpLoad %v4float %52                ; RelaxedPrecision
                 OpStore %resource %53
         %55 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %56 =   OpLoad %v4float %55                ; RelaxedPrecision
                 OpStore %restrict %56
         %58 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %59 =   OpLoad %v4float %58                ; RelaxedPrecision
                 OpStore %shared %59
         %61 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 =   OpLoad %v4float %61                ; RelaxedPrecision
                 OpStore %smooth %62
         %64 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 =   OpLoad %v4float %64                ; RelaxedPrecision
                 OpStore %subroutine %65
         %66 =   OpFMul %v4float %29 %32            ; RelaxedPrecision
         %67 =   OpFMul %v4float %66 %35            ; RelaxedPrecision
         %68 =   OpFMul %v4float %67 %38            ; RelaxedPrecision
         %69 =   OpFMul %v4float %68 %41            ; RelaxedPrecision
         %70 =   OpFMul %v4float %69 %44            ; RelaxedPrecision
         %71 =   OpFMul %v4float %70 %47            ; RelaxedPrecision
         %72 =   OpFMul %v4float %71 %50            ; RelaxedPrecision
         %73 =   OpFMul %v4float %72 %53            ; RelaxedPrecision
         %74 =   OpFMul %v4float %73 %56            ; RelaxedPrecision
         %75 =   OpFMul %v4float %74 %59            ; RelaxedPrecision
         %76 =   OpFMul %v4float %75 %62            ; RelaxedPrecision
         %77 =   OpFMul %v4float %76 %65            ; RelaxedPrecision
                 OpReturnValue %77
               OpFunctionEnd
