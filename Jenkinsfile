properties([
  disableConcurrentBuilds(abortPrevious: true)
])

timestamps {
  node('regular-memory-node') {
    stage('checkout') {
      checkout scm
    }

    stage('Test Authentication') {
      try {
        withCredentials([
          string(credentialsId: 'a791118f-a1ea-46cd-b876-56da1b9bc71c', variable: 'NEXUS_PASSWORD'),
          string(credentialsId: 'sfctest0-parameters-secret', variable: 'PARAMETERS_SECRET')
        ]) {
          sh '''\
          |#!/bin/bash -e
          |$WORKSPACE/ci/test_authentication_docker.sh
          '''.stripMargin()
        }
      } finally {
        junit testResults: '**/AuthenticationTests/junit-results.xml', allowEmptyResults: true
      }
    }
  }
}
