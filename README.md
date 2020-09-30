# LCU Connector

Inspired by [Pipix's lcu connector](https://github.com/Pupix/lcu-connector)
<br>
The connector parses the credentials from the lockfile for the API
<br>
Make sure you check out the [API reference](https://riot-api-libraries.readthedocs.io/en/latest/lcu.html)

#### Features
- Upon initialization you can either provide League of Legends installation directory to the constructor - or leave it empty and just launch league client.
- File checking, timeouts and reasonable console output
- Modern code style (at least I am trying)

### How to build
- Load up the project (.sln)
- Build -> Build solution | ctrl+B

Note: if you are getting strange build errors make sure that the project configuration type is set to 'Static library (.lib)'

### Usage
- Include everything in your desired project, or straight up create a main method in the same project.
- Initialize
- Get creative with the HTTP requests

### Samples
- Init with League home dir as parameter
<pre><code>const LeagueConnector conn{L"D:\\Games\\League of Legends"};
std::cout << conn;</code></pre>
- Init without parameters (will retrieve path from client process)
<pre><code>const LeagueConnector conn;
std::cout << conn;</code></pre>
- How I use it (Aram boost)
<pre><code>int main()
{
    try
    {
        const LeagueConnector conn{L"D:\\Games\\League of Legends"};
        std::cout << conn;

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        const std::wstring headers{ L"Content-Type: application/json\r\nAccept: application/json\r\nAuthorization: Basic " + converter.from_bytes(conn.getEncodedAuth()) };

        std::wcout << "URL: " << converter.from_bytes(conn.getUrl()) << "\n";

        const auto host{ converter.from_bytes(conn.getHost()) };
        const std::wstring path{ L"/lol-champ-select/v1/team-boost/purchase/" };

        executeHttpsPost(headers.c_str(), host.c_str(), path.c_str(), std::stoi(conn.getPort()));

        std::cout << "Boost applied!\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what();
    }
    return 0;
}</code></pre>

- Output
<br>
![console output img](https://i.imgur.com/ywUDYx8.png)