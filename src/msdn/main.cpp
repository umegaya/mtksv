#include "server.h"
#include "handler.h"
#include "config.h"

int main(int argc, char *argv[]) {
	mtk::Config c;
	c.Parse(argc, argv);
	c.ConfigFileValue("c", "name", "value");

	auto h = new mtk::DotNetHandler();
	h->Init(c.Value<std::string>("a", "no_such_file"));
	mtk::Server sv(c, h);
	sv.Run();
	return 0;
}
