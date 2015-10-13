include(RunCMake)

run_cmake(Concat)
run_cmake(ConcatNoArgs)

run_cmake(Uuid)
run_cmake(UuidMissingNamespace)
run_cmake(UuidMissingNamespaceValue)
run_cmake(UuidBadNamespace)
run_cmake(UuidMissingNameValue)
run_cmake(UuidMissingTypeValue)
run_cmake(UuidBadType)

run_cmake(RegexClear)

run_cmake(UTF-16BE)
run_cmake(UTF-16LE)
run_cmake(UTF-32BE)
run_cmake(UTF-32LE)
