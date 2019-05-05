#!/bin/bash

function build_and_zip {
    bazel build :CommunicationModuleZip --platforms @AvrToolchain//platform:AVR --remote_http_cache=http://build.es.uni-due.de:9090
    if [ ! -z "$OUTPUT_DIR" ]; then
        mkdir -p "$OUTPUT_DIR"
        cp -f bazel-genfiles/CommunicationModule.zip "$OUTPUT_DIR"/communicationmodule.zip
    fi
}

build_and_zip atmega32u4
