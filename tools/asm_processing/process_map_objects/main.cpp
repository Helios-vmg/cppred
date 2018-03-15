#include "FirstLevelParser.h"
#include "SecondLevelParser.h"
#include "HiddenObjectParser.h"
#include <iostream>
#include <boost/filesystem.hpp>
#include <set>

std::vector<std::unique_ptr<FirstLevelParser::Parser>> parse_directory(const std::string &path){
	typedef boost::filesystem::directory_iterator di;

	std::vector<std::unique_ptr<FirstLevelParser::Parser>> ret;
	for (di i(path), e; i != e; ++i){
		auto filepath = i->path().string();
		std::cout << "Parsing " << filepath << std::endl;
		ret.emplace_back(std::make_unique<FirstLevelParser::Parser>(filepath));
	}
	return ret;
}

nlohmann::json load_data(){
	HiddenObjectParser hop("../../pokered/data/hidden_objects.asm");
	std::vector<std::unique_ptr<FirstLevelParser::Parser>> parsers;
	try{
		parsers = parse_directory("../../pokered/data/mapObjects");
	} catch (std::exception &){
		std::cerr << "Parsing error.\n";
		return -1;
	}

	std::vector<std::unique_ptr<SecondLevelParser::Parser>> parsers2;
	for (auto &p : parsers)
		parsers2.emplace_back(std::make_unique<SecondLevelParser::Parser>(*p));

	nlohmann::json serialized;
	nlohmann::json::array_t map_objects;
	for (auto &p : parsers2)
		map_objects.push_back(p->serialize());
	{
		auto temp = hop.serialize();
		for (auto &j : map_objects){
			std::string name = j.value("name", "");
			auto it = temp.find(name);
			if (it == temp.end())
				continue;
			for (auto &i : it->second)
				j["objects"].push_back(i);
		}
	}
	serialized["map_objects"] = map_objects;
	return serialized;
}

void serialize_json(const char *path, const nlohmann::json &json){
	std::ofstream file(path);
	file << json.dump(true);
}

#define USE_TEMP

void serialize_csv(const char *path, const nlohmann::json &json){
	std::ofstream file(path);
#ifdef USE_TEMP
	std::stringstream temp;
	std::ostream &stream = temp;
#else
	std::ostream &stream = file;
#endif
	stream << "id,name,legacy_sprite_id,type,x,y,param1,param2,param3,param4,param5,param6,param7\n";
	
	int i = 1;
	for (auto &array : json["map_objects"]){
		for (auto &object : array["objects"]){
			auto type = object["type"];
			stream << i << "," << array["name"] << ",";
			{
				auto it = object.find("legacy_sprite_id");
				if (it != object.end()){
					auto temp2 = it->get<int>();
					stream << temp2;
				}
			}
			stream << "," << type << "," << object["x"] << "," << object["y"];
			int count;
			if (type == "event_disp"){
				count = 0;
			}else if (type == "sign"){
				count = 1;
				stream << "," << object["text_index"];
			}else if (type == "hidden"){
				count = 2;
				stream << "," << object["param"] << "," << object["script"];
			}else if (type == "warp"){
				count = 3;
				stream << "," << object["warp_index"] << "," << object["destination"] << "," << object["destination_warp_index"];
			}else if (type == "npc"){
				count = 5;
				stream << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"];
			}else if (type == "item"){
				count = 6;
				stream << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["item_id"];
			}else if (type == "pokemon"){
				count = 7;
				stream << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["species"] << "," << object["level"];
			}else if (type == "trainer"){
				count = 7;
				stream << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["trainer_class"] << "," << object["trainer_number"];
			}else
				assume(false);
			for (int j = count; j < 7; j++)
				stream << ',';
			stream << std::endl;
			i++;
		}
	}

#ifdef USE_TEMP
	auto s = temp.str();
	for (auto c : s){
		if (c == '"')
			continue;
		file << c;
	}
#endif
}

int main(){
	auto json = load_data();
	serialize_json("map_objects.json", json);
	serialize_csv("map_objects.csv", json);

	return 0;
}
