               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %swizzle_lvalue_h2hhh2h "swizzle_lvalue_h2hhh2h"
               OpName %func_vh4 "func_vh4"
               OpName %t "t"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %t RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %38 = OpTypeFunction %void %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_5 = OpConstant %float 5
         %57 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %62 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
         %66 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%swizzle_lvalue_h2hhh2h = OpFunction %v2float None %23
         %24 = OpFunctionParameter %_ptr_Function_float
         %25 = OpFunctionParameter %_ptr_Function_float
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
         %29 = OpLoad %v2float %26
         %30 = OpLoad %v2float %26
         %31 = OpVectorShuffle %v2float %30 %29 3 2
               OpStore %26 %31
         %32 = OpLoad %float %24
         %33 = OpLoad %float %25
         %34 = OpFAdd %float %32 %33
         %35 = OpLoad %float %27
         %36 = OpCompositeConstruct %v2float %34 %35
               OpReturnValue %36
               OpFunctionEnd
   %func_vh4 = OpFunction %void None %38
         %39 = OpFunctionParameter %_ptr_Function_v4float
         %40 = OpLabel
          %t = OpVariable %_ptr_Function_v2float Function
         %43 = OpVariable %_ptr_Function_float Function
         %45 = OpVariable %_ptr_Function_float Function
         %48 = OpVariable %_ptr_Function_v2float Function
         %50 = OpVariable %_ptr_Function_float Function
               OpStore %43 %float_1
               OpStore %45 %float_2
         %46 = OpLoad %v4float %39
         %47 = OpVectorShuffle %v2float %46 %46 0 2
               OpStore %48 %47
               OpStore %50 %float_5
         %51 = OpFunctionCall %v2float %swizzle_lvalue_h2hhh2h %43 %45 %48 %50
         %52 = OpLoad %v2float %48
         %53 = OpLoad %v4float %39
         %54 = OpVectorShuffle %v4float %53 %52 4 1 5 3
               OpStore %39 %54
               OpStore %t %51
         %55 = OpLoad %v4float %39
         %56 = OpVectorShuffle %v4float %55 %51 0 4 2 5
               OpStore %39 %56
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %57
         %58 = OpFunctionParameter %_ptr_Function_v2float
         %59 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %63 = OpVariable %_ptr_Function_v4float Function
         %71 = OpVariable %_ptr_Function_v4float Function
               OpStore %result %62
               OpStore %63 %62
         %64 = OpFunctionCall %void %func_vh4 %63
         %65 = OpLoad %v4float %63
               OpStore %result %65
         %67 = OpFOrdEqual %v4bool %65 %66
         %70 = OpAll %bool %67
               OpSelectionMerge %74 None
               OpBranchConditional %70 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %79 = OpLoad %v4float %75
               OpStore %71 %79
               OpBranch %74
         %73 = OpLabel
         %80 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %82 = OpLoad %v4float %80
               OpStore %71 %82
               OpBranch %74
         %74 = OpLabel
         %83 = OpLoad %v4float %71
               OpReturnValue %83
               OpFunctionEnd
