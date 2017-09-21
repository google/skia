Checkout and build the emscripten_sdk following the instructions here:
http://webassembly.org/getting-started/developers-guide/

Then, run
infra/bots/assets/emscripten_sdk/create_and_upload.py -s /path/to/dir/emsdk

It will take a while because the emsdk dir is > 26GB.