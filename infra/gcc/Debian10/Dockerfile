FROM debian:10-slim

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  build-essential \
  && rm -rf /var/lib/apt/lists/* \
  && addgroup --gid 2000 skia \
  && adduser --uid 2000 --gid 2000 skia

USER skia:skia
