               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %t RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %26 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %41 = OpTypeFunction %void %_ptr_Function_v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_5 = OpConstant %float 5
         %60 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %65 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
         %69 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%swizzle_lvalue_h2hhh2h = OpFunction %v2float None %26
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpFunctionParameter %_ptr_Function_float
         %29 = OpFunctionParameter %_ptr_Function_v2float
         %30 = OpFunctionParameter %_ptr_Function_float
         %31 = OpLabel
         %32 = OpLoad %v2float %29
         %33 = OpLoad %v2float %29
         %34 = OpVectorShuffle %v2float %33 %32 3 2
               OpStore %29 %34
         %35 = OpLoad %float %27
         %36 = OpLoad %float %28
         %37 = OpFAdd %float %35 %36
         %38 = OpLoad %float %30
         %39 = OpCompositeConstruct %v2float %37 %38
               OpReturnValue %39
               OpFunctionEnd
   %func_vh4 = OpFunction %void None %41
         %42 = OpFunctionParameter %_ptr_Function_v4float
         %43 = OpLabel
          %t = OpVariable %_ptr_Function_v2float Function
         %46 = OpVariable %_ptr_Function_float Function
         %48 = OpVariable %_ptr_Function_float Function
         %51 = OpVariable %_ptr_Function_v2float Function
         %53 = OpVariable %_ptr_Function_float Function
               OpStore %46 %float_1
               OpStore %48 %float_2
         %49 = OpLoad %v4float %42
         %50 = OpVectorShuffle %v2float %49 %49 0 2
               OpStore %51 %50
               OpStore %53 %float_5
         %54 = OpFunctionCall %v2float %swizzle_lvalue_h2hhh2h %46 %48 %51 %53
         %55 = OpLoad %v2float %51
         %56 = OpLoad %v4float %42
         %57 = OpVectorShuffle %v4float %56 %55 4 1 5 3
               OpStore %42 %57
               OpStore %t %54
         %58 = OpLoad %v4float %42
         %59 = OpVectorShuffle %v4float %58 %54 0 4 2 5
               OpStore %42 %59
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %60
         %61 = OpFunctionParameter %_ptr_Function_v2float
         %62 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %66 = OpVariable %_ptr_Function_v4float Function
         %73 = OpVariable %_ptr_Function_v4float Function
               OpStore %result %65
               OpStore %66 %65
         %67 = OpFunctionCall %void %func_vh4 %66
         %68 = OpLoad %v4float %66
               OpStore %result %68
         %70 = OpFOrdEqual %v4bool %68 %69
         %72 = OpAll %bool %70
               OpSelectionMerge %76 None
               OpBranchConditional %72 %74 %75
         %74 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %81 = OpLoad %v4float %77
               OpStore %73 %81
               OpBranch %76
         %75 = OpLabel
         %82 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %84 = OpLoad %v4float %82
               OpStore %73 %84
               OpBranch %76
         %76 = OpLabel
         %85 = OpLoad %v4float %73
               OpReturnValue %85
               OpFunctionEnd
