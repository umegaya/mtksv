#pragma once

#include "server.h"
#include "mtk/src/conn.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/debug-helpers.h>


extern "C" {
	void mono_mkbundle_init();
	void *mtkdn_server(mtk_addr_t*, mtk_svconf_t*);
}

namespace mtk {
class MonoHandler : public IHandler {
	MonoDomain *domain_;
	MonoMethod *login_, *handle_, *close_;
	static thread_local MonoThread *thread_;
public:
	MonoHandler() {}
	mtk::Server *Init(int argc, char *argv[]);

	//below called from multiple thread.
	void TlsInit(Worker *w);
	void TlsFin(Worker *w);
	grpc::Status Handle(Conn *c, Request &req) override {
		auto type = req.type();
		auto data = req.payload().data();
		auto dlen = req.payload().length();
		void *args[4] = {
			/* System.IntPtr c, int type, byte* data, uint len */
			(void *)&c, 
			(void *)&type, 
			(void *)data,
			(void *)&dlen,
		};
		MonoObject *exc, *ret = mono_runtime_invoke(handle_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "ev:Handle callback exception raised");
			DumpException(exc);
			return grpc::Status::CANCELLED;
		}
		return grpc::Status::OK;
	}
	void Close(Conn *c) override {
		void *args[4] = {
			/* System.IntPtr c */
			(void *)&c, 
		};
		MonoObject *exc;
		mono_runtime_invoke(close_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "ev:Close callback exception raised");
			DumpException(exc);
		}
	}
	mtk_cid_t Login(Conn *c, Request &req, MemSlice &s) override {
		SystemPayload::Connect creq;
		if (Codec::Unpack((const uint8_t *)req.payload().c_str(), req.payload().length(), creq) < 0) {
			LOG(error, "ev:Login callback invalid payload");
			return 0;
		}
		MonoObject *obj;
		auto cid = creq.id();
		auto data = creq.payload().data();
		auto dlen = creq.payload().length();
		LOG(info, "login payload len:{},p:{}", dlen, (void *)data);
		void *args[5] = {
			/* ulong cid, byte* data, uint len, out byte[] repdata */
			(void *)&c,
			(void *)&cid,
			(void *)data,
			(void *)&dlen,
			(void *)&obj,
		};
		MonoObject *exc, *ret = mono_runtime_invoke(login_, nullptr, args, &exc);
		if (exc != nullptr) {
			LOG(error, "ev:Login callback exception raised");
			DumpException(exc);
			mtk_svconn_close(c);
			return 0;
		}
		MonoArray *a = (MonoArray *)obj;
		s.Put(mono_array_addr_with_size(a, 0, 0), mono_array_length(a));
		return *(uint64_t *)mono_object_unbox(ret);
	}
	Conn *NewConn(Worker *worker, IHandler *handler) {
		return new Conn(worker, handler);
	}
	static Server *NewServer(Server::Address *, Server::Config *);
protected:
	void AddInternalCalls();
	static MonoMethod *FindMethod(MonoClass *klass, const std::string &name);
	static void DumpException(MonoObject *exc);
	static MonoObject *CallVirtual(MonoObject *self, const char *method, void *args[], int n_args);
	MonoArray *FromArgs(int argc, char *argv[]);
};
}
