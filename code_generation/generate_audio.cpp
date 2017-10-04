#include "code_generators.h"
#include "../FreeImage/Source/ZLib/zlib.h"
#include <iostream>
#include <memory>
#include <set>

typedef std::uint32_t u32;

static const std::vector<std::string> input_files = {
	"input/audio.txt",
	"input/custom_audio_headers.txt",
};
static const char * const hash_key = "generate_audio";
static const char * const date_string = __DATE__ __TIME__;
static const u32 invalid_u32 = std::numeric_limits<u32>::max();

class AudioCommand{
protected:
	u32 params[4];
	std::string referenced_sequence;
	int parameter_count;
public:
	AudioCommand(std::istream &stream, int n){
		this->parameter_count = n;
		int m = n;
		for (int i = 0; i < n; i++){
			std::string temp;
			stream >> temp;
			auto converted = to_unsigned_default(temp, invalid_u32);
			if (converted == invalid_u32){
				if (i == n - 1)
					this->referenced_sequence = temp;
				else
					throw std::runtime_error("Syntax error.");
				m--;
			}else
				this->params[i] = (u32)converted;
		}
		std::fill(this->params + m, this->params + 4, invalid_u32);
	}
	virtual ~AudioCommand(){}
	const std::string &get_referenced_sequence() const{
		return this->referenced_sequence;
	}
	virtual u32 command_id() const = 0;
	void set_dst(u32 dst){
		this->params[this->parameter_count - 1] = dst;
	}
	void serialize(std::vector<std::uint8_t> &sequences){
		write_varint(sequences, this->command_id());
		for (int i = 0; i < this->parameter_count; i++)
			write_varint(sequences, this->params[i]);
	}
};

class AudioSequence{
	std::string name;
	std::vector<std::unique_ptr<AudioCommand>> commands;
	u32 location = invalid_u32;
public:
	AudioSequence(const std::string &name): name(name){}
	const std::string &get_name() const{
		return this->name;
	}
	void add_command(std::unique_ptr<AudioCommand> &&p){
		this->commands.emplace_back(std::move(p));
	}
	u32 command_count() const{
		if (this->commands.size() >= invalid_u32)
			throw std::runtime_error("Numerical limits exceeded.");
		return (u32)this->commands.size();
	}
	void set_location(u32 &n){
		this->location = n;
		n += this->commands.size();
	}
	u32 get_location() const{
		return this->location;
	}
	std::vector<std::string> get_referenced_sequences() const{
		std::vector<std::string> ret;
		for (auto &command : this->commands){
			auto &seq = command->get_referenced_sequence();
			if (!seq.size())
				continue;
			ret.push_back(seq);
		}
		return ret;
	}
	std::vector<std::unique_ptr<AudioCommand>> &get_commands(){
		return this->commands;
	}
	void serialize(std::vector<std::uint8_t> &sequences){
		//write_varint(sequences, (u32)this->commands.size());
		for (auto &c : this->commands)
			c->serialize(sequences);
	}
};

#define DEFINE_SEQUENCE_CLASS(name, parameter_count, id)                            \
	class name##Command : public AudioCommand{                                      \
	public:                                                                         \
		name##Command(std::istream &stream): AudioCommand(stream, parameter_count){ \
		}                                                                           \
		u32 command_id() const override{ return id; }                               \
	}

