@Library('pipeline-utils')
import com.snowflake.DevEnvUtils
import groovy.json.JsonOutput

properties([
  disableConcurrentBuilds(abortPrevious: true)
])

timestamps {
  node('regular-memory-node') {
    stage('checkout') {
      checkout scm
    }

    stage('Authenticate Artifactory') {
      script {
        new DevEnvUtils().withSfCli {
          sh "sf artifact oci auth"
        }
      }
    }

    stage('Test Authentication') {
      try {
        withCredentials([
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
