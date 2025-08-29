FROM emscripten/emsdk:4.0.7

RUN apt update && apt install -y \
    libfreetype6-dev \
    python3 \
    python-is-python3 \
    wget \
    unzip

RUN wget -O /tmp/ninja.zip https://chrome-infra-packages.appspot.com/dl/infra/3pp/tools/ninja/linux-amd64/+/version:3@1.13.1.chromium.4 && \
    unzip /tmp/ninja.zip -d /tmp/ninja && \
    cp /tmp/ninja/ninja /usr/local/bin/ninja && \
    chmod +x /usr/local/bin/ninja

RUN mkdir -p /OUT /SRC
