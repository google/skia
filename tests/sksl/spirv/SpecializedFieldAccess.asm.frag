               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %StructB "StructB"
               OpMemberName %StructB 0 "val"
               OpName %StructA "StructA"
               OpMemberName %StructA 0 "structB"
               OpMemberName %StructA 1 "val"
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "testStructA"
               OpMemberName %testStorageBuffer 1 "testArr"
               OpName %testSecondStorageBuffer "testSecondStorageBuffer"
               OpMemberName %testSecondStorageBuffer 0 "testStructArr"
               OpName %foo_fff_testArr "foo_fff_testArr"
               OpName %bar_fSf_testStructArr "bar_fSf_testStructArr"
               OpName %main "main"
               OpMemberDecorate %StructB 0 Offset 0
               OpMemberDecorate %StructA 0 Offset 0
               OpMemberDecorate %StructA 0 RelaxedPrecision
               OpMemberDecorate %StructA 1 Offset 4
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpMemberDecorate %testStorageBuffer 0 RelaxedPrecision
               OpMemberDecorate %testStorageBuffer 1 Offset 8
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %5 Binding 0
               OpDecorate %5 DescriptorSet 0
               OpDecorate %_runtimearr_StructA ArrayStride 8
               OpMemberDecorate %testSecondStorageBuffer 0 Offset 0
               OpMemberDecorate %testSecondStorageBuffer 0 RelaxedPrecision
               OpDecorate %testSecondStorageBuffer BufferBlock
               OpDecorate %12 Binding 1
               OpDecorate %12 DescriptorSet 0
      %float = OpTypeFloat 32
    %StructB = OpTypeStruct %float
    %StructA = OpTypeStruct %StructB %float
%_runtimearr_float = OpTypeRuntimeArray %float
%testStorageBuffer = OpTypeStruct %StructA %_runtimearr_float
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %5 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
%_runtimearr_StructA = OpTypeRuntimeArray %StructA
%testSecondStorageBuffer = OpTypeStruct %_runtimearr_StructA
%_ptr_Uniform_testSecondStorageBuffer = OpTypePointer Uniform %testSecondStorageBuffer
         %12 = OpVariable %_ptr_Uniform_testSecondStorageBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
         %17 = OpTypeFunction %float %_ptr_Function_float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %35 = OpTypeFunction %void
%foo_fff_testArr = OpFunction %float None %17
         %18 = OpFunctionParameter %_ptr_Function_float
         %19 = OpLabel
         %22 = OpLoad %float %18
         %23 = OpConvertFToS %int %22
         %24 = OpAccessChain %_ptr_Uniform_float %5 %int_1 %23
         %26 = OpLoad %float %24
               OpReturnValue %26
               OpFunctionEnd
%bar_fSf_testStructArr = OpFunction %float None %17
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
         %30 = OpLoad %float %27
         %31 = OpConvertFToS %int %30
         %32 = OpAccessChain %_ptr_Uniform_float %12 %int_0 %31 %int_0 %int_0
         %33 = OpLoad %float %32
               OpReturnValue %33
               OpFunctionEnd
       %main = OpFunction %void None %35
         %36 = OpLabel
         %39 = OpVariable %_ptr_Function_float Function
         %43 = OpVariable %_ptr_Function_float Function
         %47 = OpVariable %_ptr_Function_float Function
         %37 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %int_1
         %38 = OpLoad %float %37
               OpStore %39 %38
         %40 = OpFunctionCall %float %foo_fff_testArr %39
         %41 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %int_0 %int_0
         %42 = OpLoad %float %41
               OpStore %43 %42
         %44 = OpFunctionCall %float %foo_fff_testArr %43
         %45 = OpAccessChain %_ptr_Uniform_float %5 %int_0 %int_0 %int_0
         %46 = OpLoad %float %45
               OpStore %47 %46
         %48 = OpFunctionCall %float %bar_fSf_testStructArr %47
               OpReturn
               OpFunctionEnd
