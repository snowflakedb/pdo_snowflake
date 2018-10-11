FROM ubuntu:16.04

ARG SNOWSQL_VERSION=1.1.53
ARG PROXY_IP=172.20.128.10
ARG PROXY_PORT=3128

RUN apt-get update -q -y
RUN apt-get upgrade -q -y
RUN apt-get install -q -y iptables  
RUN apt-get install -q -y git vim cmake pkg-config curl gcc g++ zlib1g-dev jq lcov
RUN apt-get install -q -y language-pack-en-base software-properties-common

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8
	
COPY iptables.txt /root
RUN echo "source ~/iptables.txt" >> /root/.bashrc

ENV http_proxy http://${PROXY_IP}:${PROXY_PORT}
ENV https_proxy http://${PROXY_IP}:${PROXY_PORT}
ENV HTTP_PROXY http://${PROXY_IP}:${PROXY_PORT}
ENV HTTPS_PROXY http://${PROXY_IP}:${PROXY_PORT}

COPY build_run_libsnowflakeclient_proxy_test.sh /root
