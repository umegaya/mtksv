#include "handler.h"
#include "mtk/src/mtk.h"

namespace mtk {
#define MTKLIB_NS "Mtk.Core"
#define EXPORT_MTK_LIB(method) { \
	mono_add_internal_call(MTKLIB_NS"::"#method, (void *)&method); \
}
void MonoHandler::AddInternalCalls() {
	//create server instance
	mono_add_internal_call(MTKLIB_NS"::mtkdn_server", (void *)&MonoHandler::NewServer);

	//utils
	EXPORT_MTK_LIB(mtk_queue_pop);
    EXPORT_MTK_LIB(mtk_queue_elem_free);
    EXPORT_MTK_LIB(mtk_time);
    EXPORT_MTK_LIB(mtk_second);
    EXPORT_MTK_LIB(mtk_slice_put);
    EXPORT_MTK_LIB(mtk_lib_ref);
    EXPORT_MTK_LIB(mtk_lib_unref);

    //logging
    EXPORT_MTK_LIB(mtk_log);
    EXPORT_MTK_LIB(mtk_log_config);

	//listener
    EXPORT_MTK_LIB(mtk_listen);
    EXPORT_MTK_LIB(mtk_server_queue);
    EXPORT_MTK_LIB(mtk_server_join);

	//server conn operation
    EXPORT_MTK_LIB(mtk_svconn_cid);
    EXPORT_MTK_LIB(mtk_svconn_msgid);
    EXPORT_MTK_LIB(mtk_svconn_send);
    EXPORT_MTK_LIB(mtk_svconn_notify);
    EXPORT_MTK_LIB(mtk_svconn_error);
    EXPORT_MTK_LIB(mtk_svconn_task);
    EXPORT_MTK_LIB(mtk_svconn_close);
    EXPORT_MTK_LIB(mtk_svconn_putctx);
    EXPORT_MTK_LIB(mtk_svconn_getctx);
    EXPORT_MTK_LIB(mtk_svconn_finish_login);
    EXPORT_MTK_LIB(mtk_svconn_defer_login);
    EXPORT_MTK_LIB(mtk_svconn_find_deferred);

	//other server conn, using cid 
    EXPORT_MTK_LIB(mtk_cid_send);
    EXPORT_MTK_LIB(mtk_cid_notify);
    EXPORT_MTK_LIB(mtk_cid_error);
    EXPORT_MTK_LIB(mtk_cid_task);
    EXPORT_MTK_LIB(mtk_cid_close);
    EXPORT_MTK_LIB(mtk_cid_getctx);

	//client conn
    EXPORT_MTK_LIB(mtk_connect);
    EXPORT_MTK_LIB(mtk_conn_cid);
    EXPORT_MTK_LIB(mtk_conn_poll);
    EXPORT_MTK_LIB(mtk_conn_close);
    EXPORT_MTK_LIB(mtk_conn_reset); //this just restart connection, never destroy. 
    EXPORT_MTK_LIB(mtk_conn_send);
    EXPORT_MTK_LIB(mtk_conn_timeout);
    EXPORT_MTK_LIB(mtk_conn_reconnect_wait); 
    EXPORT_MTK_LIB(mtk_conn_watch);
    EXPORT_MTK_LIB(mtk_conn_connected);
}
mtk::Server *MonoHandler::NewServer(Server::Address *a, Server::Config *c) {
	auto *sv = new mtk::Server();
	return &((*sv).SetAddress(*a).SetConfig(*c));
}
MonoMethod *MonoHandler::FindMethod(const std::string &name) {
	MonoMethodDesc *d = mono_method_desc_new(name.c_str(), true);
	if (d == nullptr) {
		return nullptr;
	}
	MonoMethod *m = mono_method_desc_search_in_image(d, image_);
	if (m == nullptr) {
		mono_method_desc_free(d);
		return nullptr;
	}
	mono_method_desc_free(d);
	return m;
}
MonoArray *MonoHandler::FromArgs(int argc, char *argv[]) {
	auto *str_class = mono_class_from_name(image_, "System", "String");
	auto *a = mono_array_new(domain_, str_class, argc);
	for (int i = 0; i < argc; i++) {
		auto *str = mono_string_new(domain_, argv[i]);
		mono_array_set(a, MonoString*, i, str);
	}
	return a;
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
	assembly_ = mono_domain_assembly_open(domain_, "Server.dll");
	if (assembly_ == nullptr) {
		LOG(error, "fail to load assembly");
		return nullptr;
	}
	image_ = mono_assembly_get_image(assembly_);
	if (image_ == nullptr) {
		LOG(error, "fail to get image");
		return nullptr;
	}
	if ((login_ = FindMethod("Mtk.EntryPoint::Login()")) == nullptr) {
		LOG(error, "fail ot get method login");
		return nullptr;
	}
	if ((handle_ = FindMethod("Mtk.EntryPoint::Handle()")) == nullptr) {
		LOG(error, "fail ot get method handle_");
		return nullptr;
	}
	if ((close_ = FindMethod("Mtk.EntryPoint::Close()")) == nullptr) {
		LOG(error, "fail ot get method close_");
		return nullptr;
	}

	//create server instance
	MonoMethod *bootstrap;
	if ((bootstrap = FindMethod("Mtk.EntryPoint::Bootstrap()")) == nullptr) {
		LOG(error, "fail ot get method bootstrap");
		return nullptr;
	}
	MonoObject *svptr, *err = nullptr;
	void *args[1] = { (void *)FromArgs(argc, argv) };
	if ((svptr = mono_runtime_invoke(bootstrap, nullptr, args, &err)) == nullptr || err != nullptr) {
		LOG(error, "fail ot call method bootstrap");
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
}
void MonoHandler::TlsFin(Worker *w) {
	mono_thread_detach(thread_);
}
}
