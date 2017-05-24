#include "server.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

namespace mtk {
static std::string s_localhost = "0.0.0.0:50051";
bool Server::LoadFile(const std::string &path, std::string &content) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == nullptr) {
        return false;
    }
    char buffer[st.st_size];
    auto sz = fread(buffer, 1, st.st_size, fp);
    content = std::string(buffer, sz);
    return true;
}
bool Server::CreateCred(const Address &c, CredOptions &options) {
    std::string cert, ca, key;
    if (!LoadFile(c.cert, cert) || 
        !LoadFile(c.ca, ca) ||
        !LoadFile(c.key, key)) {
        return false;
    }
    options.pem_root_certs = ca;
    options.pem_key_cert_pairs = {
        { .private_key = key, .cert_chain = cert },
    };
    return true;
}
Server::~Server() {
    if (handler_ != nullptr) {
        delete handler_;
    }
    if (address_.host != nullptr) {
        free((void *)address_.host);
    }
    if (address_.cert != nullptr) {
        free((void *)address_.cert);
    }
    if (address_.key != nullptr) {
        free((void *)address_.key);
    }
    if (address_.ca != nullptr) {
        free((void *)address_.ca);
    }
}
Server &Server::SetAddress(const Address &a) { 
    address_ = {
        .host = strdup(a.host),
        .cert = strdup(a.cert),
        .key = strdup(a.key),
        .ca = strdup(a.ca),
    };
    return *this; 
}

void Server::Run() {
    CredOptions opts;
    bool has_cred = CreateCred(address_, opts);
    Kick(
        address_.host == nullptr ? s_localhost : address_.host, 
        config_.n_worker, 
        handler_, 
        has_cred ? &opts : nullptr
    );
}
}
