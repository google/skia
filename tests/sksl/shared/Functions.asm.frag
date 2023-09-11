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
               OpName %foo_ff2 "foo_ff2"
               OpName %bar_vf "bar_vf"
               OpName %y "y"
               OpName %main "main"
               OpName %x "x"
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
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
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
         %25 = OpTypeFunction %float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
         %34 = OpTypeFunction %void %_ptr_Function_float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
      %int_1 = OpConstant %int 1
         %57 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_10 = OpConstant %float 10
  %float_200 = OpConstant %float 200
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
    %foo_ff2 = OpFunction %float None %25
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
         %28 = OpLoad %v2float %26
         %29 = OpCompositeExtract %float %28 0
         %30 = OpLoad %v2float %26
         %31 = OpCompositeExtract %float %30 1
         %32 = OpFMul %float %29 %31
               OpReturnValue %32
               OpFunctionEnd
     %bar_vf = OpFunction %void None %34
         %35 = OpFunctionParameter %_ptr_Function_float
         %36 = OpLabel
          %y = OpVariable %_ptr_Function__arr_float_int_2 Function
         %55 = OpVariable %_ptr_Function_v2float Function
         %42 = OpLoad %float %35
         %44 = OpAccessChain %_ptr_Function_float %y %int_0
               OpStore %44 %42
         %45 = OpLoad %float %35
         %47 = OpFMul %float %45 %float_2
         %49 = OpAccessChain %_ptr_Function_float %y %int_1
               OpStore %49 %47
         %50 = OpAccessChain %_ptr_Function_float %y %int_0
         %51 = OpLoad %float %50
         %52 = OpAccessChain %_ptr_Function_float %y %int_1
         %53 = OpLoad %float %52
         %54 = OpCompositeConstruct %v2float %51 %53
               OpStore %55 %54
         %56 = OpFunctionCall %float %foo_ff2 %55
               OpStore %35 %56
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %57
         %58 = OpFunctionParameter %_ptr_Function_v2float
         %59 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
         %62 = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_10
               OpStore %62 %float_10
         %63 = OpFunctionCall %void %bar_vf %62
         %64 = OpLoad %float %62
               OpStore %x %64
         %66 = OpFOrdEqual %bool %64 %float_200
               OpSelectionMerge %71 None
               OpBranchConditional %66 %69 %70
         %69 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %74 = OpLoad %v4float %72
               OpStore %67 %74
               OpBranch %71
         %70 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %76 = OpLoad %v4float %75
               OpStore %67 %76
               OpBranch %71
         %71 = OpLabel
         %77 = OpLoad %v4float %67
               OpReturnValue %77
               OpFunctionEnd
