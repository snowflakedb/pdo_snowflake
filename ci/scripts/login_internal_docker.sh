#!/bin/bash -e
#
# Login to internal Docker registry (Nexus)
# Matches .NET pattern: uses USERNAME env var or defaults to 'jenkins'
#

INTERNAL_REPO=${INTERNAL_REPO:-nexus.int.snowflakecomputing.com:8086}

echo "Logging in to internal Docker registry: ${INTERNAL_REPO}"

# Use USERNAME env var (set by Jenkins) or default to 'jenkins'
NEXUS_USER=${USERNAME:-jenkins}

if [ -z "$NEXUS_PASSWORD" ]; then
    echo "ERROR: NEXUS_PASSWORD environment variable not set"
    exit 1
fi

echo "$NEXUS_PASSWORD" | docker login -u "$NEXUS_USER" --password-stdin ${INTERNAL_REPO}

echo "Docker login successful"
