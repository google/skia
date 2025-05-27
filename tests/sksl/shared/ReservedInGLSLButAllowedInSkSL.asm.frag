               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %active "active"
               OpName %centroid "centroid"
               OpName %coherent "coherent"
               OpName %common "common"
               OpName %filter "filter"
               OpName %partition "partition"
               OpName %patch "patch"
               OpName %precise "precise"
               OpName %resource "resource"
               OpName %restrict "restrict"
               OpName %shared "shared"
               OpName %smooth "smooth"
               OpName %subroutine "subroutine"
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
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
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
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
     %active = OpVariable %_ptr_Function_v4float Function
   %centroid = OpVariable %_ptr_Function_v4float Function
   %coherent = OpVariable %_ptr_Function_v4float Function
     %common = OpVariable %_ptr_Function_v4float Function
     %filter = OpVariable %_ptr_Function_v4float Function
  %partition = OpVariable %_ptr_Function_v4float Function
      %patch = OpVariable %_ptr_Function_v4float Function
    %precise = OpVariable %_ptr_Function_v4float Function
   %resource = OpVariable %_ptr_Function_v4float Function
   %restrict = OpVariable %_ptr_Function_v4float Function
     %shared = OpVariable %_ptr_Function_v4float Function
     %smooth = OpVariable %_ptr_Function_v4float Function
 %subroutine = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
               OpStore %active %29
         %31 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %32 = OpLoad %v4float %31
               OpStore %centroid %32
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %35 = OpLoad %v4float %34
               OpStore %coherent %35
         %37 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %37
               OpStore %common %38
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
               OpStore %filter %41
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
               OpStore %partition %44
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 = OpLoad %v4float %46
               OpStore %patch %47
         %49 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %50 = OpLoad %v4float %49
               OpStore %precise %50
         %52 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %53 = OpLoad %v4float %52
               OpStore %resource %53
         %55 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %56 = OpLoad %v4float %55
               OpStore %restrict %56
         %58 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %59 = OpLoad %v4float %58
               OpStore %shared %59
         %61 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 = OpLoad %v4float %61
               OpStore %smooth %62
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
               OpStore %subroutine %65
         %66 = OpFMul %v4float %29 %32
         %67 = OpFMul %v4float %66 %35
         %68 = OpFMul %v4float %67 %38
         %69 = OpFMul %v4float %68 %41
         %70 = OpFMul %v4float %69 %44
         %71 = OpFMul %v4float %70 %47
         %72 = OpFMul %v4float %71 %50
         %73 = OpFMul %v4float %72 %53
         %74 = OpFMul %v4float %73 %56
         %75 = OpFMul %v4float %74 %59
         %76 = OpFMul %v4float %75 %62
         %77 = OpFMul %v4float %76 %65
               OpReturnValue %77
               OpFunctionEnd
