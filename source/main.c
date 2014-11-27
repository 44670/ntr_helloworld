
#include "global.h"



Handle fsUserHandle;
FS_archive sdmcArchive = {0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};

u32 sayHello() {
	showDbg("hello world!", 0, 0);
	return 0;
}

int main() {

	initSharedFunc();
	nsDbgPrint("initializing helloworld plugin\n");
	plgGetSharedServiceHandle("fs:USER", &fsUserHandle);
	plgRegisterMenuEntry(1, "Say Hello", sayHello);

}

