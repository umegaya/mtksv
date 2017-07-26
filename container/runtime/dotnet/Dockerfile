FROM debian:stretch-slim

ADD mtkdn /
ADD config /etc/mono/
ADD libserver.so /usr/local/lib/

ENTRYPOINT ["/mtkdn"]
CMD ["noargs"]
