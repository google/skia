# The base docker image has an android emulator that can be run by doing the following
# docker run --privileged -d --name android_em -e DEVICE="Samsung Galaxy S6" gcr.io/skia-public/android-skqp:8.1_v1
# Then, the running container can be attached to by:
# docker exec -it android_em /bin/bash
#
# Additionally, this image can be used to build an Android app, since $ANDROID_HOME is set.
FROM butomo1989/docker-android-x86-8.1:1.4-p1

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
        clang-6.0 \
        git \
        python

RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools

WORKDIR /root

RUN wget -O /root/android-ndk-r18-linux-x86_64.zip https://storage.googleapis.com/skia-cdn/android-sdk/android-ndk-r18-linux-x86_64.zip && \
    unzip /root/android-ndk-r18-linux-x86_64.zip && \
    rm -f /root/android-ndk-r18-linux-x86_64.zip

RUN sdkmanager ndk-bundle "lldb;3.1" "cmake;3.6.4111459"

RUN update-alternatives --install /usr/bin/cc  cc  /usr/lib/llvm-6.0/bin/clang   20 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/lib/llvm-6.0/bin/clang++ 20

ENV CC="/usr/lib/llvm-6.0/bin/clang" \
    CXX="/usr/lib/llvm-6.0/bin/clang++" \
    PATH=/usr/lib/llvm-6.0/bin:$PATH

ENV PATH=$PATH:/opt/depot_tools

ENV ANDROID_NDK=/root/android-ndk-r18
