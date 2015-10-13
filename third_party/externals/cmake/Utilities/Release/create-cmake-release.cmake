if(NOT DEFINED CMAKE_CREATE_VERSION)
  set(CMAKE_CREATE_VERSION "release")
  message("Using default value of 'release' for CMAKE_CREATE_VERSION")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/logs)

set(RELEASE_SCRIPTS_BATCH_1
  dash2win64_release.cmake    # Windows
  dashmacmini2_release.cmake  # Mac Darwin universal ppc;i386
  dashmacmini5_release.cmake  # Mac Darwin64 universal x86_64;i386
  magrathea_release.cmake     # Linux
  linux64_release.cmake       # Linux x86_64
  ibm_aix_release.cmake       # AIX
)

set(RELEASE_SCRIPTS_BATCH_2
  dash2win64_cygwin.cmake     # Cygwin
)

function(write_batch_shell_script filename)
  set(scripts ${ARGN})
  set(i 0)
  file(WRITE ${filename} "#!/bin/bash")
  foreach(f ${scripts})
    math(EXPR x "420*(${i}/4)")
    math(EXPR y "160*(${i}%4)")
    file(APPEND ${filename}
    "
\"${CMAKE_COMMAND}\" -DCMAKE_CREATE_VERSION=${CMAKE_CREATE_VERSION} -DCMAKE_DOC_TARBALL=\"${CMAKE_DOC_TARBALL}\" -P \"${CMAKE_ROOT}/Utilities/Release/${f}\" < /dev/null >& \"${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log\" &
xterm -geometry 64x6+${x}+${y} -sb -sl 2000 -T ${f}-${CMAKE_CREATE_VERSION}.log -e tail -f \"${CMAKE_CURRENT_SOURCE_DIR}/logs/${f}-${CMAKE_CREATE_VERSION}.log\" &
")
    math(EXPR i "${i}+1")
  endforeach()
  execute_process(COMMAND chmod a+x ${filename})
endfunction()

function(write_docs_shell_script filename)
  find_program(SPHINX_EXECUTABLE
    NAMES sphinx-build sphinx-build.py
    DOC "Sphinx Documentation Builder (sphinx-doc.org)"
    )
  if(NOT SPHINX_EXECUTABLE)
    message(FATAL_ERROR "SPHINX_EXECUTABLE (sphinx-build) is not found!")
  endif()

  set(name cmake-${CMAKE_CREATE_VERSION}-docs)
  file(WRITE "${filename}" "#!/usr/bin/env bash

name=${name} &&
inst=\"\$PWD/\$name\"
(GIT_WORK_TREE=x git archive --prefix=\${name}-src/ ${CMAKE_CREATE_VERSION}) | tar x &&
rm -rf \${name}-build &&
mkdir \${name}-build &&
cd \${name}-build &&
\"${CMAKE_COMMAND}\" ../\${name}-src/Utilities/Sphinx \\
  -DCMAKE_INSTALL_PREFIX=\"\$inst/\" \\
  -DSPHINX_EXECUTABLE=\"${SPHINX_EXECUTABLE}\" \\
  -DSPHINX_HTML=ON -DSPHINX_MAN=ON &&
make install &&
cd .. &&
tar czf \${name}.tar.gz \${name} ||
echo 'Failed to create \${name}.tar.gz'
")
  execute_process(COMMAND chmod a+x ${filename})
  set(CMAKE_DOC_TARBALL "${name}.tar.gz" PARENT_SCOPE)
endfunction()

write_docs_shell_script("create-${CMAKE_CREATE_VERSION}-docs.sh")
write_batch_shell_script("create-${CMAKE_CREATE_VERSION}-batch1.sh" ${RELEASE_SCRIPTS_BATCH_1})
unset(CMAKE_DOC_TARBALL) # No pre-built docs in second batch.
write_batch_shell_script("create-${CMAKE_CREATE_VERSION}-batch2.sh" ${RELEASE_SCRIPTS_BATCH_2})

message("Run one at a time:
 ./create-${CMAKE_CREATE_VERSION}-docs.sh   &&
 ./create-${CMAKE_CREATE_VERSION}-batch1.sh &&
 ./create-${CMAKE_CREATE_VERSION}-batch2.sh &&
 echo done
")
