#!/bin/bash -e
#
# Run a docker test
#
set -o pipefail

docker run \
    -e TRAVIS_JOB_ID=$TRAVIS_JOB_ID \
    -v $(pwd):/cfg \
    -v $(pwd):/base \
    -it \
    $DOCKER_NAME /base/docker/scripts/build_run_ubuntu.sh
