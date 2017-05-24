#include "server.h"
#include "handler.h"
#include "config.h"

int main(int argc, char *argv[]) {
	auto h = new mtk::MonoHandler();
	auto sv = std::unique_ptr<mtk::Server>(h->Init(argc, argv));
	if (sv == nullptr) {
		LOG(fatal, "fail to start mtkdn");
		delete h;
		return -1;
	}
	sv->Run();
	return 0;
}
