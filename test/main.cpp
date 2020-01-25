#include <iostream>

#include "../src/timestamp.h"
#include "../src/log.h"


int main() {
	abc::timestamp ts1;

	std::cout << ts1.year() << "-" << ts1.month() << "-" << ts1.day() << std::endl;

	return 0;
}
