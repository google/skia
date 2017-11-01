This asset is the necessary includes and libs to compile/link the gpu code
for x86_64 Chromebooks with gpu supports EGL and GLES.

Zip up the /usr/lib64 folder on any x86_64 Chromebook (e.g. Pixelbook). Extract it somewhere
on the dev machine and use that folder as the input to create_and_upload:

    ./infra/bots/assets/chromebook_x86_64_gles/create_and_upload.py --lib_path [dir]

This script installs the following GL packages and then bundles them with the
unzipped libs:

    libgles2-mesa-dev libegl1-mesa-dev

