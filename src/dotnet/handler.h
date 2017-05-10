#pragma once

#include "mtk/src/conn.h"

//#include <glib/glib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace mtk {
class MonoHandler : public IHandler {
	MonoDomain *domain_;
	MonoAssembly *assembly_;
public:
	MonoHandler() {}
	bool Init(const std::string &filename) {
		domain_ = mono_jit_init("mtk");
		if (domain_ == nullptr) {
			return false;
		}
		assembly_ = mono_domain_assembly_open(domain_, filename.c_str());
		if (assembly_ == nullptr) {
			return false;
		}
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
