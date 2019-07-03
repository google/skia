# This image can be used to build an Android app using the NDK, since $ANDROID_HOME is set.
# The only "Skia-specific" thing is the depot_tools, everything else is pretty generic.
#
# The base Docker image (butomo1989/docker-android-x86-8.1:1.4-p1) has an android emulator
# that can be run by doing the following
# docker run --privileged -d --name android_em -e DEVICE="Samsung Galaxy S6" butomo1989/docker-android-x86-8.1:1.4-p1
# Then, the running container can be attached to by:
# docker exec -it android_em /bin/bash
# Of course, this Docker image can also do that, it's just a bit heavier for "emulation only"
# tasks.

FROM butomo1989/docker-android-x86-8.1:1.4-p1

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
        clang-6.0 \
        git \
        python

RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools

WORKDIR /root

RUN wget -O /root/android-ndk-r20-linux-x86_64.zip https://dl.google.com/android/repository/android-ndk-r20-linux-x86_64.zip && \
    unzip /root/android-ndk-r20-linux-x86_64.zip && \
    rm -f /root/android-ndk-r20-linux-x86_64.zip

# "yes" agrees to the license. (You might think it's waiting for input, but it's not.)
RUN yes | sdkmanager ndk-bundle "lldb;3.1" "cmake;3.6.4111459"

RUN update-alternatives --install /usr/bin/cc  cc  /usr/lib/llvm-6.0/bin/clang   20 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/lib/llvm-6.0/bin/clang++ 20

ENV CC="/usr/lib/llvm-6.0/bin/clang" \
    CXX="/usr/lib/llvm-6.0/bin/clang++" \
    PATH=/usr/lib/llvm-6.0/bin:$PATH

ENV PATH=$PATH:/opt/depot_tools

ENV ANDROID_NDK=/root/android-ndk-r20
