#pragma once
#include "HostSystemServiceProviders.h"
#include "threads.h"
#include <deque>
#include "utility.h"

//Implements version 1.4
class BgbNetworkProtocol : public NetworkProtocol{
	struct packet{
		static const byte_t command_version         =      1;
		static const byte_t command_joypad          =    101;
		static const byte_t command_sync1           =    104;
		static const byte_t command_sync2           =    105;
		static const byte_t command_sync3           =    106;
		static const byte_t command_status          =    108;
		static const byte_t command_wantdisconnect  =    109;
		static const byte_t current_version_major   =      1;
		static const byte_t current_version_minor   =      4;
		static const byte_t sync_active_mode        = bit(0);
		static const byte_t sync_high_speed_mask    = bit(1);
		static const byte_t sync_double_speed_mask  = bit(2);
		static const byte_t sync_bit7               = bit(7);
		static const byte_t status_running          = bit(0);
		static const byte_t status_paused           = bit(1);
		static const byte_t status_supportreconnect = bit(2);
		byte_t command,
			b2,
			b3,
			b4;
		std::uint32_t timestamp;
	};
	struct queue_element{
		enum class Type{
			Stop,
			Connected,
			ConnectionAborted,
			ConnectionEstablished,
			Disconnected,
			IncomingPacket,
			OutgoingPacket,
		};
		Type type;
		packet data;
		DisconnectionCause cause;

		explicit queue_element(Type type): type(type){}
		explicit queue_element(const packet &data): type(Type::IncomingPacket), data(data){}
		explicit queue_element(DisconnectionCause cause): type(Type::Disconnected), cause(cause){}
	};
	enum class ConnectionState{
		Initial,
		Connecting,
		Ready,
		SentSync1,
		Sync2Queued,
		Finished,
	};

	ConnectionState state = ConnectionState::Initial;
	std::function<void()> on_connected;
	std::function<void(DisconnectionCause)> on_disconnected;
	std::function<void(transfer_data)> on_data_received;
	Event queue_event;
	std::deque<queue_element> event_queue;
	std::mutex event_queue_mutex;
	std::unique_ptr<std::thread> communication_thread;
	std::unique_ptr<std::thread> connection_thread;
	Event handshake_data_event;
	Event handshake_data_reply_event;
	packet handshake_data;
	packet queued_sync2;

	void push_element(const queue_element &qe);
	queue_element pop_element_waiting();
	void connected();
	void connection_aborted();
	void connection_established();
	void disconnected(const queue_element &);
	void received_packet(const queue_element &);
	void send_packet(const queue_element &);

	void socket_connected();
	void socket_disconnected(DisconnectionCause);
	size_t socket_received_data(const std::vector<byte_t> &);
	packet construct_version_packet();
	packet construct_status_packet();
	void set_handshake_packet(const packet &);
	bool wait_for_handshake_packet(packet &, int timeout = -1);
	void post_packet(const packet &);
	void communication_thread_function();
	void connection_thread_function();
	void process_communication(bool incoming, const packet &);
	void configure_connection_at_handshake(const packet &);
	void final_send_packet(packet);
	static transfer_data to_transfer_data(const packet &);
public:
	BgbNetworkProtocol(NetworkProviderConnection *connection);
	virtual ~BgbNetworkProtocol();
	void send_data(transfer_data) override;
	void set_on_connected(const std::function<void()> &) override;
	void set_on_disconnected(const std::function<void(DisconnectionCause)> &) override;
	void set_on_data_received(const std::function<void(transfer_data)> &) override;
};
