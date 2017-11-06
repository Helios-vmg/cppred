#include "generate_audio.h"
#include "../FreeImage/Source/ZLib/zlib.h"
#include "../common/calculate_frequency.h"
#include "../common/AudioCommandType.h"
#include "../common/AudioResourceType.h"
#include <iostream>
#include <memory>
#include <set>
#include <limits>

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
	const std::vector<std::unique_ptr<AudioCommand>> &get_commands() const{
		return this->commands;
	}
	void serialize(std::vector<std::uint8_t> &sequences){
		//write_varint(sequences, (u32)this->commands.size());
		for (auto &c : this->commands)
			c->serialize(sequences);
	}
};

#define DEFINE_SEQUENCE_CLASS(name, parameter_count)                                \
	class name##Command : public AudioCommand{                                      \
	public:                                                                         \
		name##Command(std::istream &stream): AudioCommand(stream, parameter_count){}\
		u32 command_id() const override{ return (u32)AudioCommandType::name; }      \
	}

DEFINE_SEQUENCE_CLASS(Tempo, 1);
DEFINE_SEQUENCE_CLASS(Volume, 2);
//DEFINE_SEQUENCE_CLASS(Duty, 1);
class DutyCommand : public AudioCommand{
public:
	DutyCommand(std::istream &stream): AudioCommand(stream, 1){
		this->params[0] <<= 6;
		this->params[0] &= 0xC0;
	}
	u32 command_id() const override{ return (u32)AudioCommandType::Duty; }
};
DEFINE_SEQUENCE_CLASS(DutyCycle, 1);
DEFINE_SEQUENCE_CLASS(Vibrato, 3);
DEFINE_SEQUENCE_CLASS(TogglePerfectPitch, 0);
DEFINE_SEQUENCE_CLASS(NoteType, 3);
DEFINE_SEQUENCE_CLASS(Rest, 1);
//DEFINE_SEQUENCE_CLASS(Octave, 1);
class OctaveCommand : public AudioCommand{
public:
	OctaveCommand(std::istream &stream): AudioCommand(stream, 1){
		if (this->params[0] < 1 || this->params[0] > 8){
			std::stringstream stream;
			stream << "Invalid octave parameter: " << this->params[0];
			throw std::runtime_error(stream.str());
		}
		this->params[0] = 8 - this->params[0];
	}
	u32 command_id() const override{ return (u32)AudioCommandType::Octave; }
};
DEFINE_SEQUENCE_CLASS(Note, 2);
DEFINE_SEQUENCE_CLASS(DSpeed, 1);
//DEFINE_SEQUENCE_CLASS(NoiseInstrument, 2);
class SnareCommand : public AudioCommand{
public:
	SnareCommand(std::istream &stream): AudioCommand(stream, 2){
		switch (this->params[0]){
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				break;
			case 6:
			case 7:
			case 8:
			case 9:
				this->params[0] += 2;
				break;
			default:
				{
					std::stringstream error_stream;
					error_stream << "First snare argument must be in the range [1; 9]. Encountered: " << this->params[0];
					throw std::runtime_error(error_stream.str());
				}
		}
	}
	u32 command_id() const override{ return (u32)AudioCommandType::NoiseInstrument; }
};
//DEFINE_SEQUENCE_CLASS(MutedSnare, 2);
class MutedSnareCommand : public AudioCommand{
public:
	MutedSnareCommand(std::istream &stream): AudioCommand(stream, 2){
		switch (this->params[0]){
			case 1:
				this->params[0] = 15;
				break;
			case 2:
			case 3:
			case 4:
				this->params[0] += 15;
				break;
			default:
				{
					std::stringstream error_stream;
					error_stream << "First muted_snare argument must be one of [1; 4]. Encountered: " << this->params[0];
					throw std::runtime_error(error_stream.str());
				}
		}
	}
	u32 command_id() const override{ return (u32)AudioCommandType::NoiseInstrument; }
};
DEFINE_SEQUENCE_CLASS(UnknownSfx10, 1);
DEFINE_SEQUENCE_CLASS(UnknownSfx20, 3);
DEFINE_SEQUENCE_CLASS(UnknownNoise20, 3);
DEFINE_SEQUENCE_CLASS(ExecuteMusic, 0);
//DEFINE_SEQUENCE_CLASS(PitchBend, 3);
class PitchBendCommand : public AudioCommand{
public:
	PitchBendCommand(std::istream &stream): AudioCommand(stream, 3){
		this->params[1] = calculate_frequency(this->params[1], this->params[2]);
		this->parameter_count = 2;
	}
	u32 command_id() const override{ return (u32)AudioCommandType::PitchBend; }
};
//DEFINE_SEQUENCE_CLASS(Triangle, 2);
class TriangleCommand : public AudioCommand{
public:
	TriangleCommand(std::istream &stream): AudioCommand(stream, 2){
		switch (this->params[0]){
			case 1:
				this->params[0] = 6;
				break;
			case 2:
				this->params[0] = 7;
				break;
			case 3:
				this->params[0] = 16;
				break;
			default:
				{
					std::stringstream error_stream;
					error_stream << "First triangle argument must be one of [1, 2, 3]. Encountered: " << this->params[0];
					throw std::runtime_error(error_stream.str());
				}
		}
	}
	u32 command_id() const override{ return (u32)AudioCommandType::NoiseInstrument; }
};
DEFINE_SEQUENCE_CLASS(StereoPanning, 1);
//DEFINE_SEQUENCE_CLASS(Cymbal, 2);
class CymbalCommand : public AudioCommand{
public:
	CymbalCommand(std::istream &stream): AudioCommand(stream, 2){
		switch (this->params[0]){
			case 1:
			case 2:
			case 3:
				this->params[0] += 11;
				break;
			default:
				{
					std::stringstream error_stream;
					error_stream << "First cymbal argument must be one of [1, 2, 3]. Encountered: " << this->params[0];
					throw std::runtime_error(error_stream.str());
				}
		}
	}
	u32 command_id() const override{ return (u32)AudioCommandType::NoiseInstrument; }
};
DEFINE_SEQUENCE_CLASS(Loop, 2);
DEFINE_SEQUENCE_CLASS(Call, 1);
DEFINE_SEQUENCE_CLASS(Goto, 1);
DEFINE_SEQUENCE_CLASS(IfRed, 0);
DEFINE_SEQUENCE_CLASS(Else, 0);
DEFINE_SEQUENCE_CLASS(EndIf, 0);
DEFINE_SEQUENCE_CLASS(End, 0);

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
	u32 get_channel_no() const{
		return this->channel_no;
	}
};

