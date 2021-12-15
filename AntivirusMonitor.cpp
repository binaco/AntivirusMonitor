#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>

DWORD __stdcall ThreadProc(LPVOID lpParameter)
{
	std::wstring bannedProcess = *static_cast<std::wstring*>(lpParameter);
	std::wcout << bannedProcess << '\n';
	return 0;
}

int main()
{
	std::wstring str;
	std::getline(std::wcin, str);
	std::wstringstream stream;
	stream << str;
	std::vector<std::wstring> bannedProcesses;
	while (stream)
	{
		std::wstring temp;
		stream >> temp;
		bannedProcesses.push_back(temp);
	}

	std::vector<HANDLE> threads;
	for (size_t i = 0; i < bannedProcesses.size(); ++i)
	{
		threads.push_back(CreateThread(0, 0, ThreadProc, &bannedProcesses[i], 0, 0));
	}

	if (!threads.empty())
	{
		WaitForMultipleObjects(threads.size(), &threads[0], TRUE, INFINITE);
	}
}