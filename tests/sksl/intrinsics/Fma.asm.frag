               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %one "one"
               OpName %two "two"
               OpName %three "three"
               OpName %four "four"
               OpName %five "five"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %four RelaxedPrecision
               OpDecorate %five RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_5 = OpConstant %float 5
   %float_17 = OpConstant %float 17
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
        %one = OpVariable %_ptr_Function_float Function
        %two = OpVariable %_ptr_Function_float Function
      %three = OpVariable %_ptr_Function_float Function
       %four = OpVariable %_ptr_Function_float Function
       %five = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %32 = OpAccessChain %_ptr_Uniform_float %28 %int_0
         %34 = OpLoad %float %32
               OpStore %one %34
         %36 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %38 = OpAccessChain %_ptr_Uniform_float %36 %int_1
         %39 = OpLoad %float %38
               OpStore %two %39
         %41 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %42 = OpAccessChain %_ptr_Uniform_float %41 %int_2
         %43 = OpLoad %float %42
               OpStore %three %43
         %45 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %47 = OpAccessChain %_ptr_Uniform_float %45 %int_3
         %48 = OpLoad %float %47
               OpStore %four %48
         %50 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %52 = OpAccessChain %_ptr_Uniform_float %50 %int_4
         %53 = OpLoad %float %52
               OpStore %five %53
         %56 = OpExtInst %float %1 Fma %34 %39 %43
         %58 = OpFOrdEqual %bool %56 %float_5
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %61 = OpExtInst %float %1 Fma %43 %48 %53
         %63 = OpFOrdEqual %bool %61 %float_17
               OpBranch %60
         %60 = OpLabel
         %64 = OpPhi %bool %false %25 %63 %59
               OpSelectionMerge %69 None
               OpBranchConditional %64 %67 %68
         %67 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %72 = OpLoad %v4float %70
               OpStore %65 %72
               OpBranch %69
         %68 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %74 = OpLoad %v4float %73
               OpStore %65 %74
               OpBranch %69
         %69 = OpLabel
         %75 = OpLoad %v4float %65
               OpReturnValue %75
               OpFunctionEnd
