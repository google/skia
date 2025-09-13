### Compilation failed:

error: SPIR-V validation error: Expected Constituent type to be equal to the corresponding member type of Result Type struct
  %30 = OpCompositeConstruct %S %29

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testPushConstants "testPushConstants"    ; id %11
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"  ; id %15
               OpMemberName %testUniforms 0 "uboArray"
               OpName %sk_FragColor "sk_FragColor"  ; id %17
               OpName %main "main"                  ; id %6
               OpName %S "S"                        ; id %24
               OpMemberName %S 0 "a"
               OpName %s1 "s1"                      ; id %23
               OpName %s2 "s2"                      ; id %31

               ; Annotations
               OpDecorate %_arr_float_int_2 ArrayStride 4
               OpMemberDecorate %testPushConstants 0 Offset 0
               OpDecorate %testPushConstants Block
               OpDecorate %_arr_float_int_2_0 ArrayStride 16
               OpMemberDecorate %testUniforms 0 Offset 0
               OpDecorate %testUniforms Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %53 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 4
%testPushConstants = OpTypeStruct %_arr_float_int_2     ; Block
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
          %7 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2     ; ArrayStride 16
%testUniforms = OpTypeStruct %_arr_float_int_2_0    ; Block
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
         %13 = OpVariable %_ptr_Uniform_testUniforms Uniform    ; Binding 0, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
          %S = OpTypeStruct %_arr_float_int_2_0
%_ptr_Function_S = OpTypePointer Function %S
      %int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %50 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0


               ; Function main
       %main = OpFunction %void None %21

         %22 = OpLabel
         %s1 =   OpVariable %_ptr_Function_S Function
         %s2 =   OpVariable %_ptr_Function_S Function
         %44 =   OpVariable %_ptr_Function_v4float Function
         %27 =   OpAccessChain %_ptr_PushConstant__arr_float_int_2 %7 %int_0
         %29 =   OpLoad %_arr_float_int_2 %27
         %30 =   OpCompositeConstruct %S %29
                 OpStore %s1 %30
         %32 =   OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %13 %int_0
         %34 =   OpLoad %_arr_float_int_2_0 %32
         %35 =   OpCompositeConstruct %S %34
                 OpStore %s2 %35
         %36 =   OpCompositeExtract %float %29 0
         %37 =   OpCompositeExtract %float %34 0
         %38 =   OpFOrdEqual %bool %36 %37
         %40 =   OpCompositeExtract %float %29 1
         %41 =   OpCompositeExtract %float %34 1
         %42 =   OpFOrdEqual %bool %40 %41
         %43 =   OpLogicalAnd %bool %42 %38
                 OpSelectionMerge %48 None
                 OpBranchConditional %43 %46 %47

         %46 =     OpLabel
                     OpStore %44 %50
                     OpBranch %48

         %47 =     OpLabel
                     OpStore %44 %52
                     OpBranch %48

         %48 = OpLabel
         %53 =   OpLoad %v4float %44                ; RelaxedPrecision
                 OpStore %sk_FragColor %53
                 OpReturn
               OpFunctionEnd

1 error
