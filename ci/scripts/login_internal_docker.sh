#!/bin/bash
#
# Login to internal Docker registry (Nexus)
#

INTERNAL_REPO=${INTERNAL_REPO:-nexus.int.snowflakecomputing.com:8086}

echo "Logging in to internal Docker registry: ${INTERNAL_REPO}"

if [ -z "$NEXUS_USERNAME" ]; then
    echo "ERROR: NEXUS_USERNAME environment variable not set"
    exit 1
fi

if [ -z "$NEXUS_PASSWORD" ]; then
    echo "ERROR: NEXUS_PASSWORD environment variable not set"
    exit 1
fi

echo "$NEXUS_PASSWORD" | docker login -u "$NEXUS_USERNAME" --password-stdin ${INTERNAL_REPO}

echo "Docker login successful"
