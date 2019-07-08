import logging
import os


for l in os.listdir('/repos/lottie-samples/'):
  os.system('node lottie-web-perf.js --port 8083 --input "/repos/lottie-samples/%s" --lottie_player ./node_modules/lottie-web/build/player/lottie.min.js && python parse.py perf.json --output /tmp/output1.json' % l)
