FROM alpine:3.13.2
RUN apk add ninja cmake
COPY ./builder/build/eradiate-kernel-dist.tar /build/mitsuba/
WORKDIR /build/mitsuba
RUN tar -xf eradiate-kernel-dist.tar && ninja install && cd / && rm /build
