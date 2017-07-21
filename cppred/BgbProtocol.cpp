#include "BgbProtocol.h"
#include <cassert>
#include <iostream>
#include <cstring>

BgbNetworkProtocol::BgbNetworkProtocol(NetworkProviderConnection *connection): NetworkProtocol(connection){
	auto This = this;
	this->communication_thread.reset(new std::thread([This](){ This->communication_thread_function(); }));
	this->connection->set_on_accept([This](){ This->socket_connected(); });
	this->connection->set_on_disconnection([This](DisconnectionCause cause){ This->socket_disconnected(cause); });
	this->connection->set_on_data_receive([This](const std::vector<byte_t> &data){ return This->socket_received_data(data); });
}

BgbNetworkProtocol::~BgbNetworkProtocol(){
	join_thread(this->connection_thread);
	join_thread(this->communication_thread);
}

//void BgbNetworkProtocol::initiate_as_master(){
//	this->provider->start_server();
//}

//void BgbNetworkProtocol::initiate_as_slave(){}

//void BgbNetworkProtocol::send_data(transfer_data){
//	
//}

void BgbNetworkProtocol::set_on_connected(const std::function<void()> &f){
	this->on_connected = f;
}

void BgbNetworkProtocol::set_on_disconnected(const std::function<void(DisconnectionCause)> &f){
	this->on_disconnected = f;
}

void BgbNetworkProtocol::set_on_data_received(const std::function<void(transfer_data)> &f){
	this->on_data_received = f;
}

BgbNetworkProtocol::packet BgbNetworkProtocol::construct_version_packet(){
	packet ret;
	memset(&ret, 0, sizeof(ret));
	ret.command = packet::command_version;
	ret.b2 = packet::current_version_major;
	ret.b3 = packet::current_version_minor;
	return ret;
}

BgbNetworkProtocol::packet BgbNetworkProtocol::construct_status_packet(){
	packet ret;
	memset(&ret, 0, sizeof(ret));
	ret.command = packet::command_status;
	ret.b2 = packet::status_running;
	return ret;
}

void BgbNetworkProtocol::push_element(const queue_element &qe){
	std::lock_guard<std::mutex> lg(this->event_queue_mutex);
	this->event_queue.push_back(qe);
}

BgbNetworkProtocol::queue_element BgbNetworkProtocol::pop_element_waiting(){
	{
		std::lock_guard<std::mutex> lg(this->event_queue_mutex);
		if (this->event_queue.size()){
			auto ret = this->event_queue.back();
			this->event_queue.pop_back();
			return ret;
		}
	}

	this->queue_event.wait();

	std::lock_guard<std::mutex> lg(this->event_queue_mutex);
	auto ret = this->event_queue.back();
	this->event_queue.pop_back();
	return ret;
}

void BgbNetworkProtocol::socket_connected(){
	this->push_element(queue_element(queue_element::Type::Connected));
}

void BgbNetworkProtocol::socket_disconnected(DisconnectionCause cause){
	this->push_element(queue_element(cause));
}

void BgbNetworkProtocol::connection_thread_function(){
	do{
		{
			auto version = this->construct_version_packet();
			this->push_element(queue_element(version));
			packet other_version;
			if (!this->wait_for_handshake_packet(other_version, 1000))
				break;
			if (memcmp(&version, &other_version, sizeof(version)))
				break;
		}
		{
			auto status = this->construct_status_packet();
			this->push_element(queue_element(status));
			packet other_status;
			if (!this->wait_for_handshake_packet(other_status, 1000))
				break;
			this->configure_connection_at_handshake(other_status);
		}
		this->push_element(queue_element(queue_element::Type::ConnectionEstablished));
		return;
	}while (false);

	this->push_element(queue_element(queue_element::Type::ConnectionAborted));
}

