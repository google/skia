#!/bin/bash

set -x -e

wget -O knowledge.zip https://gold-stage.skia.org/json/knowledge
cp knowledge.zip ../CTS18/app/src/main/assets/knowledge.zip
