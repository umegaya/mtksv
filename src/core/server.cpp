#include "server.h"

namespace mtk {
static std::string s_localhost = "localhost:50051";
bool Server::CreateCred(CredOptions &options) {
    std::string cert, ca, key;
    if (!config_.FileValue("cert", cert) || 
        !config_.FileValue("ca", ca) ||
        !config_.FileValue("key", ca)) {
        return false;
    }
    options.pem_root_certs = ca;
    options.pem_key_cert_pairs = {
        { .private_key = key, .cert_chain = cert },
    };
    return true;
}
void Server::Run() {
    CredOptions opts;
    bool has_cred = CreateCred(opts);
    Kick(config_.Value<std::string>("host", s_localhost), 
        config_.Value<uint32_t>("worker", 1), 
        handler_, 
        has_cred ? &opts : nullptr);
}
}
