#include "handler.h"
#include "mtk/src/mtk.h"
#include <stdlib.h>

extern "C" {
	void mono_object_describe_fields (MonoObject *obj);
}
void *mtkdn_server(mtk_addr_t *a, int n_addr, mtk_svconf_t *c) {
	return mtk::MonoHandler::NewServer(a, n_addr, c);
}

namespace mtk {
#define MTKLIB_NS "Mtk.Core"
#define EXPORT_MTK_LIB(method) { \
	mono_add_internal_call(MTKLIB_NS"::mtkdn_"#method, (void *)&mtk_##method); \
}
#define DO(stmt, ...) { \
	if ((stmt) == nullptr) { \
		LOG(error, __VA_ARGS__); \
		return nullptr; \
	} \
}
#define INVOKE(stmt, err, ...) { \
	err = nullptr; \
	if ((stmt) == nullptr || err != nullptr) { \
		LOG(error, __VA_ARGS__); \
		DumpException(err); \
		return nullptr; \
	} \
}

thread_local MonoThread *MonoHandler::thread_;

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
mtk::Server *MonoHandler::NewServer(Server::Address *a, int n_addr, Server::Config *c) {
	auto *sv = new mtk::Server();
	sv->SetAddress(a, n_addr).SetConfig(*c);
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
MonoObject *MonoHandler::CallVirtual(MonoObject *self, const char *method, void *args[], int n_args) {
	auto klass = mono_object_get_class(self);
	if (klass == nullptr) {
		LOG(error, "ev:CallVirtual fail to get class");
		mono_object_describe_fields(self);
		ASSERT(false);
		return nullptr;
	}
	auto kname = mono_class_get_name(klass);
	auto virt_method = mono_class_get_method_from_name(klass, method, n_args);
	if (virt_method == nullptr) {
		if (n_args <= 0) {
			auto prop = mono_class_get_property_from_name (klass, method);
			virt_method = mono_property_get_get_method (prop);
		}
		if (virt_method == nullptr) {
			LOG(error, "ev:CallVirtual fail to get method,class:{},method:{}", kname, method);
			mono_object_describe_fields(self);
			ASSERT(false);
			return nullptr;
		}
	}
	virt_method = mono_object_get_virtual_method(self, virt_method);
	if (virt_method == nullptr) {
		LOG(error, "ev:CallVirtual fail to get virtual method,class:{},method:{}", kname, method);
		mono_object_describe_fields(self);
		ASSERT(false);
		return nullptr;
	}
	MonoObject *exc = nullptr;
	auto obj = mono_runtime_invoke(virt_method, self, args, &exc);
	if (exc != nullptr) {
		LOG(error, "ev:CallVirtual exception raised,class:{},method:{}", kname, method);
		mono_object_describe_fields(exc);
		return nullptr;
	}
	return obj;
}
MonoClass *MonoHandler::GetLogicClass(MonoImage *image) {
	const char *klass_name = getenv("MTKSV_LOGIC");
	if (klass_name == nullptr) {
		LOG(error, "ev:fail to get logic class");
		return nullptr;
	} 
	const char *find = strrchr(klass_name, '.');
	size_t sz = find - klass_name;
	char ns[sz + 1];
	strncpy(ns, klass_name, sz);
	ns[sz] = 0;
	return mono_class_from_name(image, ns, find + 1); 
}
void MonoHandler::DumpException(MonoObject *exc) {
	MonoObject *pexc;
	while (true) {
		pexc = CallVirtual(exc, "InnerException", nullptr, 0);
		if (pexc == nullptr) {
			break;
		} else {
			exc = pexc;
		}
	}
	auto msg = CallVirtual(exc, "Message", nullptr, 0), 
		 stack = CallVirtual(exc, "StackTrace", nullptr, 0);
	if (msg == nullptr) {
		LOG(error, "ev:fail to get data from exception");
		mono_object_describe_fields(exc);
		return;
	}
	//mono_object_describe_fields(exc);
	if (stack != nullptr) {
		auto msgstr = mono_string_to_utf8((MonoString *)msg), 
			 stkstr = mono_string_to_utf8((MonoString *)stack);
		LOG(error, "ev:mono exception,msg:{},at:{}", msgstr, stkstr);
		mono_free(msgstr); 
		mono_free(stkstr);
	} else {
		auto msgstr = mono_string_to_utf8((MonoString *)msg);
		LOG(error, "ev:mono exception,msg:{}", msgstr);
		mono_free(msgstr); 
	}
}
mtk::Server *MonoHandler::Init(int argc, char *argv[]) {
	mono_mkbundle_init();
	AddInternalCalls();

	MonoAssembly *assembly;
	MonoImage *image;
	MonoClass *logic_class;
	MonoObject *err;
	mtk::Server *sv;

	DO(domain_ = mono_jit_init("mtkdn"), "ev:fail to init domain");
	DO(thread_ = mono_thread_attach(domain_), "ev:fail to attach thread");
	DO(assembly = mono_domain_assembly_open(domain_, "Server.dll"), "ev:fail to load server assembly");
	DO(image = mono_assembly_get_image(assembly), "ev:fail to get server image");
	DO(logic_class = GetLogicClass(image), "ev:fail to get logic class");
	{
		//create server instance with logic class
		MonoMethod *bootstrap, *build;
		MonoClass *builder_class;
		MonoObject *builder, *svptr;

		DO(bootstrap = FindMethod(logic_class, "Bootstrap"), "fail ot get method bootstrap");

		void *args[1] = { (void *)FromArgs(argc, argv) };
		INVOKE(builder = mono_runtime_invoke(bootstrap, nullptr, args, &err), err, 
				"ev:fail to call method logic::Bootstrap {} {}", (void *)builder, (void *)err);

		//call builder to retrieve server pointer
		DO(assembly = mono_domain_assembly_open(domain_, "Mtk.dll"), "ev:fail to load server assembly");
		DO(image = mono_assembly_get_image(assembly), "ev:fail to get server image");
		DO(builder_class = mono_class_from_name(image, "Mtk", "Core/ServerBuilder"), "ev:fail to get class Mtk.Core/ServerBuilder");
		DO(build = FindMethod(builder_class, "Build"), "ev:fail to get build method");
		INVOKE(svptr = mono_runtime_invoke(build, builder, nullptr, &err), err, 
				"ev:fail to call method Mtk.Core.ServerBuilder.Build {} {}", (void *)builder, (void *)err)
		DO(sv = (mtk::Server *)*(intptr_t *)mono_object_unbox(svptr), "bootstrap fail to create server");
	}
	{	
		//create logic class
		MonoMethod *factory;
		MonoClass *entrypoint_class;

		DO(factory = FindMethod(logic_class, "Instance"), "ev:fail to get method logic::Instance");
		INVOKE(logic_ = mono_runtime_invoke(factory, nullptr, nullptr, &err), err, 
			"ev:fail to call method logic::Instance {} {}", (void *)logic_, (void *)err);
	
		DO(assembly = mono_domain_assembly_open(domain_, "Mtk.dll"), "ev:fail to load server assembly");
		DO(image = mono_assembly_get_image(assembly), "ev:fail to get server image");
		DO(entrypoint_class = mono_class_from_name(image, "Mtk", "EntryPoint"), "ev:fail to get class Mtk.EntryPoint");

		DO(close_ = FindMethod(entrypoint_class, "Close"), "ev:fail to get method Mtk.EntryPoint.Close");
		DO(login_ = FindMethod(entrypoint_class, "Login"), "ev:fail to get method Mtk.EntryPoint.Login");
		DO(handle_ = FindMethod(entrypoint_class, "Handle"), "ev:fail to get method Mtk.EntryPoint.Handle");
	}
	//mono code does not seems to destroy MonoArray. its handled by gc?
	LOG(info, "ev:mono handler init success,sv:{}", (void *)sv);
	return &(sv->SetHandler(this));
}
void MonoHandler::Shutdown() {
	([this] () -> void * {
		LOG(info, "ev:start mono shutdown");
		MonoAssembly *assembly;
		MonoImage *image;
		MonoClass *entrypoint_class;
		MonoObject *err, *tmp;
		MonoMethod *shutdown;

		DO(assembly = mono_domain_assembly_open(domain_, "Mtk.dll"), "ev:fail to load server assembly");
		DO(image = mono_assembly_get_image(assembly), "ev:fail to get server image");
		DO(entrypoint_class = mono_class_from_name(image, "Mtk", "EntryPoint"), "ev:fail to get class Mtk.EntryPoint");

		DO(shutdown = FindMethod(entrypoint_class, "Shutdown"), "ev:fail to get method Mtk.EntryPoint.Shutdown");
		INVOKE(tmp = mono_runtime_invoke(shutdown, &logic_, nullptr, &err), err, 
			"ev:fail to call method Mtk.EntryPoint.Shutdown,logic:{},err:{}", (void *)logic_, (void *)err);	
		LOG(info, "ev:mono shutdown success");
	})();
}
void MonoHandler::TlsInit(Worker *w) {
	thread_ = mono_thread_attach(domain_);
	LOG(info, "ev:start worker thread,th:{},worker:{}", (void *)thread_, (void *)w);
}
void MonoHandler::TlsFin(Worker *w) {
	mono_thread_detach(thread_);
	LOG(info, "ev:end worker thread,th:{},worker:{}", (void *)thread_, (void *)w);
}
}
