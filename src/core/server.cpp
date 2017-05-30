#include "server.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

namespace mtk {
Server::~Server() {
    if (handler_ != nullptr) {
        delete handler_;
    }
    if (addrs_ != nullptr) {
        for (int i = 0; i < n_addrs_; i++) {
            const auto &a = addrs_[i];
            if (a.host != nullptr) { free((void *)a.host); }
            if (a.cert != nullptr) { free((void *)a.cert); }
            if (a.key != nullptr) { free((void *)a.key); }
            if (a.ca != nullptr) { free((void *)a.ca); }
        }
        delete []addrs_;
    }
}
Server &Server::SetAddress(const Address *a, int n_addrs) { 
    n_addrs_ = n_addrs;
    addrs_ = new Address[n_addrs];
    for (int i = 0; i < n_addrs; i++) {
        addrs_[i] = {
            .host = strdup(a[i].host),
            .cert = a[i].cert == nullptr ? nullptr : strdup(a[i].cert),
            .key = a[i].key == nullptr ? nullptr : strdup(a[i].key),
            .ca = a[i].ca == nullptr ? nullptr : strdup(a[i].ca),
        };
    }
    return *this; 
}

void Server::Run() {
    Kick(
        addrs_,
        n_addrs_, 
        config_.n_worker, 
        handler_
    );
}
}
