#include "handler.h"
#include "mtk/src/mtk.h"
extern "C" {
	void mono_object_describe_fields (MonoObject *obj);
}

void *mtkdn_server(mtk_addr_t *a, mtk_svconf_t *c) {
	return mtk::MonoHandler::NewServer(a, c);
}

namespace mtk {
#define MTKLIB_NS "Mtk.Core"
#define EXPORT_MTK_LIB(method) { \
	mono_add_internal_call(MTKLIB_NS"::mtkdn_"#method, (void *)&mtk_##method); \
}
void MonoHandler::AddInternalCalls() {
	//utils
	EXPORT_MTK_LIB(queue_pop);			//TODO: to internal call
    EXPORT_MTK_LIB(queue_elem_free);
    EXPORT_MTK_LIB(time);				//TODO: to internal call
    EXPORT_MTK_LIB(second);				//TODO: to internal call
    EXPORT_MTK_LIB(slice_put);			//TODO: to internal call
    EXPORT_MTK_LIB(lib_ref);
    EXPORT_MTK_LIB(lib_unref);

    //logging
    EXPORT_MTK_LIB(log);				//TODO: to internal call
    EXPORT_MTK_LIB(log_config);

	//listener
    EXPORT_MTK_LIB(listen);
    EXPORT_MTK_LIB(server_queue);
    EXPORT_MTK_LIB(server_join);

	//server conn operation
    EXPORT_MTK_LIB(svconn_cid);			//TODO: to internal call
    EXPORT_MTK_LIB(svconn_msgid);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_send);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_notify);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_error);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_task);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_close);
    EXPORT_MTK_LIB(svconn_putctx);
    EXPORT_MTK_LIB(svconn_getctx);			//TODO: to internal call
    EXPORT_MTK_LIB(svconn_finish_login);	//TODO: to internal call
    EXPORT_MTK_LIB(svconn_defer_login);		//TODO: to internal call
    EXPORT_MTK_LIB(svconn_find_deferred);	//TODO: to internal call

	//other server conn, using cid 
    EXPORT_MTK_LIB(cid_send);				//TODO: to internal call
    EXPORT_MTK_LIB(cid_notify);				//TODO: to internal call
    EXPORT_MTK_LIB(cid_error);				//TODO: to internal call
    EXPORT_MTK_LIB(cid_task);				//TODO: to internal call
    EXPORT_MTK_LIB(cid_close);
    EXPORT_MTK_LIB(cid_getctx);				//TODO: to internal call

	//client conn
    EXPORT_MTK_LIB(connect);
    EXPORT_MTK_LIB(conn_cid);
    EXPORT_MTK_LIB(conn_poll);				//TODO: to internal call
    EXPORT_MTK_LIB(conn_close);
    EXPORT_MTK_LIB(conn_reset); //this just restart connection, never destroy. 
    EXPORT_MTK_LIB(conn_send);				//TODO: to internal call
    EXPORT_MTK_LIB(conn_timeout);
    EXPORT_MTK_LIB(conn_reconnect_wait); 
    EXPORT_MTK_LIB(conn_watch);
    EXPORT_MTK_LIB(conn_connected);			//TODO: to internal call
}
mtk::Server *MonoHandler::NewServer(Server::Address *a, Server::Config *c) {
	auto *sv = new mtk::Server();
	sv->SetAddress(*a).SetConfig(*c);
	return sv;
}
MonoMethod *MonoHandler::FindMethod(MonoClass *klass, const std::string &name) {
	MonoMethod *m;
	void *iter = NULL;
	//assume entrypoint does not contain parameter overloaded methods like Foo(int, int) and Foo(string)
	while ((m = mono_class_get_methods (klass, &iter))) {
		//mono_signature_get_desc (mono_method_signature (method), desc->include_namespace)
		//signature is like: intptr,ulong,byte*,uint,byte[]&
		if (name == mono_method_get_name(m)) {
			return m;
		}
	}
	return nullptr;
}
MonoArray *MonoHandler::FromArgs(int argc, char *argv[]) {
	auto *a = mono_array_new(domain_, mono_get_string_class(), argc);
	for (int i = 0; i < argc; i++) {
		auto *str = mono_string_new(domain_, argv[i]);
		mono_array_set(a, MonoString*, i, str);
	}
	return a;
}
void MonoHandler::DumpException(MonoObject *exc) {
	mono_object_describe_fields(exc);
}
thread_local MonoThread *MonoHandler::thread_;
mtk::Server *MonoHandler::Init(int argc, char *argv[]) {
	mono_mkbundle_init();
	AddInternalCalls();
	domain_ = mono_jit_init("mtkdn");
	if (domain_ == nullptr) {
		LOG(error, "fail to init domain");
		return nullptr;
	}
	thread_ = mono_thread_attach(domain_);
	if (thread_ == nullptr) {
		LOG(error, "fail to attach thread");
		return nullptr;
	}
	auto assembly = mono_domain_assembly_open(domain_, "Mtk.dll");
	if (assembly == nullptr) {
		LOG(error, "fail to load mtk assembly");
		return nullptr;
	}
	auto image = mono_assembly_get_image(assembly);
	if (image == nullptr) {
		LOG(error, "fail to get mtk image");
		return nullptr;
	}
	auto entrypoint = mono_class_from_name(image, "Mtk", "EntryPointBase"); 
	if (entrypoint == nullptr) {
		LOG(error, "fail to get entrypointbase");
		return nullptr;
	}
	if ((close_ = FindMethod(entrypoint, "Close")) == nullptr) {
		LOG(error, "fail ot get method close_");
		return nullptr;
	}
	if ((login_ = FindMethod(entrypoint, "Login")) == nullptr) {
		LOG(error, "fail ot get method login");
		return nullptr;
	}
	if ((handle_ = FindMethod(entrypoint, "Handle")) == nullptr) {
		LOG(error, "fail ot get method handle_");
		return nullptr;
	}

	assembly = mono_domain_assembly_open(domain_, "Server.dll");
	if (assembly == nullptr) {
		LOG(error, "fail to load server assembly");
		return nullptr;
	}
	image = mono_assembly_get_image(assembly);
	if (image == nullptr) {
		LOG(error, "fail to get server image");
		return nullptr;
	}
	entrypoint = mono_class_from_name(image, "Mtk", "EntryPoint"); 
	if (entrypoint == nullptr) {
		LOG(error, "fail to get entrypoint");
		return nullptr;
	}

	//create server instance
	MonoMethod *bootstrap;
	if ((bootstrap = FindMethod(entrypoint, "Bootstrap")) == nullptr) {
		LOG(error, "fail ot get method bootstrap");
		return nullptr;
	}
	MonoObject *svptr, *err = nullptr;
	void *args[1] = { (void *)FromArgs(argc, argv) };
	if ((svptr = mono_runtime_invoke(bootstrap, nullptr, args, &err)) == nullptr || err != nullptr) {
		LOG(error, "fail ot call method bootstrap {} {}", (void *)svptr, (void *)err);
		DumpException(err);
		return nullptr;
	}
	mtk::Server *sv = (mtk::Server *)*(intptr_t *)mono_object_unbox(svptr);
	if (sv == nullptr) {
		LOG(error, "bootstrap fail to create server");
		return nullptr;
	}
	//mono code does not seems to destroy MonoArray. its handled by gc?
	LOG(info, "ev:mono handler init success,sv:{}", (void *)sv);
	return &(sv->SetHandler(this));
}
void MonoHandler::TlsInit(Worker *w) {
	thread_ = mono_thread_attach(domain_);
	LOG(info, "ev:start worker thread,monoth:{},worker:{}", (void *)thread_, (void *)w);
}
void MonoHandler::TlsFin(Worker *w) {
	mono_thread_detach(thread_);
	LOG(info, "ev:end worker thread,monoth:{},worker:{}", (void *)thread_, (void *)w);
}
}
