#include "server.h"
#include "handler.h"
#include "config.h"

int main(int argc, char *argv[]) {
	mtk::Config c;
	c.Parse(argc, argv);
	c.ConfigFileValue("c", "name", "value");

	mtk::Server sv(c, new mtk::MonoHandler());
	sv.Run();
	return 0;
}
