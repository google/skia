import os

index = 0
for l in os.listdir('/repos/lottie-samples/'):
  index = index+1
  print '%d %s' % (index, l)
  os.system('node skottie-wasm-perf.js --input "/repos/lottie-samples/%s" --canvaskit_js node_modules/canvaskit-wasm/bin/canvaskit.js --canvaskit_wasm node_modules/canvaskit-wasm/bin/canvaskit.wasm && python parse.py perf.json --output /tmp/output2.json' % l)
  print '----------------------------------'
