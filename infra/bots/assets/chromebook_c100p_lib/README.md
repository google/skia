This asset is the necessary includes and libs to compile/link the gpu code
for the ASUS Flip c100p chromebook. The mali gpu supports EGL and GLES.

Zip up the /usr/lib folder on the Asus C100p Chromebook. Extract it somewhere
on the dev machine and use that folder as the input to create_and_upload:

    ./infra/bots/assets/chromebook_c100p_lib/create_and_upload.py --lib_path [dir]

This script installs the following GL packages and then bundles them with the
unzipped libs:

    libgles2-mesa-dev libegl1-mesa-dev

