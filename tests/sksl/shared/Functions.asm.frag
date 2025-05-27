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
               OpName %foo_ff2 "foo_ff2"
               OpName %bar_vf "bar_vf"
               OpName %y "y"
               OpName %main "main"
               OpName %x "x"
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
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
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
         %22 = OpTypeFunction %float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
         %31 = OpTypeFunction %void %_ptr_Function_float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
      %int_1 = OpConstant %int 1
         %54 = OpTypeFunction %v4float %_ptr_Function_v2float
   %float_10 = OpConstant %float 10
  %float_200 = OpConstant %float 200
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
    %foo_ff2 = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpLabel
         %25 = OpLoad %v2float %23
         %26 = OpCompositeExtract %float %25 0
         %27 = OpLoad %v2float %23
         %28 = OpCompositeExtract %float %27 1
         %29 = OpFMul %float %26 %28
               OpReturnValue %29
               OpFunctionEnd
     %bar_vf = OpFunction %void None %31
         %32 = OpFunctionParameter %_ptr_Function_float
         %33 = OpLabel
          %y = OpVariable %_ptr_Function__arr_float_int_2 Function
         %52 = OpVariable %_ptr_Function_v2float Function
         %39 = OpLoad %float %32
         %41 = OpAccessChain %_ptr_Function_float %y %int_0
               OpStore %41 %39
         %42 = OpLoad %float %32
         %44 = OpFMul %float %42 %float_2
         %46 = OpAccessChain %_ptr_Function_float %y %int_1
               OpStore %46 %44
         %47 = OpAccessChain %_ptr_Function_float %y %int_0
         %48 = OpLoad %float %47
         %49 = OpAccessChain %_ptr_Function_float %y %int_1
         %50 = OpLoad %float %49
         %51 = OpCompositeConstruct %v2float %48 %50
               OpStore %52 %51
         %53 = OpFunctionCall %float %foo_ff2 %52
               OpStore %32 %53
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %54
         %55 = OpFunctionParameter %_ptr_Function_v2float
         %56 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
         %59 = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_10
               OpStore %59 %float_10
         %60 = OpFunctionCall %void %bar_vf %59
         %61 = OpLoad %float %59
               OpStore %x %61
         %63 = OpFOrdEqual %bool %61 %float_200
               OpSelectionMerge %69 None
               OpBranchConditional %63 %67 %68
         %67 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %72 = OpLoad %v4float %70
               OpStore %65 %72
               OpBranch %69
         %68 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %74 = OpLoad %v4float %73
               OpStore %65 %74
               OpBranch %69
         %69 = OpLabel
         %75 = OpLoad %v4float %65
               OpReturnValue %75
               OpFunctionEnd
