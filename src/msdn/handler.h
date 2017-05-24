#pragma once

#include "mtk/src/conn.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace mtk {
class DotNetHandler : public IHandler {
	MonoDomain *domain_;
	MonoAssembly *assembly_;
public:
	DotNetHandler() {}
	bool Init(const std::string &filename) {
		LOG(info, "init success");
		return true;
	}
	grpc::Status Handle(Conn *c, Request &req) {
		return grpc::Status::OK;
	}
	void Close(Conn *c) {
	}
	mtk_cid_t Login(Conn *c, Request &req, MemSlice &s) {
		return 0;
	}
	Conn *NewConn(Worker *worker, IHandler *handler) {
		return new Conn(worker, handler);
	}
};
}
