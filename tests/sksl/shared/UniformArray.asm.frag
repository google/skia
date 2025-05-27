               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testArray"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %index "index"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 80
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 96
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %53 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %_arr_float_int_5 %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
      %index = OpVariable %_ptr_Function_int Function
               OpStore %index %int_0
               OpBranch %29
         %29 = OpLabel
               OpLoopMerge %33 %32 None
               OpBranch %30
         %30 = OpLabel
         %34 = OpLoad %int %index
         %35 = OpSLessThan %bool %34 %int_5
               OpBranchConditional %35 %31 %33
         %31 = OpLabel
         %37 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_0
         %39 = OpLoad %int %index
         %40 = OpAccessChain %_ptr_Uniform_float %37 %39
         %42 = OpLoad %float %40
         %43 = OpLoad %int %index
         %45 = OpIAdd %int %43 %int_1
         %46 = OpConvertSToF %float %45
         %47 = OpFUnordNotEqual %bool %42 %46
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %53 = OpLoad %v4float %50
               OpReturnValue %53
         %49 = OpLabel
               OpBranch %32
         %32 = OpLabel
         %54 = OpLoad %int %index
         %55 = OpIAdd %int %54 %int_1
               OpStore %index %55
               OpBranch %29
         %33 = OpLabel
         %56 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %57 = OpLoad %v4float %56
               OpReturnValue %57
               OpFunctionEnd
