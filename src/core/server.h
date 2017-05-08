#pragma once

#include "mtk/src/server.h"
#include "config.h"

namespace mtk {
class Server : public IServer {
private:
	Config config_;
	IHandler *handler_;
public:
	Server(const Config &c, IHandler *h) : IServer(), 
		config_(std::move(c)), handler_(h) {}
	bool CreateCred(CredOptions &options);
	void Run() override;
};
}