class AudioHeader{
	std::string name;
	u32 bank;
	std::vector<AudioChannel> channels;
	AudioResourceType type;
public:
	AudioHeader(const std::string &name, u32 bank, AudioResourceType type):
		name(name),
		bank(bank),
		type(type){}
	AudioHeader(AudioHeader &&other):
		name(std::move(other.name)),
		bank(other.bank),
		channels(std::move(other.channels)),
		type(other.type){}
	const AudioHeader &operator=(const AudioHeader &other){
		this->name = other.name;
		this->bank = other.bank;
		this->channels = other.channels;
		this->type = other.type;
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
		write_varint(headers, (u32)this->type);
		write_varint(headers, this->channels.size());
		for (auto &c : this->channels)
			c.serialize(headers);
	}
	const std::string &get_name() const{
		return this->name;
	}
	AudioResourceType get_type() const{
		return this->type;
	}

	bool get_is_music() const{
		return this->type == AudioResourceType::Music;
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

static AudioResourceType to_AudioResourceType(const std::string &s){
	if (s == "music")
		return AudioResourceType::Music;
	if (s == "noise")
		return AudioResourceType::NoiseInstrument;
	if (s == "cry")
		return AudioResourceType::Cry;
	if (s == "sfx")
		return AudioResourceType::Sfx;
	throw std::runtime_error("Error: Invalid audio resource type: \"" + s + "\". Must be \"music\", \"noise\", \"cry\", or \"sfx\".");
}

static std::vector<AudioHeader> parse_headers(const std::vector<std::string> &input){
	std::vector<AudioHeader> ret;

	AudioHeader *current_header = nullptr;
	for (auto &s : input){
		if (s[0] == '.'){
			std::stringstream stream(s.substr(1));
			std::string name;
			u32 bank;
			std::string header_type;
			stream >> name >> bank >> header_type;
			ret.emplace_back(name, bank, to_AudioResourceType(header_type));
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

void throw_channel_error(bool three, const char *command, const AudioSequence &sequence, const AudioChannel &channel, const AudioHeader &header){
	std::stringstream stream;
	stream << "Error: " << command << " used in channel";
	if (!three)
		stream << " other than";
	stream << " 3 or 7. Sequence: \"" << sequence.get_name()
		<< "\", channel: " << channel.get_channel_no() << ", header: \"" << header.get_name() << "\".\n";
	throw std::runtime_error(stream.str());
}

void throw_error_non_ch3(const char *command, const AudioSequence &sequence, const AudioChannel &channel, const AudioHeader &header){
	throw_channel_error(false, command, sequence, channel, header);
}

void throw_error_ch3(const char *command, const AudioSequence &sequence, const AudioChannel &channel, const AudioHeader &header){
	throw_channel_error(true, command, sequence, channel, header);
}

const char *to_string(AudioCommandType type){
	const char * const strings[] = {
		"Tempo",
		"Volume",
		"Duty",
		"DutyCycle",
		"Vibrato",
		"TogglePerfectPitch",
		"NoteType",
		"Rest",
		"Octave",
		"Note",
		"DSpeed",
		"Snare",
		"MutedSnare",
		"UnknownSfx10",
		"UnknownSfx20",
		"UnknownNoise20",
		"ExecuteMusic",
		"PitchBend",
		"Triangle",
		"StereoPanning",
		"Cymbal",
		"Loop",
		"Call",
		"Goto",
		"IfRed",
		"Else",
		"EndIf",
		"End",
	};
	assert((int)type < sizeof(strings) / sizeof(*strings));
	return strings[(int)type];
}

class AudioData{
	std::map<std::string, std::unique_ptr<AudioSequence>> sequences;
	std::vector<AudioHeader> headers;

	void load_main_file(const std::string &path){
		std::ifstream file(path, std::ios::binary);
		if (!file)
			throw std::runtime_error("File not found: " + path);

		auto lines = file_splitter(file);

		std::vector<std::string> sequence_strings;
		std::vector<std::string> header_strings;
		bool reading_sequences = true;
		while (lines.size()){
			auto line = move_pop_front(lines);
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
		std::ifstream file(path, std::ios::binary);
		if (!file)
			throw std::runtime_error("File not found: " + path);
		auto lines = file_splitter(file);
		std::vector<std::string> strings;
		strings.reserve(lines.size());
		while (lines.size()){
			auto line = move_pop_front(lines);
			if (line.size())
				strings.emplace_back(std::move(line));
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
	void check_header(const AudioHeader &header){
		std::vector<const AudioChannel *> lt_4;
		std::vector<const AudioChannel *> geq_4;
		for (auto &channel : header.get_channels()){
			if (header.get_type() == AudioResourceType::Cry && channel.get_channel_no() == 6)
				throw std::runtime_error("Error: Resource " + header.get_name() + " is marked as 'cry', but allocates audio channel 6. Pokemon cries cannot use channel 6.");

			(channel.get_channel_no() < 4 ? lt_4 : geq_4).push_back(&channel);
			std::set<std::string> stack;
			stack.insert(channel.get_referenced_sequence());
			auto it = this->sequences.find(channel.get_referenced_sequence());
			if (it == this->sequences.end())
				throw std::runtime_error("Error: Resource " + header.get_name() + " references non-existent sequence: " + channel.get_referenced_sequence());
			this->check_sequence(*it->second, channel, header, stack);
		}
		if (header.get_is_music() && geq_4.size()){
			std::cerr << "Warning: Header " << header.get_name() << " is marked as music, but allocates channels greater than channel No. 3. See the following channels:\n";
			for (auto channel : geq_4)
				std::cerr << "  * " << channel->get_referenced_sequence() << " => " << channel->get_channel_no() << std::endl;
		}
		if (!header.get_is_music() && lt_4.size()){
			std::cerr << "Warning: Header " << header.get_name() << " is marked as non-music, but allocates channels less than channel No. 4. See the following channels:\n";
			for (auto channel : lt_4)
				std::cerr << "  * " << channel->get_referenced_sequence() << " => " << channel->get_channel_no() << std::endl;
		}
	}
	void check_sequence(const AudioSequence &sequence, const AudioChannel &channel, const AudioHeader &header, std::set<std::string> &stack){
		auto ch = channel.get_channel_no();
		for (auto &command : sequence.get_commands()){
			auto cmd = (AudioCommandType)command->command_id();
			switch (cmd){
				case AudioCommandType::UnknownSfx20:
				case AudioCommandType::NoteType:
				case AudioCommandType::Note:
				case AudioCommandType::Vibrato:
					if (ch % 4 == 3)
						throw_error_ch3(to_string(cmd), sequence, channel, header);
					break;
				case AudioCommandType::UnknownNoise20:
				case AudioCommandType::NoiseInstrument:
					if (ch % 4 != 3)
						throw_error_non_ch3(to_string(cmd), sequence, channel, header);
					break;

				case AudioCommandType::Goto:
					this->check_sequence(*this->sequences.find(command->get_referenced_sequence())->second, channel, header, stack);
					return;
				case AudioCommandType::Call:
				case AudioCommandType::Loop:
					if (stack.find(command->get_referenced_sequence()) != stack.end())
						break;
					stack.insert(command->get_referenced_sequence());
					this->check_sequence(*this->sequences.find(command->get_referenced_sequence())->second, channel, header, stack);
					stack.erase(command->get_referenced_sequence());
					break;
				case AudioCommandType::End:
					return;


				default:
					break;
			}
		}
	}
public:
	AudioData(std::ostream &log_file){
		this->load_secondary_file(input_files[1]);
		this->load_main_file(input_files[0]);
		this->consistency_check();
		this->remove_unreachable_sequences(log_file);
		this->place_sequences();
	}
	void serialize_sequences(std::vector<std::uint8_t> &sequences) const{
		u32 command_count = 0;
		for (auto &s : this->sequences)
			command_count += (u32)s.second->get_commands().size();
		write_varint(sequences, command_count);
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

	void consistency_check(){
		for (auto &header : this->headers)
			this->check_header(header);
	}
};

static void write_header_and_source(const char *header_path, const char *source_path, const std::vector<AudioHeader> &headers, const AudioData &data){
	std::vector<std::uint8_t> sequences, serialized_headers;
	data.serialize_sequences(sequences);
	data.serialize_headers(serialized_headers);

	{
		std::ofstream header(header_path);
		header << "#pragma once\n"
			<< generated_file_warning <<
			"\n"
			"extern const byte_t audio_sequence_data[];\n"
			"static const size_t audio_sequence_data_size = " << sequences.size() << ";\n"
			"extern const byte_t audio_header_data[];\n"
			"static const size_t audio_header_data_size = " << serialized_headers.size() << ";\n"
			"enum class AudioResourceId{\n"
			"    None = 0,\n";
		size_t i = 1;
		for (auto &h : headers)
			header << "    " << h.get_name() << " = " << i++ << ",\n";
		header << "    Stop = " << i++ << ",\n"
			"};\n";
	}
	{
		std::ofstream source(source_path);
		source << generated_file_warning <<
			"\n"
			"extern const byte_t audio_sequence_data[] = ";
		write_buffer_to_stream(source, sequences);
		source << std::dec << ";\n"
			"extern const byte_t audio_header_data[] = ";
		write_buffer_to_stream(source, serialized_headers);
		source << std::dec << ";\n";
	}
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

	write_header_and_source("output/audio.h", "output/audio.inl", data.get_headers(), data);

	known_hashes[hash_key] = current_hash;
}

void generate_audio(known_hashes_t &known_hashes){
	try{
		generate_audio_internal(known_hashes);
	}catch (std::exception &e){
		throw std::runtime_error((std::string)"generate_audio(): " + e.what());
	}
}
