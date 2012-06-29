#!/bin/bash

# NOTE(chudy): May have to update location of moc binary depending on system.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MOC="/usr/bin/moc"
#MOC="/usr/local/google/home/chudy/tools/Qt/Desktop/Qt/4.8.1/gcc/bin/moc"
SRC_DIR=$SCRIPT_DIR/QT

$MOC $SRC_DIR/SkDebuggerGUI.h -o $SRC_DIR/moc_SkDebuggerGUI.cpp
$MOC $SRC_DIR/SkCanvasWidget.h -o $SRC_DIR/moc_SkCanvasWidget.cpp
$MOC $SRC_DIR/SkInspectorWidget.h -o $SRC_DIR/moc_SkInspectorWidget.cpp
$MOC $SRC_DIR/SkSettingsWidget.h -o $SRC_DIR/moc_SkSettingsWidget.cpp
