
void onFatalError(u32 lr) {
	acquireVideo();
	showDbg("fatal. LR: %08x", lr, 0);
	releaseVideo();
}

void patchGame() {
	u32 jmpCode[] = {0xe1a0000e, 0xe51ff004, (u32)onFatalError};
	memcpy((void*) 0x318A88, jmpCode, sizeof(jmpCode));
}

void launch() {
	u32 i, t = 0;
	u32 outAddr = 0;
	u8 buf[0x1010];
	u32 pid = 0, handle = 0, fileHandle = 0;
	u32 ret, ptr;
	typedef (*funcType)(void);

	//u32 ret = svc_addCodeSegment(0x260000, 0x373000);
	showDbg("&launch: %08x", (u32)launch, 0);
	

	svc_getProcessId(&pid, 0xffff8001);
	ret = svc_openProcess(&handle, pid);
	if (ret != 0) {
		showDbg("openProcess failed, ret: %08x", ret, 0);
		return;
	}
	
	ret = svc_controlMemory(&outAddr, 0x08000000, 0, 0, 0x5, 3);
	showDbg("freememory ret: %08x", ret, 0);
	
	ret = svc_controlMemory(&outAddr, 0x260000, 0x260000, 0x383000, 0x103, 3);
	xsprintf(buf, "ret: %08x, addr: %08x", ret, outAddr);
	showMsg(buf);
	
	ret = svc_controlProcessMemory(&outAddr, 0x260000, 0x260000, 0x383000, handle, 6, 7);
	xsprintf(buf, "ret: %08x, addr: %08x", ret, outAddr);
	showMsg(buf);
	
	*((vu32*)0x251000) = 0xe12fff1e; 
	ret = svc_flushProcessDataCache(0xffff8001, 0x00100000, 0x373000);
	((funcType)0x251000)();
	showMsg("cleared.");
	
	u8 fileName[] = "/load.bin";
	FS_path testPath = (FS_path){PATH_CHAR, strlen(fileName) + 1, fileName};
	ret = FSUSER_OpenFileDirectly(fsUserHandle, &fileHandle, sdmcArchive, testPath, 7, 0);
	if (fileHandle == 0) {
		showDbg("openFile failed: %08x", ret, 0);
	}
	u64 size64 = 0;
	FSFILE_GetSize(fileHandle, &size64);
	u32 size = (u32)size64;
	showDbg("size: %08x", size, 0);
	for (i = 0; i < size; i += 0x1000) {
		ptr = 0x00100000 + i;
		xsprintf(buf, "load: %08x", ptr);
		print(buf, 10, 10, 255, 0, 0);
		updateScreen();
		FSFILE_Read(fileHandle, &t, i, (void*) buf, 0x1000);
		memcpy((void*) ptr, buf, 0x1000);
	}
	showDbg("load ok", 0, 0);
	patchGame();
	ret = svc_flushProcessDataCache(0xffff8001, 0x00100000, size);
	showDbg("flush ret: %08x", ret, 0);
	releaseVideo();
	((funcType)0x100000)();
	
}
void loadRemoteProcess(u32 pid) {
	u32 ret, outAddr, hprocess, i, hfile, hdebug, ptr, t;
	u8* fileName = "/arm11.bin";
	u8 buf[0x1020];
	
	FS_path testPath = (FS_path){PATH_CHAR, strlen(fileName) + 1, fileName};
	ret = FSUSER_OpenFileDirectly(fsUserHandle, &hfile, sdmcArchive, testPath, 7, 0);
	if (ret != 0) {
		showDbg("openFile failed: %08x", ret, 0);
		goto final;
	}
	u64 size64 = 0;
	FSFILE_GetSize(hfile, &size64);
	u32 size = (u32)size64;
	showDbg("size: %08x", size, 0);
	
	ret = svc_debugActiveProcess(&hdebug, pid);
	if (ret != 0) {
		showDbg("debugActiveProcess failed: %08x", ret, 0);
		goto final;
	}
	
	ret = svc_openProcess(&hprocess, pid);
	if (ret != 0) {
		showDbg("openProcess failed: %08x", ret, 0);
		goto final;
	}
	
	ret = svc_controlProcessMemory(&outAddr, 0x100000, 0x100000, ((size / 0x1000) + 1) * 0x1000, hprocess, 6, 7);
	if (ret != 0) {
		showDbg("controlProcessMemory failed: %08x", ret, 0);
	}
	

	for (i = 0; i < size; i += 0x1000) {
		ptr = 0x00100000 + i;
		xsprintf(buf, "load: %08x", ptr);
		print(buf, 10, 10, 255, 0, 0);
		updateScreen();
		FSFILE_Read(hfile, &t, i, (void*) buf, 0x1000);
		ret = svc_writeProcessMemory(hdebug, buf, ptr, 0x1000);
		if (ret != 0) {
			showDbg("writeProcessMemory failed: %08x", ret, 0);
			goto final;
		}
	}
	
final:;
}
