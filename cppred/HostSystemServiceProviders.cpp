#include "HostSystemServiceProviders.h"
#include "HostSystem.h"
#include <fstream>
#include <iostream>
#include <cassert>

StorageProvider::~StorageProvider(){
}

std::unique_ptr<std::vector<byte_t>> StorageProvider::load_file(const path_t &path, size_t maximum_size){
	std::unique_ptr<std::vector<byte_t>> ret;
	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return ret;

	auto string = casted_path->get_std_basic_string();
	//std::cout << "Requested file load: \"" << string << "\", maximum size: " << maximum_size << " bytes.\n";

	std::ifstream file(string.c_str(), std::ios::binary);
	if (!file)
		return ret;

	file.seekg(0, std::ios::end);
	if ((size_t)file.tellg() > maximum_size)
		return ret;
	ret.reset(new std::vector<byte_t>(file.tellg()));
	file.seekg(0);
	file.read((char *)&(*ret)[0], ret->size());
	return ret;
}

bool StorageProvider::save_file(const path_t &path, const void *buffer, size_t size){
	if (!buffer)
		return false;

	auto casted_path = std::dynamic_pointer_cast<StdBasicString<char>>(path);
	if (!casted_path)
		return false;

	auto string = casted_path->get_std_basic_string();
	//std::cout << "Requested file save: \"" << string << "\", size: " << size << " bytes.\n";

	std::ofstream file(string.c_str(), std::ios::binary);
	if (!file)
		return false;
	file.write((const char *)buffer, size);
	return true;
}

path_t StorageProvider::get_save_location(Cartridge &cart, SaveFileType type){
	auto ret = cart.get_path()->get_directory();
	auto name = cart.get_path()->get_filename()->remove_extension();
	const char *extension = nullptr;
	switch (type){
		case SaveFileType::Ram:
			extension = ".sav";
			break;
		case SaveFileType::Rtc:
			extension = ".rtc";
			break;
	}
	assert(extension);
	*name += extension;
	return ret->append_path_part(name);
}

void EventProvider::toggle_fastforward(bool on){
	this->host->toggle_fastforward(on);
}

void EventProvider::toggle_slowdown(bool on){
	this->host->toggle_slowdown(on);
}

void EventProvider::toggle_pause(int pause){
	this->host->toggle_pause(pause);
}

const char months_accumulated[] = { 0, 3, 3, 6, 8, 11, 13, 16, 19, 21, 24, 26 };

int days_from_date(const DateTime &date){
	int year = date.year;
	int month = date.month;
	int day = date.day;
	bool is_leap_year = !(year % 4) & !!(year % 100) | !(year % 400);
	year--;
	month--;
	int day_of_year = month * 28 + months_accumulated[month] + (is_leap_year & (month > 1)) + day;
	return year * 365 + year / 4 - year / 100 + year / 400 + day_of_year - 693594;
}

DateTime DateTime::from_posix(posix_time_t posix){
	static const char days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	DateTime ret;
	ret.year = 1970;
	while (1){
		bool leap = !(ret.year % 4) & !!(ret.year % 100) | !(ret.year % 400);
		unsigned seconds_in_year = (365 + leap) * 86400;
		if (posix < seconds_in_year)
			break;
		posix -= seconds_in_year;
		ret.year++;
	}
	ret.month = 0;
	while (1){
		bool leap = !(ret.year % 4) & !!(ret.year % 100) | !(ret.year % 400);
		bool leap_february = leap & (ret.month == 2);
		unsigned seconds_in_month = (days[ret.month] + leap_february) * 86400;
		if (posix < seconds_in_month)
			break;
		posix -= seconds_in_month;
		ret.month++;
	}
	ret.month++;
	ret.day = (std::uint8_t)(posix / 86400 + 1);
	posix %= 86400;
	ret.hour = (std::uint8_t)(posix / 3600);
	posix %= 3600;
	ret.minute = (std::uint8_t)(posix / 60);
	posix %= 60;
	ret.second = (std::uint8_t)posix;
	return ret;
}

posix_time_t DateTime::to_posix() const{
	bool is_leap_year = !(this->year % 4) & !!(this->year % 100) | !(this->year % 400);
	auto year = this->year - 1900;
	auto month = this->month - 1;
	unsigned days = 28 * month + months_accumulated[month] + (is_leap_year & (month > 1)) + this->day - 1;

	days += (year + 299) / 400;
	days -= (year - 1) / 100;
	days += (year - 69) / 4;
	days += (year - 70) * 365;

	posix_time_t ret = days;
	ret *= 86400;
	ret += this->hour * 3600 + this->minute * 60 + this->second;

	return ret;
}

DateTime DateTimeProvider::double_timestamp_to_date(double t){
	return DateTime::from_posix(DateTimeProvider::double_timestamp_to_posix(t));
}

posix_time_t DateTimeProvider::double_timestamp_to_posix(double t){
	return (posix_time_t)((t - 25569) * 86400);
}

double DateTimeProvider::date_to_double_timestamp(DateTime date){
	return days_from_date(date) + date.hour * (1.0 / 24.0) + date.minute * (1.0 / (24 * 60)) + date.second * (1.0 / (24 * 60 * 60));
}

DateTime StdDateTimeProvider::local_now(){
	auto timestamp = ::time(nullptr);
	tm t = *localtime(&timestamp);
	DateTime ret;
	ret.year = t.tm_year + 1900;
	ret.month = t.tm_mon + 1;
	ret.day = t.tm_mday;
	ret.hour = t.tm_hour;
	ret.minute = t.tm_min;
	ret.second = t.tm_sec;
	return ret;
}

std::uint32_t NetworkProvider::little_endian_to_native_endian(std::uint32_t n){
#ifndef BIG_ENDIAN
	return n;
#else
	std::uint32_t ret;
	ret = n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	n >>= 8;
	ret <<= 8;
	ret |= n & 0xFF;
	return ret;
#endif
}

std::uint32_t NetworkProvider::native_endian_to_little_endian(std::uint32_t n){
	return NetworkProvider::little_endian_to_native_endian(n);
}

void AudioOutputProvider::set_callbacks(get_data_callback_t gdc, return_data_callback_t rdc){
	std::lock_guard<std::mutex> lg(this->mutex);
	this->get_data_callback = gdc;
	this->return_data_callback = rdc;
}
