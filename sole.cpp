#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <fstream>
#include <urlmon.h>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <thread>
#include <chrono>
#include "httplib.h"
#include <TlHelp32.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "urlmon.lib")
std::filesystem::path cs2_path = "";
std::string tetst_dir = "";
using json = nlohmann::json;
DWORD getProcessPid(const wchar_t* szProcName) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) return 0;
	PROCESSENTRY32W pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(hSnap, &pe32)) return 0;
	do {
		if (lstrcmpiW(szProcName, pe32.szExeFile) == 0)
			return pe32.th32ProcessID;
	} while (Process32NextW(hSnap, &pe32));
	return 0;
}
bool LaunchExecutable(const std::filesystem::path& exe_path, const std::string& command_line_args, int max_attempts) {
	// 检查文件是否存在
	int PID = 0;
	if (getProcessPid(L"cs2.exe") != 0) {
		std::cout << "CS2已启动" << std::endl;
		return true;
	}
	if (!std::filesystem::exists(exe_path)) {
		std::cerr << "错误: 文件不存在 " << exe_path << std::endl;
		return false;
	}
	// 转换路径为字符串以用于命令行
	//std::string command = "\"" + exe_path.string() + "\" " + command_line_args;
	int attempts = 0;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	while (attempts < max_attempts) {
		std::cout << "执行命令:" << command_line_args << std::endl;;
		// 执行命令
		// 组装命令行，确保可执行文件路径被双引号包围
		std::string cmd = "\"" + exe_path.string() + "\" " + command_line_args;

		// CreateProcess 需要非const字符串作为参数
		std::vector<char> cmdLine(cmd.begin(), cmd.end());
		cmdLine.push_back('\0'); // 添加字符串结束符

	if (!CreateProcessA(NULL,   // 不使用模块名称，使用命令行
		cmdLine.data(),        // 命令行
		NULL,           // 进程句柄不可继承
		NULL,           // 线程句柄不可继承
		FALSE,          // 设置句柄继承属性
		0,              // 没有创建标志
		NULL,           // 使用父进程的环境块
		NULL,           // 使用父进程的起始目录
		&si,            // 指向STARTUPINFO结构的指针
		&pi)            // 指向PROCESS_INFORMATION结构的指针
		)
	{
		std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
		return false;
	}
		// 等待一段时间让程序启动
		std::this_thread::sleep_for(std::chrono::seconds(20)); // 等待20秒
		PID = getProcessPid(L"cs2.exe");
		std::cout << "PID ：" << PID << std::endl;;
		if (PID != 0) {
			std::cout << "程序已启动" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(20));
			return true;
		}
		else {
			// 等待10秒
			std::this_thread::sleep_for(std::chrono::seconds(10));
			attempts++;
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	std::cerr << "尝试了 " << max_attempts << " 次后失败，不再重试。" << std::endl;
	return false;
}

class VConsoleClient {
private:
	SOCKET sock;
	struct sockaddr_in server;

public:
	VConsoleClient() : sock(INVALID_SOCKET) {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			throw std::runtime_error("WSAStartup failed.");
		}
	}

	~VConsoleClient() {
		Disconnect();
		WSACleanup();
	}

	bool Connect(const std::string& ipAddress, int port) {
		if (sock != INVALID_SOCKET) {
			return false;
		}

		// 创建socket
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) {
			throw std::runtime_error("无法创建套接字.");
		}

		server.sin_addr.s_addr = inet_addr(ipAddress.c_str());
		server.sin_family = AF_INET;
		server.sin_port = htons(port);

		// 连接到服务器
		if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
			sock = INVALID_SOCKET; // Reset socket to invalid
			std::filesystem::path upload_path = cs2_path /"game" / "bin" / "win64" / "cs2.exe";
			std::string command_line_args = "-addon " + tetst_dir + " -tools -steam -retail -gpuraytracing";
			if (!LaunchExecutable(upload_path, command_line_args, 10)) {
				std::cerr << "程序无法启动" << std::endl;
				std::cerr << "未知错误建议联系小莫修复" << std::endl;
				system("pause");
			}
			else {
				std::cerr << "启动CS2成功" << std::endl;
				Sleep(5000);
				
				return false;
			}
			
		}
		std::cout << "已连接到服务器.\n";
		return true;
	}

	void SendCommand(const std::string& command) {
		if (sock == INVALID_SOCKET) {
			throw std::runtime_error("未连接到服务器.");
		}
		EncodeAndSend(sock, "CMND", command);
	}

	void Disconnect() {
		if (sock != INVALID_SOCKET) {
			closesocket(sock);
			sock = INVALID_SOCKET;
			std::cout << "已断开与服务器的连接.\n";
		}
	}

