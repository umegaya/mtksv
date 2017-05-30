#pragma once

#include "mtk/src/server.h"

namespace mtk {
class Server : public IServer {
public:
	typedef mtk_svconf_t Config;
protected:
	Address *addrs_;
	int n_addrs_;
	Config config_;
	IHandler *handler_;
public:
	Server() : IServer(), addrs_(nullptr), handler_(nullptr) {}
	~Server();
	void Run() override;

	Server &SetAddress(const Address *addrs, int n_addr);
	inline Server &SetConfig(const Config &c) { config_ = c; return *this; }
	inline Server &SetHandler(IHandler *h) { handler_ = h; return *this; }
};
}
