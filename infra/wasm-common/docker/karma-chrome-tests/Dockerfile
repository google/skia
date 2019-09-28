# Docker container with Chrome, and karma/jasmine, to be used to run JS tests.
# Inspired by https://github.com/eirslett/chrome-karma-docker
#
# Tests will be run as non-root (user skia, in fact), so /OUT should have permissions
# 777 so as to be able to create output there.

FROM node:8.11

RUN apt-get update && apt-get upgrade -y

RUN wget https://github.com/Yelp/dumb-init/releases/download/v1.2.2/dumb-init_1.2.2_amd64.deb
RUN dpkg -i dumb-init_*.deb

ENTRYPOINT ["/usr/bin/dumb-init", "--"]

RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list'
RUN apt-get update && apt-get install -y google-chrome-stable

RUN npm install --global jasmine-core@3.1.0 karma@2.0.5 \
        karma-chrome-launcher@2.2.0 karma-jasmine@1.1.2 requirejs@2.3.5 \
        is-docker@1.1.0

# Allows require('is-docker') or require('karma') to work from anywhere.
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