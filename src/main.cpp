#include "handler.h"
#include "config.h"

int main(int argc, char *argv[]) {
	Config c;
	c.Parse(argc, argv);

	Server sv(c);
	sv.Run();
	return 0;
}