size_t BgbNetworkProtocol::socket_received_data(const std::vector<byte_t> &data){
	if (data.size() < sizeof(packet))
		return 0;

	auto packets = (const packet *)&data[0];
	size_t packet_count = data.size() / sizeof(packet);
	size_t ret = packet_count * sizeof(packet);
	for (size_t i = 0; i < packet_count; i++){
		auto packet = packets[i];
		packet.timestamp = NetworkProvider::little_endian_to_native_endian(packet.timestamp);
		this->post_packet(packet);
	}
	return ret;
}

void BgbNetworkProtocol::communication_thread_function(){
	this->state = ConnectionState::Initial;
	while (this->state != ConnectionState::Finished){
		auto qe = this->pop_element_waiting();
		switch (qe.type){
			case queue_element::Type::Stop:
				this->state = ConnectionState::Finished;
				break;
			case queue_element::Type::Connected:
				this->connected();
				break;
			case queue_element::Type::ConnectionAborted:
				this->connection_aborted();
				break;
			case queue_element::Type::ConnectionEstablished:
				this->connection_established();
				break;
			case queue_element::Type::Disconnected:
				this->disconnected(qe);
				break;
			case queue_element::Type::IncomingPacket:
				this->received_packet(qe);
				break;
			case queue_element::Type::OutgoingPacket:
				this->send_packet(qe);
				break;
			default:
				abort();
		}
	}
}

void BgbNetworkProtocol::connected(){
	this->state = ConnectionState::Connecting;
	auto This = this;
	this->connection_thread.reset(new std::thread([This](){ This->connection_thread_function(); }));
}

void BgbNetworkProtocol::connection_aborted(){
	if (this->state != ConnectionState::Connecting)
		abort();
	this->connection->abort();
	this->state = ConnectionState::Finished;
}

void BgbNetworkProtocol::connection_established(){
	if (this->state != ConnectionState::Connecting)
		abort();
	join_thread(this->connection_thread);
	this->state = ConnectionState::Ready;
}

void BgbNetworkProtocol::disconnected(const queue_element &qe){
	if (this->state != ConnectionState::Connecting)
		this->on_disconnected(qe.cause);
}

void BgbNetworkProtocol::received_packet(const queue_element &qe){
	if (this->state == ConnectionState::Connecting){
		this->set_handshake_packet(qe.data);
		return;
	}

	this->process_communication(true, qe.data);
}

void BgbNetworkProtocol::send_packet(const queue_element &qe){
	if (this->state == ConnectionState::Connecting){
		this->final_send_packet(qe.data);
		return;
	}

	this->process_communication(false, qe.data);
}

void BgbNetworkProtocol::final_send_packet(packet p){
	p.timestamp = NetworkProvider::native_endian_to_little_endian(p.timestamp);
	this->connection->send_data(&p, sizeof(p));
}

NetworkProtocol::transfer_data BgbNetworkProtocol::to_transfer_data(const packet &p){
	transfer_data ret;
	ret.data = p.b2;
	ret.fast_mode = !!(p.b3 & packet::sync_high_speed_mask);
	ret.double_speed_mode = !!(p.b3 & packet::sync_double_speed_mask);
	ret.passive_mode = !!(p.b3 & packet::sync_active_mode);
	return ret;
}

