#include "LeagueConnector.h"
#include "Base64.h" // TODO: learn more about linker process so we can tidy up includes

LeagueConnector::LeagueConnector(const std::wstring& leagueRootDir)
{
    std::wstring finalLockFileLocation;

    if (leagueRootDir.empty())
    {
        DWORD pid{ 0 };
        while (true)
        {
            GetProcessIdByName(m_leagueProcessName, pid);

            if (pid == 0)
            {
                constexpr int timeoutInSeconds{ 5 };
                std::cout << "League process not found, checking again in " << timeoutInSeconds << " seconds\n";
                std::this_thread::sleep_for(std::chrono::seconds(timeoutInSeconds));
            }
            else
            {
                std::cout << "Process found, working...\n";
                break;
            }
        }

        std::wstring processLocation;
        GetProcessLocation(pid, processLocation);

        // so we can exclude the .exe
        const size_t lastSlashIndex{ std::wstring{ processLocation }.find_last_of(L'\\') };
        finalLockFileLocation = std::wstring{ processLocation }.substr(0, lastSlashIndex) + L'\\' + std::wstring{ m_lockFileName };
    }
    else
    {
        finalLockFileLocation = leagueRootDir + L'\\' + std::wstring{ m_lockFileName };
    }

    constexpr int retries{ 5 };
    if (!IsFileExistent(finalLockFileLocation, retries))
    {
        std::wcerr << L"File: '" << finalLockFileLocation << L"' does not exist!\n";
        throw std::exception("Lock file not found");
    }

    std::string fileContents;
    GetFileContents(finalLockFileLocation, fileContents);
    if (fileContents.empty())
    {
        throw std::exception("Lock file is empty");
    }

    std::vector<std::string> params;
    size_t last{ 0 };
    size_t next;
    while ((next = fileContents.find(':', last)) != std::string::npos)
    {
        params.push_back(fileContents.substr(last, next - last));
        last = next + 1;
    }
    params.push_back(fileContents.substr(last));

    m_port = params[2];
    m_password = params[3];
    m_protocol = params[4];
    m_encodedAuth = base64_encode(m_username + ":" + m_password);
    m_url = m_protocol + "://" + m_host + ":" + m_port;
}

std::ostream& operator<<(std::ostream& out, const LeagueConnector& lc)
{
    out << "Port: " << lc.m_port << '\n';
    out << "Password: " << lc.m_password << '\n';
    out << "Protocol: " << lc.m_protocol << '\n';
    out << "Encoded basic auth: " << lc.m_encodedAuth << '\n';
    return out;
}

void LeagueConnector::GetFileContents(const std::wstring& filePath, std::string& outContents)
{
    std::ifstream fStream{ filePath };
    // If we couldn't open the output file stream for reading
    if (!fStream)
    {
        // Print an error and exit
        std::wcerr << "file '" << filePath << "' could not be opened for reading!\n";
        throw std::exception("Failed opening file for reading");
    }

    const std::string lockFileContents((std::istreambuf_iterator<char>(fStream)),
        std::istreambuf_iterator<char>());

    // TODO: maybe check here?
    outContents = lockFileContents;
}

void LeagueConnector::GetProcessLocation(DWORD pid, std::wstring& outProcessLocation)
{
    wchar_t processLocationBuf[MAX_PATH];
    const HANDLE handle{ OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };
    if (!handle)
    {
        throw std::exception("Can't open handle to process");
    }

    const DWORD result{ GetModuleFileNameExW(handle, nullptr, processLocationBuf, MAX_PATH) };
    CloseHandle(handle);
    if (result == 0)
    {
        throw std::exception("Can't get process location");
    }

    outProcessLocation = processLocationBuf;
}

void LeagueConnector::GetProcessIdByName(const std::wstring& pName, DWORD& outPid)
{
    if (pName.empty())
    {
        throw std::exception("Process name is empty");
    }

    // Take a snapshot of all processes in the system.
    const HANDLE hProcessSnap{ CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) };
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        throw std::exception("CreateToolhelp32Snapshot (of processes)");
    }
    PROCESSENTRY32 pe32;
    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        throw std::exception("Process32First");
    }

    do
    {
        const HANDLE hProcess{ OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID) };
        if (hProcess == nullptr)
            continue;

        const DWORD dwPriorityClass{ GetPriorityClass(hProcess) };
        if (!dwPriorityClass)
        {
            throw std::exception("GetPriorityClass");
        }
        CloseHandle(hProcess);

        if (wcscmp(pe32.szExeFile, pName.c_str()) == 0)
        {
            outPid = pe32.th32ProcessID;
            break;
        }

    } while (Process32Next(hProcessSnap, &pe32));
}

inline bool LeagueConnector::IsFileExistent(const std::wstring& fName, const int retries)
{
    int retryCnt{ 0 };
    do
    {
        const std::wifstream f(fName.c_str());
        if (f.good())
        {
            std::wcout << "File " << fName << " was found, reading...\n";
            return true;
        }

        std::wcout << "File " << fName << " not found, retrying...\n";

        constexpr int timeoutBetweenChecks{ 5 };
        std::this_thread::sleep_for(std::chrono::seconds(timeoutBetweenChecks));

    } while (retryCnt++ < retries);

    return false;
}