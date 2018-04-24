FROM ubuntu:14.04

ARG SNOWSQL_VERSION=1.1.53
ARG PROXY_IP=172.20.128.10
ARG PROXY_PORT=3128

RUN apt-get update -q -y
RUN apt-get upgrade -q -y
RUN apt-get install -q -y iptables curl

ADD snowsql-${SNOWSQL_VERSION}-linux_x86_64.bash /root

RUN SNOWSQL_DEST=/root/bin SNOWSQL_LOGIN_SHELL=/root/.bashrc bash /root/snowsql-${SNOWSQL_VERSION}-linux_x86_64.bash

ENV LC_ALL C.UTF-8
ENV LANG C.UTF-8

RUN /root/bin/snowsql -h

COPY iptables.txt /root
RUN echo "source ~/iptables.txt" >> /root/.bashrc

ENV HTTP_PROXY http://${PROXY_IP}:${PROXY_PORT}
ENV HTTPS_PROXY http://${PROXY_IP}:${PROXY_PORT}

COPY run_snowsql_proxy_test.sh /root
