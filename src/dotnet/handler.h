#pragma once

#include "server.h"
#include "mtk/src/conn.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/debug-helpers.h>


extern "C" {
	void mono_mkbundle_init();
}

namespace mtk {
class MonoHandler : public IHandler {
	MonoDomain *domain_;
	MonoAssembly *assembly_;
	MonoImage *image_;
	MonoMethod *login_, *handle_, *close_;
	static thread_local MonoThread *thread_;
public:
	MonoHandler() {}
	mtk::Server *Init(int argc, char *argv[]);

	//below called from multiple thread.
	void TlsInit(Worker *w);
	void TlsFin(Worker *w);
	grpc::Status Handle(Conn *c, Request &req) override {
		void *args[4] = {
			/* System.IntPtr c, int type, byte* data, uint len */
			(void *)c, 
			(void *)(intptr_t)req.type(), 
			(void *)req.payload().data(),
			(void *)(intptr_t)req.payload().length(),
		};
		MonoObject *exc, *ret = mono_runtime_invoke(handle_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "Handle callback exception raised");
			return grpc::Status::CANCELLED;
		}
		return grpc::Status::OK;
	}
	void Close(Conn *c) override {
		void *args[4] = {
			/* System.IntPtr c */
			(void *)c, 
		};
		MonoObject *exc;
		mono_runtime_invoke(handle_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "Close callback exception raised");
		}
	}
	mtk_cid_t Login(Conn *c, Request &req, MemSlice &s) override {
		SystemPayload::Connect creq;
		if (Codec::Unpack((const uint8_t *)req.payload().c_str(), req.payload().length(), creq) < 0) {
			LOG(error, "Login callback invalid payload");
			return 0;
		}
		MonoObject *obj;
		void *args[5] = {
			/* ulong cid, byte* data, uint len, out byte[] repdata */
			(void *)c,
			(void *)creq.id(), 
			(void *)creq.payload().data(),
			(void *)(intptr_t)creq.payload().length(),
			(void *)&obj,
		};
		MonoObject *exc, *ret = mono_runtime_invoke(handle_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "Login callback exception raised");
			return 0;
		}
		MonoArray *a = (MonoArray *)obj;
		s.Put(mono_array_addr_with_size(a, 0, 0), mono_array_length(a));
		return *(uint64_t *)mono_object_unbox(ret);
	}
	Conn *NewConn(Worker *worker, IHandler *handler) {
		return new Conn(worker, handler);
	}
protected:
	void AddInternalCalls();
	static mtk::Server *NewServer(Server::Address *, Server::Config *);
	MonoMethod *FindMethod(const std::string &name);
	MonoArray *FromArgs(int argc, char *argv[]);
};
}
