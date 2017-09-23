#if (defined _WIN32 || defined _WIN64) && 0
#include "WinSockNetworking.h"
#include "exceptions.h"
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif

WinsockNetworkProvider::WinsockNetworkProvider(){
	WSADATA data;
	auto error = WSAStartup(MAKEWORD(2, 2), &data);
	if (error){
		std::stringstream stream;
		stream << "WSAStartup() failed with error 0x" << std::hex << std::setw(8) << std::setfill('0') << error;
		throw GenericException(stream.str());
	}
	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2){
		WSACleanup();
		throw GenericException("WSAStartup() did not return a usable DLL version.");
	}
}

WinsockNetworkProvider::~WinsockNetworkProvider(){
	this->connections.clear();
	WSACleanup();
}

NetworkProviderConnection *WinsockNetworkProvider::create_connection(){
	this->connections.emplace_back(std::unique_ptr<NetworkProviderConnection>(new WinsockConnection(*this)));
	return this->connections.back().get();
}

WinsockConnection::WinsockConnection(WinsockNetworkProvider &provider) : NetworkProviderConnection(provider){}

WinsockConnection::~WinsockConnection(){
	this->close();
}

void WinsockConnection::configure_as_server(unsigned port){
	this->hostname.clear();
	this->port = port;
}

void WinsockConnection::configure_as_client(const std::string &server_hostname, unsigned port){
	this->hostname = server_hostname;
	this->port = port;
}

bool WinsockConnection::open(){
	this->close();
	if (this->hostname.size()){
		//Create client socket.
		sockaddr addr;
		memset(&addr, 0, sizeof(addr));
		auto host = gethostbyname(this->hostname.c_str());
		if (!host){
			auto error = WSAGetLastError();
			std::cerr << "gethostbyname() failed with error: " << error << std::endl;
			return false;
		}

		for (int i = 0; !host->h_addr_list[i]; i++){
			this->socket = ::socket(host->h_addrtype, SOCK_STREAM, IPPROTO_TCP);
			if (this->socket == INVALID_SOCKET){
				auto error = WSAGetLastError();
				std::cerr << "socket() failed with error: " << error << std::endl;
				return false;
			}

			addr.sa_family

			auto result = connect(this->socket, host->h_addr_list[i], host->h_length);
			this->close();
		}

	}else{
		//Create server socket.
	}
}

void WinsockConnection::abort(){
	
}

void WinsockConnection::send_data(const std::vector<byte_t> &buffer){
	
}

void WinsockConnection::send_data(const void *buffer, size_t size){
	
}

void WinsockConnection::close(){
	if (this->socket == INVALID_SOCKET)
		return;
	closesocket(this->socket);
	this->socket = INVALID_SOCKET;
}

#endif
