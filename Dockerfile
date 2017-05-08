FROM mtktools/builder

ADD . mtksv
RUN cd mtksv && git submodule update --init --recursive && make mono