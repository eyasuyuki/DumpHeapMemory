// DumpHeapMemory.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define BUF_SIZE 4096
#define COL_WIDTH 32



struct Heap
{
	int pid; // Process id where heaps of interest are located.
	HANDLE process_handle; // Handle to the process whose heap is being dumped
	HEAPENTRY32 entry; // Holds heap information.
	HEAPLIST32 heap_list; // Heap records

	void GetHeapInfo();
	void GetHeap();
};

void Heap::GetHeapInfo()
{
	heap_list.dwSize = sizeof(HEAPLIST32);
	process_handle = CreateToolhelp32Snapshot(TH32CS_SNAPALL, pid);
	if (process_handle != NULL)
	{
		BOOL result = Heap32ListFirst(process_handle, &heap_list);
		do {
			pid = heap_list.th32ProcessID;
			int hid = heap_list.th32HeapID;
			int flag = heap_list.dwFlags;
			GetHeap();

		} while (Heap32ListNext(process_handle, &heap_list));
		CloseHandle(process_handle);
	}
}


void Heap::GetHeap()
{
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
	BOOL result = Heap32First(&entry, pid, heap_list.th32HeapID);

	if (result) {
		sprintf(filename, "%d_%lx.dump", pid, entry.dwAddress);
		file = fopen(filename, "w");
		if (file == NULL) {
			perror(msg);
			fprintf(stderr, "heap: fopen failed: %s\n", msg);
			return;
		}
		do {
			start = reinterpret_cast<char*>(entry.dwAddress);
			for (a = start; a<(start + entry.dwBlockSize); a += BUF_SIZE) {
				// calc bufsize
				if ((entry.dwBlockSize - len) < BUF_SIZE) bufsize = entry.dwBlockSize - len;
				else
					bufsize = BUF_SIZE;

				if (!Toolhelp32ReadProcessMemory(pid, a, &buf, bufsize, &bytes)) 
				{
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


void usage(_TCHAR** argv) {
	printf("Usage: %s <pid>\n", argv[0]);
	exit(-1);
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		usage(argv);
	}
	Heap heap;
	heap.pid = _tstoi(argv[1]);
	heap.GetHeapInfo();
	return 0;
}

