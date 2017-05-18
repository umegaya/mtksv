#pragma once

#include "mtk/src/conn.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

extern "C" {
	void mono_mkbundle_init();
}

namespace mtk {
class MonoHandler : public IHandler {
	MonoDomain *domain_;
	MonoAssembly *assembly_;
public:
	MonoHandler() {}
	bool Init(const std::string &filename) {
		mono_mkbundle_init();
		domain_ = mono_jit_init("mtk");
		if (domain_ == nullptr) {
			LOG(error, "fail to init domain");
			return false;
		}
		assembly_ = mono_domain_assembly_open(domain_, "Server.dll");
		if (assembly_ == nullptr) {
			LOG(error, "fail to load assembly");
			return false;
		}
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
