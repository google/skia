# Docker container with Google Chrome and puppeteer.
#
# Tests will be run as non-root (user skia, in fact), so /OUT should have permissions
# 777 so as to be able to create output there.

FROM node:8.11

RUN apt-get update && apt-get upgrade -y

RUN wget https://github.com/Yelp/dumb-init/releases/download/v1.2.2/dumb-init_1.2.2_amd64.deb
RUN dpkg -i dumb-init_*.deb

# https://github.com/GoogleChrome/puppeteer/blob/master/docs/troubleshooting.md#running-puppeteer-in-docker
# recommends using dumb-init to "prevent zombie chrome processes"
ENTRYPOINT ["/usr/bin/dumb-init", "--"]

RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list'
RUN apt-get update && apt-get install -y google-chrome-stable

ENV PUPPETEER_SKIP_CHROMIUM_DOWNLOAD true

RUN npm install --global \
    command-line-args@5.0.2 \
    command-line-usage@5.0.3 \
    express@4.16.3 \
    node-fetch@2.2.0 \
    puppeteer@1.6.2

# Allows require('puppeteer') to work from anywhere.
# https://stackoverflow.com/a/15646750
ENV NODE_PATH=/usr/local/lib/node_modules

#Add user so we don't have to run as root (prevents us from over-writing files in /SRC)
RUN groupadd -g 2000 skia \
    && useradd -u 2000 -g 2000 skia \
    && mkdir -p /home/skia \
    && chown -R skia:skia /home/skia

# These directories can be used for mounting a source checkout and having a place to put outputs.
RUN mkdir -m 0777 /SRC /OUT

# Run everything after as non-privileged user.
USER skia

WORKDIR /home/skia