#pragma once

#include "mtk/src/conn.h"

namespace mtk {
class MonoHandler : public IHandler {
public:
	MonoHandler() {}
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
