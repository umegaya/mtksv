FROM mtktools/builder

ADD . mtksv
RUN cd mtksv && git submodule update --init --recursive && make mono
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates-mono
RUN cert-sync /etc/ssl/certs/ca-certificates.crt
RUN curl -o /usr/local/lib/mono/nuget/NuGet.exe -L https://github.com/NuGet/Home/releases/download/3.3/NuGet.exe
RUN mkdir -p /tmp/mtk-tools && cp /mtksv/tools/dotnet/* /tmp/mtk-tools/ && chmod 755 /tmp/mtk-tools/* && \
	mv /tmp/mtk-tools/* /usr/local/bin/ && rm -r /tmp/mtk-tools
RUN nuget config -set repositoryPath=/usr/local/lib/mono/nuget
RUN nuget install Google.Protobuf
