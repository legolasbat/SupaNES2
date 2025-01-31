#include "SupaNES.h"

SupaNES::SupaNES() {
	// TODO: Create pointers to elements
	cart = new Cartridge(this);
}


bool SupaNES::InsertROM(const char* filepath) {
	if (!cart->LoadROM(filepath)) {
		return false;
	}

	//TODO: Reset SupaNES
	return true;
}

//SupaNES::~SupaNES() {
	// TODO: Free memory
//}
