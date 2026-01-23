import groovy.json.JsonOutput

timestamps {
  node('regular-memory-node') {
    stage('checkout') {
      scmInfo = checkout scm
      println("${scmInfo}")
      env.GIT_BRANCH = scmInfo.GIT_BRANCH
      env.GIT_COMMIT = scmInfo.GIT_COMMIT
    }

    stage('Test') {
      parallel(
        'Test Authentication': {
          stage('Test Authentication') {
            withCredentials([
              string(credentialsId: 'a791118f-a1ea-46cd-b876-56da1b9bc71c', variable: 'NEXUS_PASSWORD'),
              string(credentialsId: 'sfctest0-parameters-secret', variable: 'PARAMETERS_SECRET')
            ]) {
              sh '''\
              |#!/bin/bash -e
              |$WORKSPACE/ci/test_authentication_docker.sh
              '''.stripMargin()
            }
          }
        }
      )
    }
  }
}

pipeline {
  agent { label 'regular-memory-node' }
  options { timestamps() }
  environment {
    COMMIT_SHA_LONG = sh(returnStdout: true, script: "echo \$(git rev-parse HEAD)").trim()
    BASELINE_BRANCH = "${env.CHANGE_TARGET}"
  }
  stages {
    stage('Checkout') {
      steps {
        checkout scm
      }
    }
  }
}

def wgetUpdateGithub(String state, String folder, String targetUrl, String seconds) {
  def ghURL = "https://api.github.com/repos/snowflakedb/pdo_snowflake/statuses/$COMMIT_SHA_LONG"
  def data = JsonOutput.toJson([state: "${state}", context: "jenkins/${folder}", target_url: "${targetUrl}"])
  sh "wget ${ghURL} --spider -q --header='Authorization: token $GIT_PASSWORD' --post-data='${data}'"
}

