// DumpHeapMemory.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define BUF_SIZE 4096
#define COL_WIDTH 32

void heap(int pid, int hid) {
	HEAPENTRY32 entry;
	char filename[MAX_PATH];
	char msg[BUF_SIZE];
	unsigned char buf[BUF_SIZE];
	DWORD bufsize;
	DWORD len = 0;
	DWORD bytes;
	FILE* file;
	char* start;
	char* a;

	entry.dwSize = sizeof(HEAPENTRY32);

	BOOL result = Heap32First(&entry, pid, hid);

	if (result) {
		sprintf(filename, "%d_%lx.dump", pid, entry.dwAddress);

		file = fopen(filename, "w");
		if (file == NULL) {
			perror(msg);
			fprintf(stderr, "heap: fopen failed: %s\n", msg);
			return;
		}
		do {
			ULONG_PTR addr = entry.dwAddress;
			SIZE_T size = entry.dwBlockSize;

			start = reinterpret_cast<char*>(addr);
			for (a=start; a<(start+size); a+=BUF_SIZE) {
				// calc bufsize
				if ((size - len) < BUF_SIZE) bufsize = size - len;
				else                         bufsize = BUF_SIZE;

				if (!Toolhelp32ReadProcessMemory(pid, a, &buf, bufsize, &bytes)) {
					fclose(file);
					perror(msg);
					fprintf(stderr, "heap: ToolHelp32ReadProcessMemory failed: %s\n", msg);
					return;
				}                                                                                                                                     

				len += bytes;

				fwrite(buf, sizeof(unsigned char), bytes, file);
			}

		} while (Heap32Next(&entry));
		fclose(file);
	}
}
void list(int pid) {
	HANDLE h;
	HEAPLIST32 list;

	list.dwSize = sizeof(HEAPLIST32);

	h = CreateToolhelp32Snapshot(TH32CS_SNAPALL, pid);

	if (h != NULL) {
		BOOL result = Heap32ListFirst(h, &list);
		do {
			pid = list.th32ProcessID;
			int hid = list.th32HeapID;
			int flag = list.dwFlags;
			heap(pid, list.th32HeapID);
		} while (Heap32ListNext(h, &list));

		CloseHandle(h);
	}
}


void usage(_TCHAR** argv) {
	printf("Usage: %s <pid>\n", argv[0]);
	exit(-1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	int pid;
	if (argc < 2) usage(argv);

	pid = _tstoi(argv[1]);
	wprintf(L"pid=%d\n", pid);                                                                                                                                    

	list(pid);

	return 0;
}

