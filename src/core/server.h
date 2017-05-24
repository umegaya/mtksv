#pragma once

#include "mtk/src/server.h"

namespace mtk {
class Server : public IServer {
public:
	typedef mtk_svconf_t Config;
	typedef mtk_addr_t  Address;
protected:
	Address address_;
	Config config_;
	IHandler *handler_;
public:
	Server() : IServer(), handler_(nullptr) { memset(&address_, 0, sizeof(address_)); }
	~Server();
	void Run() override;

	Server &SetAddress(const Address &a);
	inline Server &SetConfig(const Config &c) { config_ = c; return *this; }
	inline Server &SetHandler(IHandler *h) { handler_ = h; return *this; }

	static bool CreateCred(const Address &c, CredOptions &options);
	static bool LoadFile(const std::string &path, std::string &content);
};
}
