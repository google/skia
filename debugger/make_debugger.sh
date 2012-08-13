# Used to recompile required skia libraries with static initializers turned
# off. This fixes a bug in which the linux compiler was incorrectly stripping
# required global static methods in an optimization effort.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CWD=$SCRIPT_DIR/../

DEFINES="skia_static_initializers=0" 
export GYP_DEFINES="$DEFINES"

make clean -C $CWD
make -C $CWD debugger -j 
