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

void serialize_csv(const char *path, const nlohmann::json &json){
	std::ofstream file(path);
	file << "id,name,object_name,type,x,y,param1,param2,param3,param4,param5,param6,param7\n";
	
	int i = 1;
	for (auto &array : json["map_objects"]){
		for (auto &object : array["objects"]){
			auto type = object["type"];
			file << i << "," << array["name"] << ",," << type << "," << object["x"] << "," << object["y"];
			if (type == "event_disp"){
			}else if (type == "sign"){
				file << "," << object["text_index"];
			}else if (type == "hidden"){
				file << "," << object["param"] << "," << object["script"];
			}else if (type == "warp"){
				file << "," << object["warp_index"] << "," << object["destination"] << "," << object["destination_warp_index"];
			}else if (type == "npc"){
				file << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"];
			}else if (type == "item"){
				file << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["item_id"];
			}else if (type == "pokemon"){
				file << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["species"] << "," << object["level"];
			}else if (type == "trainer"){
				file << "," << object["sprite"] << "," << object["direction"] << "," << object["move"] << "," << object["range"] << "," << object["text_id"] << "," << object["trainer_class"] << "," << object["trainer_number"];
			}else
				assume(false);
			file << std::endl;
			i++;
		}
	}
}

int main(){
	auto json = load_data();
	serialize_json("map_objects.json", json);
	serialize_csv("map_objects.csv", json);

	return 0;
}
