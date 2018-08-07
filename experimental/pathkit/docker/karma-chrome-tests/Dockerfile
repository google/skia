FROM node:8.11

RUN apt-get update && apt-get upgrade -y

RUN wget https://github.com/Yelp/dumb-init/releases/download/v1.2.2/dumb-init_1.2.2_amd64.deb
RUN dpkg -i dumb-init_*.deb

ENTRYPOINT ["/usr/bin/dumb-init", "--"]

RUN wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
RUN sh -c 'echo "deb [arch=amd64] http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google.list'
RUN apt-get update && apt-get install -y google-chrome-stable

RUN npm install --global jasmine-core@3.1.0 karma@2.0.5 \
        karma-chrome-launcher@2.2.0 karma-jasmine@1.1.2 requirejs@2.3.5

#Add user so we don't need --no-sandbox.
RUN addgroup --system testrunner && adduser --system testrunner --ingroup testrunner \
    && mkdir -p /home/testrunner/Downloads \
    && chown -R testrunner:testrunner /home/testrunner

# Run everything after as non-privileged user.
USER testrunner

WORKDIR /home/testrunner