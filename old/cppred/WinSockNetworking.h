#pragma once
#if defined _WIN32 || defined _WIN64

#include "HostSystemServiceProviders.h"
#include <WinSock2.h>

class WinsockNetworkProvider : public NetworkProvider{
public:
	WinsockNetworkProvider();
	~WinsockNetworkProvider();
	NetworkProviderConnection *create_connection() override;
};

class WinsockConnection : public NetworkProviderConnection{
	unsigned port;
	std::string hostname;
	SOCKET socket = INVALID_SOCKET;

	void close();
public:
	WinsockConnection(WinsockNetworkProvider &provider);
	~WinsockConnection();
	void configure_as_server(unsigned port);
	void configure_as_client(const std::string &server_hostname, unsigned port);
	bool open() override;
	void abort() override;
	void send_data(const std::vector<byte_t> &) override;
	void send_data(const void *, size_t) override;
};

#endif
