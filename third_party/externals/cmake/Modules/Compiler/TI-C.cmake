set(CMAKE_LIBRARY_PATH_FLAG "--search_path=")
set(CMAKE_LINK_LIBRARY_FLAG "--library=")
set(CMAKE_INCLUDE_FLAG_C "--include_path=")

set(CMAKE_C_CREATE_ASSEMBLY_SOURCE "<CMAKE_C_COMPILER> --compile_only --skip_assembler --c_file=<SOURCE> <DEFINES> <FLAGS> --output_file=<ASSEMBLY_SOURCE>")
set(CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> --preproc_only --c_file=<SOURCE> <DEFINES> <FLAGS> --output_file=<PREPROCESSED_SOURCE>")

set(CMAKE_C_COMPILE_OBJECT  "<CMAKE_C_COMPILER> --compile_only --c_file=<SOURCE> <DEFINES> <FLAGS> --output_file=<OBJECT>")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -r <TARGET> <OBJECTS>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> --run_linker --output_file=<TARGET> --map_file=<TARGET>.map <CMAKE_C_LINK_FLAGS> <LINK_LIBRARIES> <LINK_FLAGS> <OBJECTS>")