void BgbNetworkProtocol::process_communication(bool incoming, const packet &p){
	if (this->state == ConnectionState::Connecting || this->state == ConnectionState::Finished || this->state == ConnectionState::Initial)
		abort();
	transfer_data data;
	if (this->state == ConnectionState::Ready){
		if (!incoming){
			switch (p.command){
				case packet::command_sync3:
				case packet::command_status:
				case packet::command_wantdisconnect:
				case packet::command_version:
				case packet::command_joypad:
					//No state change.
					goto process_communication_send;
				case packet::command_sync1:
					this->state = ConnectionState::SentSync1;
					goto process_communication_send;
				case packet::command_sync2:
					this->state = ConnectionState::Sync2Queued;
					this->queued_sync2 = p;
					//Do not send packet at this time.
					return;
			}
			assert(false);
		}

		switch (p.command){
			case packet::command_version:
			case packet::command_joypad:
				//Do nothing.
				return;
			case packet::command_sync3:
			case packet::command_status:
			case packet::command_wantdisconnect:
				//No state change.
				return;
			case packet::command_sync2:
				std::cerr << "BgbNetworkProtocol::process_communication(): "
					"Warning. Peer is violating network protocol by sending "
					"a SYNC2 reply without a corresponding SYNC1 message from "
					"us. The packet will be ignored.\n";
				return;
			case packet::command_sync1:
				//Queue up a sync3 b2=1 reply.
				{
					packet reply;
					reply.command = packet::command_sync3;
					reply.b2 = 1;
					reply.b3 = 0;
					reply.b4 = 0;
					reply.timestamp = 0;
					this->push_element(queue_element(reply));
				}
				data = to_transfer_data(p);
				this->state = ConnectionState::Ready;
				goto process_communication_notify;
		}
		assert(false);
		return;
	}

	switch (this->state){
		case ConnectionState::SentSync1:
			if (!incoming){
				std::cerr << "BgbNetworkProtocol::process_communication(): "
					"Internal error. We violated network protocol!\n"
					"Aborting.\n";
				abort();
			}

			if (p.command == packet::command_sync2){
				data = to_transfer_data(p);
				this->state = ConnectionState::Ready;
				goto process_communication_notify;
			}
			if (p.command == packet::command_sync3 && p.b2 == 1){
				this->state = ConnectionState::Ready;
				return;
			}
			std::cerr << "BgbNetworkProtocol::process_communication(): "
				"Warning. Peer is violating network protocol by sending "
				"something other than a SYNC2 or a SYNC3-0 reply after we "
				"sent a SYNC1 message. The packet will be ignored and the "
				"incoming byte from the peer may be lost, if there was any.\n";
			this->state = ConnectionState::Ready;
			return;
		case ConnectionState::Sync2Queued:
			if (!incoming){
				this->state = ConnectionState::Ready;
				this->process_communication(incoming, p);
				return;
			}

			if (p.command == packet::command_sync1){
				this->final_send_packet(this->queued_sync2);
				data = to_transfer_data(p);
				this->state = ConnectionState::Ready;
				goto process_communication_notify;
			}

			this->state = ConnectionState::Ready;
			this->process_communication(incoming, p);
			assert(this->state == ConnectionState::Ready);
			this->state = ConnectionState::Sync2Queued;
			return;
		default:
			break;
	}
	assert(false);

process_communication_send:
	this->final_send_packet(p);
	return;
process_communication_notify:
	this->on_data_received(data);
	return;
}

void BgbNetworkProtocol::set_handshake_packet(const packet &p){
	this->handshake_data = p;
	this->handshake_data_event.signal();
	this->handshake_data_reply_event.wait();
}

bool BgbNetworkProtocol::wait_for_handshake_packet(packet &dst, int timeout){
	if (timeout < 0)
		this->handshake_data_event.wait();
	else if (!this->handshake_data_event.wait_for(timeout))
		return false;

	dst = this->handshake_data;
	this->handshake_data_reply_event.signal();
	return true;
}

void BgbNetworkProtocol::post_packet(const packet &p){
	this->push_element(queue_element(p));
}

void BgbNetworkProtocol::configure_connection_at_handshake(const packet &){
	//TODO
}

void BgbNetworkProtocol::send_data(transfer_data data){
	packet p;
	if (!data.passive_mode){
		p.command = packet::command_sync1;
		p.b2 = data.data;
		p.b3 = packet::sync_active_mode |
			(data.fast_mode * packet::sync_high_speed_mask) |
			(data.double_speed_mode * packet::sync_double_speed_mask) |
			packet::sync_bit7;
		p.b4 = 0;
		p.timestamp = 0; //TODO
	}else{
		p.command = packet::command_sync2;
		p.b2 = data.data;
		p.b3 = packet::sync_bit7;
		p.b4 = 0;
		p.timestamp = 0;
	}
	this->push_element(queue_element(p));
}