private:

	bool cs2_2() {
		std::cout << "正在重启CS2" << std::endl;
		std::filesystem::path upload_path = cs2_path / "game" / "bin" / "win64" / "cs2.exe";
		std::string command_line_args = "-addon " + tetst_dir + " -tools -steam -retail -gpuraytracing";
		if (!LaunchExecutable(upload_path, command_line_args, 10)) {

			std::cout << "程序无法启动" << std::endl;
			return false;
		}
		else {
			std::this_thread::sleep_for(std::chrono::seconds(20)); // 等待20秒
			if (!Connect("127.0.0.1", 29000)) {
				if (!Connect("127.0.0.1", 29000)) {
					throw std::runtime_error("连接失败.");
					return false;
				}
			}
			return true;
		}
	}

	void EncodeAndSend(SOCKET sock, const std::string& identifier, const std::string& data) {
		if (sock == INVALID_SOCKET)
			throw std::runtime_error("未连接到服务器.");
		std::vector<unsigned char> sendBuffer;
		// 添加标识符
		sendBuffer.insert(sendBuffer.end(), identifier.begin(), identifier.end());
		// 添加分隔符和固定字节
		sendBuffer.push_back(0x00);
		sendBuffer.push_back(0xD4);
		// 计算长度并添加到缓冲区，注意长度包括了后面所有的字节
		uint32_t length = static_cast<uint32_t>(data.size() + 12); // 12由于标识符(4) + 分隔符和固定字节(2) + 长度(4) + 填充(2)
		uint32_t networkLength = htonl(length);
		// 将长度信息以大端字节序方式添加到缓冲区
		auto lengthPtr = reinterpret_cast<unsigned char*>(&networkLength);
		sendBuffer.insert(sendBuffer.end(), lengthPtr, lengthPtr + sizeof(networkLength));
		// 添加填充字节
		sendBuffer.push_back(0x00); // Padding
		sendBuffer.push_back(0x00); // Padding
		// 添加数据
		sendBuffer.insert(sendBuffer.end(), data.begin(), data.end());
		// 发送数据
		int bytesSent = send(sock, reinterpret_cast<const char*>(sendBuffer.data()), static_cast<int>(sendBuffer.size()), 0);
		if (bytesSent == SOCKET_ERROR) {
			// 检查和处理错误
			int errorCode = WSAGetLastError();
			std::cerr << "错误: " << errorCode << std::endl;
			if (errorCode == 10054) {
				cs2_2();
			}
			else {
				std::cerr << "未知错误建议联系小莫修复" << std::endl;
				system("pause");
			}
		}
	}
};
// 用于JSON解析
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
	size_t newLength = size * nmemb;
	try {
		s->append((char*)contents, newLength);
	}
	catch (std::bad_alloc& e) {
		// handle memory problem
		return 0;
	}
	return newLength;
}

// 回调函数来处理读取文件数据
size_t read_callback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t retcode = fread(ptr, size, nmemb, stream);
	return retcode;
}
bool wait_for_file(const std::string& file_path, int timeout_seconds) {
	auto start = std::chrono::steady_clock::now();
	std::filesystem::path path(file_path);

	// 循环检查文件是否存在且非空
	while (true) {
		// 检查是否超时
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > timeout_seconds) {
			std::cerr << "等待文件超时。\n";
			return false;
		}

		// 检查文件是否存在且大小大于0
		if (std::filesystem::exists(path) && std::filesystem::file_size(path) > 0) {
			return true;
		}
		// 等待一段时间再次检查
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
std::string UploadFile(const std::string& file_path) {
		// 确保文件存在并可访问
		if (!wait_for_file(file_path, 10)) {
			std::cout << "等待文件超时!\n";
			return "0";
		}
	
		return "200";
	}
std::string get_project_name_from_json(const std::string& json_path) {
	std::ifstream json_stream(json_path);
	nlohmann::json json_data;

	std::string project_name;

	if (json_stream.is_open()) {
		try {
			json_stream >> json_data;

			if (json_data.contains("project_name")) {
				project_name = json_data["project_name"];
			}
			else {
				// 如果没有找到 "project_name" 键，可能需要处理错误或设置默认值
				std::cerr << "没找到project_name JSON ." << std::endl;
			}
		}
		catch (const nlohmann::json::parse_error& e) {
			// JSON解析错误处理
			std::cerr << "解析错误 error: " << e.what() << std::endl;
		}
	}
	else {
		// 文件打开错误处理
		std::cerr << "无法打开JSON文件." << std::endl;
	}

	return project_name;
}
std::string get_executable_path() {
	std::vector<char> path_buffer(MAX_PATH);
	DWORD copied = GetModuleFileNameA(NULL, path_buffer.data(), path_buffer.size());
	while (copied >= path_buffer.size()) {
		path_buffer.resize(path_buffer.size() * 2);
		copied = GetModuleFileNameA(NULL, path_buffer.data(), path_buffer.size());
	}

	return std::string(path_buffer.begin(), path_buffer.begin() + copied);
}
int main(int argc, char* argv[]) {
	try {
		VConsoleClient client;
		std::string filename = "mp3.mp3";//音乐文件名
		std::string readBuffer="";
		std::string exe_path = get_executable_path();
		std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
		std::string ini_path = exe_dir.string() + "\\name.json";
		tetst_dir = get_project_name_from_json(ini_path);
		bool for_b = false;
		for (int i = 0; i < 4; ++i) { // 回退四级目录，从 'sounds' 开始
			exe_dir = exe_dir.parent_path();
		}
		cs2_path = exe_dir;
		if (!client.Connect("127.0.0.1", 29000)) {
			if (!client.Connect("127.0.0.1", 29000)) {
				throw std::runtime_error("连接失败.");
			}
		}
			client.SendCommand("play " + filename);
			client.Disconnect();
			std::filesystem::path upload_path = exe_dir / "game" / "csgo_addons" / tetst_dir / "sounds" / filename;
			upload_path.replace_extension(".vsnd_c"); // 更改文件扩展名
			std::string file_to_upload = upload_path.string(); // 替换为你要上传文件的路径
			if (wait_for_file(filename, 10)) {
				std::cout << "转换" << file_to_upload << "成功!" << std::endl;
		    }
		    else {
			std::cout << "文件无效";
		     }
		
	}
	catch (const std::exception& e) {
		std::cerr << "try捕捉到错误：" << e.what() << std::endl;
		std::ofstream error_log("error_log.txt", std::ios::app);
		if (error_log.is_open()) {
			error_log << "try捕捉到错误：" << e.what() << std::endl;
			error_log.close(); // 关闭文件
		}
		else {
			std::cerr << "无法打开错误日志文件。" << std::endl;
		}
	}
	
	return 0;
}