DEFINE_SEQUENCE_CLASS(Tempo, 1, 0);
DEFINE_SEQUENCE_CLASS(Volume, 2, 1);
DEFINE_SEQUENCE_CLASS(Duty, 1, 2);
DEFINE_SEQUENCE_CLASS(DutyCycle, 1, 3);
DEFINE_SEQUENCE_CLASS(Vibrato, 3, 4);
DEFINE_SEQUENCE_CLASS(TogglePerfectPitch, 0, 5);
DEFINE_SEQUENCE_CLASS(NoteType, 3, 6);
DEFINE_SEQUENCE_CLASS(Rest, 1, 7);
DEFINE_SEQUENCE_CLASS(Octave, 1, 8);
DEFINE_SEQUENCE_CLASS(Note, 2, 9);
DEFINE_SEQUENCE_CLASS(DSpeed, 1, 10);
DEFINE_SEQUENCE_CLASS(Snare, 2, 11);
DEFINE_SEQUENCE_CLASS(MutedSnare, 2, 12);
DEFINE_SEQUENCE_CLASS(UnknownSfx10, 1, 13);
DEFINE_SEQUENCE_CLASS(UnknownSfx20, 4, 14);
DEFINE_SEQUENCE_CLASS(UnknownNoise20, 3, 15);
DEFINE_SEQUENCE_CLASS(ExecuteMusic, 0, 16);
DEFINE_SEQUENCE_CLASS(PitchBend, 2, 17);
DEFINE_SEQUENCE_CLASS(Triangle, 2, 18);
DEFINE_SEQUENCE_CLASS(StereoPanning, 1, 19);
DEFINE_SEQUENCE_CLASS(Cymbal, 2, 20);
DEFINE_SEQUENCE_CLASS(Loop, 2, 21);
DEFINE_SEQUENCE_CLASS(Call, 1, 22);
DEFINE_SEQUENCE_CLASS(Goto, 1, 23);
DEFINE_SEQUENCE_CLASS(IfRed, 0, 24);
DEFINE_SEQUENCE_CLASS(Else, 0, 25);
DEFINE_SEQUENCE_CLASS(EndIf, 0, 26);
DEFINE_SEQUENCE_CLASS(End, 0, 27);

class AudioChannel{
	u32 channel_no;
	std::string referenced_sequence;
	u32 dst = invalid_u32;
public:
	AudioChannel(u32 channel_no, const std::string &referenced_sequence):
		channel_no(channel_no),
		referenced_sequence(referenced_sequence){}
	AudioChannel(AudioChannel &&other):
		channel_no(other.channel_no),
		referenced_sequence(std::move(other.referenced_sequence)),
		dst(other.dst){}
	AudioChannel(const AudioChannel &other):
		channel_no(other.channel_no),
		referenced_sequence(other.referenced_sequence),
		dst(other.dst){}
	const AudioChannel &operator=(const AudioChannel &other){
		this->channel_no = other.channel_no;
		this->referenced_sequence = other.referenced_sequence;
		this->dst = other.dst;
		return *this;
	}
	u32 get_dst() const{
		return this->dst;
	}
	void set_dst(u32 n){
		this->dst = n;
	}
	const std::string &get_referenced_sequence() const{
		return this->referenced_sequence;
	}
	void serialize(std::vector<std::uint8_t> &headers) const{
		write_varint(headers, this->dst);
		write_varint(headers, this->channel_no);
	}
};

class AudioHeader{
	std::string name;
	u32 bank;
	std::vector<AudioChannel> channels;
public:
	AudioHeader(const std::string &name, u32 bank):
		name(name),
		bank(bank){}
	AudioHeader(AudioHeader &&other):
		name(std::move(other.name)),
		bank(other.bank),
		channels(std::move(other.channels)){}
	const AudioHeader &operator=(const AudioHeader &other){
		this->name = other.name;
		this->bank = other.bank;
		this->channels = other.channels;
		return *this;
	}
	void add_channel(AudioChannel &&channel){
		this->channels.emplace_back(std::move(channel));
	}
	const std::vector<AudioChannel> &get_channels() const{
		return this->channels;
	}
	std::vector<AudioChannel> &get_channels(){
		return this->channels;
	}
	void serialize(std::vector<std::uint8_t> &headers) const{
		write_ascii_string(headers, this->name);
		write_varint(headers, this->bank);
		write_varint(headers, this->channels.size());
		for (auto &c : this->channels)
			c.serialize(headers);
	}
	const std::string &get_name() const{
		return this->name;
	}
};

#define DECLARE_CONSTRUCTOR(classname, string) constructors[#string] = [](std::istream &stream){ return std::make_unique<classname##Command>(stream); }

