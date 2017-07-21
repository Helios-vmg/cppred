#pragma once

#include "CommonTypes.h"
#include "GeneralString.h"
#include "threads.h"
#include <memory>
#include <vector>
#include <atomic>

class Cartridge;
struct RenderedFrame;
struct InputState;
class HostSystem;
struct AudioFrame;

enum class SaveFileType{
	Ram,
	Rtc,
};

class StorageProvider{
public:
	virtual ~StorageProvider() = 0;
	virtual std::unique_ptr<std::vector<byte_t>> load_file(const path_t &path, size_t maximum_size);;
	bool save_file(const path_t &path, const std::vector<byte_t> &buffer){
		return this->save_file(path, &buffer[0], buffer.size());
	}
	virtual bool save_file(const path_t &path, const void *, size_t);
	virtual path_t get_save_location(Cartridge &, SaveFileType);
};

class StdStorageProvider : public StorageProvider{
public:
};

struct DateTime{
	std::uint16_t year;
	std::uint8_t month;
	std::uint8_t day;
	std::uint8_t hour;
	std::uint8_t minute;
	std::uint8_t second;

	static DateTime from_posix(posix_time_t);
	posix_time_t to_posix() const;
};

class DateTimeProvider{
public:
	virtual DateTime local_now() = 0;
	double get_double_timestamp(){
		return this->date_to_double_timestamp(this->local_now());
	}
	static DateTime double_timestamp_to_date(double);
	static posix_time_t double_timestamp_to_posix(double);
	static double date_to_double_timestamp(DateTime);
};

class StdDateTimeProvider : public DateTimeProvider{
public:
	DateTime local_now() override;
};

class TimingProvider{
protected:
	Event *periodic_event = nullptr;
public:
	virtual ~TimingProvider(){}
	virtual void register_periodic_notification(Event &) = 0;
	virtual void unregister_periodic_notification() = 0;
};

class EventProvider{
	HostSystem *host = nullptr;
public:
	virtual ~EventProvider(){}
	struct HandleEventsResult{
		InputState *input_state = nullptr;
		bool button_down = false;
		bool button_up = false;
	};
	virtual bool handle_events(HandleEventsResult &) = 0;
	void set_host(HostSystem &host){
		this->host = &host;
	}
	void toggle_fastforward(bool);
	void toggle_slowdown(bool);
	void toggle_pause(int);
};

class GraphicsOutputProvider{
public:
	virtual ~GraphicsOutputProvider(){}
	virtual void render(const RenderedFrame *) = 0;
	virtual void write_frame_to_disk(std::string &path, const RenderedFrame &){}
};

class AudioOutputProvider{
public:
	typedef std::function<AudioFrame *()> get_data_callback_t;
	typedef std::function<void(AudioFrame *)> return_data_callback_t;
protected:
	get_data_callback_t get_data_callback;
	return_data_callback_t return_data_callback;
	std::mutex mutex;
public:
	virtual ~AudioOutputProvider(){}
	void set_callbacks(get_data_callback_t gdc, return_data_callback_t rdc);
	virtual void stop_audio() = 0;
};

enum class DisconnectionCause{
	ConnectionAborted,
	LocalUserInitiated,
	RemoteUserInitiated,
	ConnectionDropped,
};

class NetworkProviderConnection;

class NetworkProvider{
protected:
	std::vector<std::unique_ptr<NetworkProviderConnection>> connections;
public:
	virtual ~NetworkProvider(){}
	virtual NetworkProviderConnection *create_connection() = 0;
	static std::uint32_t little_endian_to_native_endian(std::uint32_t);
	static std::uint32_t native_endian_to_little_endian(std::uint32_t);
};

class NetworkProviderConnection{
	NetworkProvider *provider;
	std::function<void()> on_accept;
	std::function<void(DisconnectionCause)> on_disconnection;
	std::function<size_t(const std::vector<byte_t> &)> on_data_receive;
public:
	NetworkProviderConnection(NetworkProvider &provider): provider(&provider){}
	virtual ~NetworkProviderConnection(){}
	virtual bool open() = 0;
	virtual void abort() = 0;
	virtual void send_data(const std::vector<byte_t> &) = 0;
	virtual void send_data(const void *, size_t) = 0;

#define DEFINE_SETTER(x) void set_##x(const decltype(x) &y){ this->x = y; }
	DEFINE_SETTER(on_accept);
	DEFINE_SETTER(on_disconnection);
	DEFINE_SETTER(on_data_receive);
};

class NetworkProtocol{
protected:
	NetworkProviderConnection *connection;
public:
	struct transfer_data{
		bool passive_mode;
		//When operating in DMG mode, this is always false.
		bool fast_mode;
		//When operating in DMG mode, this is always false.
		bool double_speed_mode;
		byte_t data;
	};

	NetworkProtocol(NetworkProviderConnection *connection): connection(connection){}
	virtual ~NetworkProtocol(){}
	virtual int get_default_port() const{
		return -1;
	}
	virtual void send_data(transfer_data) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_connected(const std::function<void()> &) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_disconnected(const std::function<void(DisconnectionCause)> &) = 0;
	//Warning: This callback may be called from a different thread!
	virtual void set_on_data_received(const std::function<void(transfer_data)> &) = 0;
};
