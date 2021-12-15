#include <iostream>
#include <sstream>
#include <vector>
#include <mutex>
#include <windows.h>
#include <stdio.h>
#include <psapi.h>

size_t g_killedProcessesCount = 0;

class HandleWrapper
{
public:
    HandleWrapper(HANDLE handle) :
        m_handle(handle)
    {

    }
    ~HandleWrapper()
    {
        if (m_handle && m_handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_handle);
        }
    }
    HANDLE Get() const
    {
        return m_handle;
    }
private:
    HANDLE m_handle = INVALID_HANDLE_VALUE;
};

bool KillProcessByName(DWORD processID, const std::wstring& name)
{
    HandleWrapper process(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID));
    if (!process.Get())
    {
        return 0;
    }

    std::wstring processName(MAX_PATH, L'\0');
    DWORD nameSize = GetModuleBaseName(process.Get(), 0, &processName[0], processName.length());
    if (!nameSize)
    {
        return 0;
    }
    processName.resize(nameSize);

    if (processName == name)
    {
        if (!TerminateProcess(process.Get(), 1))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

DWORD __stdcall ThreadProc(LPVOID lpParameter)
{
	std::wstring bannedProcess = *static_cast<std::wstring*>(lpParameter);
    std::mutex lock;
    while (g_killedProcessesCount < 5)
    {
        //lock.lock();
        Sleep(1000);
        DWORD processes[1024], bytesReturned;
        if (!EnumProcesses(processes, sizeof(processes), &bytesReturned))
        {
            std::cout << "Could not get processes ids" << std::endl;
        }
        DWORD processesAmount = bytesReturned / sizeof(DWORD);
        bool isKilled = false;
        for (size_t i = 0; i < processesAmount; ++i)
        {
            if (processes[i])
            {
                if (KillProcessByName(processes[i], bannedProcess))
                {
                    isKilled = true;
                }
            }
        }
        if (isKilled)
        {
            std::cout << "Process terminated successfully" << std::endl;
            ++g_killedProcessesCount;
        }
        //lock.unlock();
    }
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

    for (size_t i = 0; i < threads.size(); ++i)
    {
        if (threads[i] != INVALID_HANDLE_VALUE)
        {
            CloseHandle(threads[i]);
        }
    }
}