static std::map<std::string, std::unique_ptr<AudioSequence>> parse_sequences(const std::vector<std::string> &input){
	std::map<std::string, std::function<std::unique_ptr<AudioCommand>(std::istream &)>> constructors;
	DECLARE_CONSTRUCTOR(Tempo, tempo);
	DECLARE_CONSTRUCTOR(Volume, volume);
	DECLARE_CONSTRUCTOR(Duty, duty);
	DECLARE_CONSTRUCTOR(DutyCycle, dutycycle);
	DECLARE_CONSTRUCTOR(Vibrato, vibrato);
	DECLARE_CONSTRUCTOR(TogglePerfectPitch, toggle_perfect_pitch);
	DECLARE_CONSTRUCTOR(NoteType, note_type);
	DECLARE_CONSTRUCTOR(Rest, rest);
	DECLARE_CONSTRUCTOR(Octave, octave);
	DECLARE_CONSTRUCTOR(Note, note);
	DECLARE_CONSTRUCTOR(DSpeed, dspeed);
	DECLARE_CONSTRUCTOR(Snare, snare);
	DECLARE_CONSTRUCTOR(MutedSnare, muted_snare);
	DECLARE_CONSTRUCTOR(UnknownSfx10, unknown_sfx_10);
	DECLARE_CONSTRUCTOR(UnknownSfx20, unknown_sfx_20);
	DECLARE_CONSTRUCTOR(UnknownNoise20, unknown_noise_20);
	DECLARE_CONSTRUCTOR(ExecuteMusic, execute_music);
	DECLARE_CONSTRUCTOR(PitchBend, pitch_bend);
	DECLARE_CONSTRUCTOR(Triangle, triangle);
	DECLARE_CONSTRUCTOR(StereoPanning, stereo_panning);
	DECLARE_CONSTRUCTOR(Cymbal, cymbal);
	DECLARE_CONSTRUCTOR(Loop, loop);
	DECLARE_CONSTRUCTOR(Call, call);
	DECLARE_CONSTRUCTOR(Goto, goto);
	DECLARE_CONSTRUCTOR(IfRed, ifred);
	DECLARE_CONSTRUCTOR(Else, else);
	DECLARE_CONSTRUCTOR(EndIf, endif);
	DECLARE_CONSTRUCTOR(End, end);

	std::map<std::string, std::unique_ptr<AudioSequence>> ret;

	AudioSequence *current_sequence = nullptr;
	for (auto &s : input){
		if (s[0] == '.'){
			auto sequence = std::make_unique<AudioSequence>(s.substr(1));
			current_sequence = sequence.get();
			ret[current_sequence->get_name()] = std::move(sequence);
			continue;
		}
		std::stringstream stream(s);
		std::string command_name;
		stream >> command_name;
		auto it = constructors.find(command_name);
		if (it == constructors.end())
			throw std::runtime_error("Syntax error: " + command_name);
		current_sequence->add_command(it->second(stream));
	}
	return ret;
}

static std::vector<AudioHeader> parse_headers(const std::vector<std::string> &input){
	std::vector<AudioHeader> ret;

	AudioHeader *current_header = nullptr;
	for (auto &s : input){
		if (s[0] == '.'){
			std::stringstream stream(s.substr(1));
			std::string name;
			u32 bank;
			stream >> name >> bank;
			ret.emplace_back(name, bank);
			current_header = &ret.back();
			continue;
		}
		std::stringstream stream(s);
		std::string command_name, sequence;
		u32 channel;
		stream >> command_name >> sequence >> channel;
		if (command_name != "channel")
			throw std::runtime_error("Syntax error: Header must contain a list of channels, but contains a '" + command_name + "'.");
		current_header->add_channel(AudioChannel(channel, sequence));
	}
	return ret;
}

class AudioData{
	std::map<std::string, std::unique_ptr<AudioSequence>> sequences;
	std::vector<AudioHeader> headers;

