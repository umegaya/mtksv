#!/bin/bash

# install mono and nuget
apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
echo "deb http://download.mono-project.com/repo/debian wheezy main" | tee /etc/apt/sources.list.d/mono-xamarin.list
apt-get update && apt-get install -y mono-devel mono-utils nuget

# setup cert to run nuget correctly
apt-get install -y --no-install-recommends ca-certificates-mono
cert-sync /etc/ssl/certs/ca-certificates.crt

# intall dependency third party assembly
mkdir -p /usr/lib/mono/nuget
/usr/bin/nuget config -set repositoryPath=/usr/lib/mono/nuget
/usr/bin/nuget install Google.Protobuf
