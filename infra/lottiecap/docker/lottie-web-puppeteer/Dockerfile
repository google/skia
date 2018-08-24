# Can run lottiecap.js in docker with the following command
# docker run -it -v $SKIA_ROOT:/SRC -v ~/lottie-samples:/LOTTIE_FILES -w /SRC/tools/lottiecap lottie-web-puppeteer node /SRC/tools/lottiecap/lottiecap.js --input /LOTTIE_FILES/body_movin.json --lottie_player /usr/local/lib/node_modules/lottie-web/build/player/lottie.min.js --in_docker
#

FROM node:8.11

RUN apt-get update && apt-get upgrade -y

RUN wget https://github.com/Yelp/dumb-init/releases/download/v1.2.2/dumb-init_1.2.2_amd64.deb
RUN dpkg -i dumb-init_*.deb

ENTRYPOINT ["/usr/bin/dumb-init", "--"]

RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list'
RUN apt-get update && apt-get install -y google-chrome-stable

ENV PUPPETEER_SKIP_CHROMIUM_DOWNLOAD true

RUN npm install --global \
    command-line-args@5.0.2 \
    command-line-usage@5.0.3 \
    express@4.16.3 \
    lottie-web@5.2.1 \
    puppeteer@1.6.2

# Allows require('puppeteer') to work from anywhere.
# https://stackoverflow.com/a/15646750
ENV NODE_PATH=/usr/local/lib/node_modules