	void load_main_file(const std::string &path){
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error("File not found: " + path);

		std::vector<std::string> sequence_strings;
		std::vector<std::string> header_strings;
		bool reading_sequences = true;
		while (true){
			std::string line;
			std::getline(file, line);
			if (!file)
				break;
			if (!line.size())
				continue;
			if (line == ":headers"){
				if (this->headers.size())
					break;
				reading_sequences = !reading_sequences;
				continue;
			}
			(reading_sequences ? sequence_strings : header_strings).push_back(line);
		}
		this->sequences = parse_sequences(sequence_strings);
		if (!this->headers.size())
			this->headers = parse_headers(header_strings);
	}
	void load_secondary_file(const std::string &path){
		std::ifstream file(path);
		if (!file)
			throw std::runtime_error("File not found: " + path);
		std::vector<std::string> strings;
		while (true){
			std::string line;
			std::getline(file, line);
			if (!file)
				break;
			if (!line.size())
				continue;
			strings.push_back(line);
		}
		if (strings.size())
			this->headers = parse_headers(strings);
	}
	void remove_unreachable_sequences(std::ostream &log_file){
		std::vector<std::string> stack;
		std::set<std::string> used;

		for (auto &header : this->headers)
			for (auto &channel : header.get_channels())
				stack.push_back(channel.get_referenced_sequence());

		while (stack.size()){
			auto head = stack.back();
			stack.pop_back();
			if (used.find(head) != used.end())
				continue;
			used.insert(head);
			auto sequence = this->sequences[head].get();
			auto refs = sequence->get_referenced_sequences();
			for (auto &ref : refs)
				stack.push_back(ref);
		}

		std::vector<std::string> to_remove;

		for (auto &s : this->sequences)
			if (used.find(s.first) == used.end())
				to_remove.push_back(s.first);

		for (auto &s : to_remove){
			log_file << "Removing unreachable sequence " << s << std::endl;
			this->sequences.erase(s);
		}
	}
	void place_sequences(){
		u32 location = 0;
		for (auto &s : this->sequences)
			s.second->set_location(location);
		for (auto &s : this->sequences){
			for (auto &c : s.second->get_commands()){
				auto &ref = c->get_referenced_sequence();
				if (!ref.size())
					continue;
				auto it = this->sequences.find(ref);
				assert(it != this->sequences.end() && it->second->get_location() != invalid_u32);
				c->set_dst(it->second->get_location());
			}
		}
		for (auto &h : this->headers){
			for (auto &c : h.get_channels()){
				auto &ref = c.get_referenced_sequence();
				if (!ref.size())
					continue;
				auto it = this->sequences.find(ref);
				assert(it != this->sequences.end() && it->second->get_location() != invalid_u32);
				c.set_dst(it->second->get_location());
			}
		}
	}
public:
	AudioData(std::ostream &log_file){
		this->load_secondary_file(input_files[1]);
		this->load_main_file(input_files[0]);
		this->remove_unreachable_sequences(log_file);
		this->place_sequences();
	}
	void serialize_sequences(std::vector<std::uint8_t> &sequences) const{
		write_varint(sequences, (u32)this->sequences.size());
		for (auto &s : this->sequences)
			s.second->serialize(sequences);
	}
	void serialize_headers(std::vector<std::uint8_t> &headers) const{
		write_varint(headers, (u32)this->headers.size());
		for (auto &s : this->headers)
			s.serialize(headers);
	}

	const std::vector<AudioHeader> &get_headers() const{
		return this->headers;
	}
};

static void write_header(const char *path, const std::vector<AudioHeader> &headers){
	std::ofstream header(path);
	header << "#pragma once\n"
		<< generated_file_warning <<
		"\n"
		"extern const byte_t audio_sequence_data[];\n"
		"extern const size_t audio_sequence_data_size;\n"
		"extern const byte_t audio_header_data[];\n"
		"extern const size_t audio_header_data_size;\n"
		"enum class AudioResourceId{\n"
		"    None = 0,\n";
	size_t i = 1;
	for (auto &h : headers)
		header << "    " << h.get_name() << " = " << i++ << ",\n";
	header << "    Stop = " << i++ << ",\n"
		"};\n";
}

static void write_source(const char *path, const AudioData &data){
	std::vector<std::uint8_t> sequences, headers;
	data.serialize_sequences(sequences);
	data.serialize_headers(headers);

	std::ofstream source(path);
	source << generated_file_warning <<
		"\n"
		"const byte_t audio_sequence_data[] = ";
	write_buffer_to_stream(source, sequences);
	source << std::dec << ";\n"
		"const size_t audio_sequence_data_size = " << sequences.size() << ";\n"
		"const byte_t audio_header_data[] = ";
	write_buffer_to_stream(source, headers);
	source << std::dec << ";\n"
		"const size_t audio_header_data_size = " << headers.size() << ";\n";
}

static void generate_audio_internal(known_hashes_t &known_hashes){
	auto current_hash = hash_files(input_files, date_string);
	if (check_for_known_hash(known_hashes, hash_key, current_hash)){
		std::cout << "Skipping generating audio.\n";
		return;
	}
	std::cout << "Generating audio...\n";

	std::ofstream log_file("output/audio_generation.log");
	AudioData data(log_file);
	
	write_header("output/audio.h", data.get_headers());
	write_source("output/audio.inl", data);

	known_hashes[hash_key] = current_hash;
}

void generate_audio(known_hashes_t &known_hashes){
	try{
		generate_audio_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_audio(): " + e.what());
	}
}
