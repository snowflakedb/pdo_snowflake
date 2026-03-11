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

    parallel(
      'Test Authentication': {
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
      },
      'Test Revocation Validation': {
        stage('Test Revocation Validation') {
          withCredentials([
            usernamePassword(credentialsId: 'jenkins-snowflake-eng-github-app',
              usernameVariable: 'GITHUB_USER',
              passwordVariable: 'GITHUB_TOKEN')
          ]) {
            try {
              sh '''\
              |#!/bin/bash -e
              |chmod +x $WORKSPACE/ci/test_revocation.sh
              |$WORKSPACE/ci/test_revocation.sh
              '''.stripMargin()
            } finally {
              archiveArtifacts artifacts: 'revocation-results.json,revocation-report.html', allowEmptyArchive: true
              publishHTML(target: [
                allowMissing: true,
                alwaysLinkToLastBuild: true,
                keepAll: true,
                reportDir: '.',
                reportFiles: 'revocation-report.html',
                reportName: 'Revocation Validation Report'
              ])
            }
          }
        }
      }
    )
  }
}
