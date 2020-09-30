#pragma once
#include <string>
#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

class LeagueConnector
{
private:
    const std::wstring m_leagueProcessName{ L"LeagueClient.exe" };
    const std::wstring m_lockFileName{ L"lockfile" };
    const std::string m_username{"riot"};
    const std::string m_host{ "127.0.0.1" };
    std::wstring m_lockFilePath;
    std::string m_port;
    std::string m_protocol;
    std::string m_password;
    std::string m_encodedAuth;
    std::string m_url;

    static void GetProcessIdByName(const std::wstring& pName, DWORD &outPid);
    static void GetProcessLocation(DWORD pid, std::wstring &outProcessLocation);
    static void GetFileContents(const std::wstring& filePath, std::string& outContents);
    static bool IsFileExistent(const std::wstring& fName, int retries = 0);
public:
    LeagueConnector(const std::wstring &leagueRootDir = L"");
    LeagueConnector(const LeagueConnector& lConn) = delete;
    ~LeagueConnector() = default;

    [[nodiscard]] std::string getPort() const { return m_port; }
    [[nodiscard]] std::string getProtocol() const { return m_protocol; }
    [[nodiscard]] std::string getEncodedAuth() const { return m_encodedAuth; }
    [[nodiscard]] std::string getUrl() const { return m_url; }
    [[nodiscard]] std::string getHost() const { return m_host; }

    friend std::ostream& operator<<(std::ostream& out, const LeagueConnector& lc);
};