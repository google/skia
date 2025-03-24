This asset is the necessary includes and libs to compile/link the gpu code
for ARM64 Chromebooks with EGL and GLES.

Zip up the lib folder(s) (eg. /usr/lib64 and /usr/local/lib64) on any Arm64 Chromebook (e.g. Cherry).
Extract it somewhere on the dev machine and use that folder as the input to 
create_and_upload:

    ./infra/bots/assets/chromebook_arm64_gles/create_and_upload.py --lib_path [dir]

This script installs the following GL packages and then bundles them with the
unzipped libs:

    libgles2-mesa-dev libegl1-mesa-dev

