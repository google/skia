### Compilation failed:

error: SPIR-V validation error: Expected Constituent type to be equal to the corresponding member type of Result Type struct
  %30 = OpCompositeConstruct %S %29

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testPushConstants "testPushConstants"
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"
               OpMemberName %testUniforms 0 "uboArray"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "a"
               OpName %s1 "s1"
               OpName %s2 "s2"
               OpDecorate %_arr_float_int_2 ArrayStride 4
               OpMemberDecorate %testPushConstants 0 Offset 0
               OpDecorate %testPushConstants Block
               OpDecorate %_arr_float_int_2_0 ArrayStride 16
               OpMemberDecorate %testUniforms 0 Offset 0
               OpDecorate %testUniforms Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %52 RelaxedPrecision
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
          %3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%testUniforms = OpTypeStruct %_arr_float_int_2_0
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
         %10 = OpVariable %_ptr_Uniform_testUniforms Uniform
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
          %S = OpTypeStruct %_arr_float_int_2_0
%_ptr_Function_S = OpTypePointer Function %S
      %int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %49 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %51 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %21
         %22 = OpLabel
         %s1 = OpVariable %_ptr_Function_S Function
         %s2 = OpVariable %_ptr_Function_S Function
         %43 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
         %29 = OpLoad %_arr_float_int_2 %27
         %30 = OpCompositeConstruct %S %29
               OpStore %s1 %30
         %32 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %34 = OpLoad %_arr_float_int_2_0 %32
         %35 = OpCompositeConstruct %S %34
               OpStore %s2 %35
         %36 = OpCompositeExtract %float %29 0
         %37 = OpCompositeExtract %float %34 0
         %38 = OpFOrdEqual %bool %36 %37
         %39 = OpCompositeExtract %float %29 1
         %40 = OpCompositeExtract %float %34 1
         %41 = OpFOrdEqual %bool %39 %40
         %42 = OpLogicalAnd %bool %41 %38
               OpSelectionMerge %47 None
               OpBranchConditional %42 %45 %46
         %45 = OpLabel
               OpStore %43 %49
               OpBranch %47
         %46 = OpLabel
               OpStore %43 %51
               OpBranch %47
         %47 = OpLabel
         %52 = OpLoad %v4float %43
               OpStore %sk_FragColor %52
               OpReturn
               OpFunctionEnd

1 error
