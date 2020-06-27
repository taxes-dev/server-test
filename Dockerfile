FROM ubuntu:20.04
ARG buildcfg

COPY bin/${buildcfg}/ServerTest /opt/server/ServerTest

EXPOSE 8080/tcp

ENTRYPOINT ["/opt/server/ServerTest","-","8080"]