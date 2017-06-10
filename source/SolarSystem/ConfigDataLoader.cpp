#include "pch.h"
#include "ConfigDataLoader.h"
#include <regex>

namespace Rendering
{
	const std::regex ConfigDataLoader::CommentPattern = std::regex("^\\s*#.*$");
	const std::regex ConfigDataLoader::SectionTagPattern = std::regex("^\\s*\\[\\s*([A-Za-z0-9_]+)\\s*\\]\\s*$");
	const std::regex ConfigDataLoader::AttributeLinePattern = std::regex("^\\s*([^=]+)=(.*)$");

	void ConfigDataLoader::LoadConfigData(const std::string& filename)
	{
		CreateDataMap(filename);
		CreateDataObject();
	}

	const ConfigData& ConfigDataLoader::GetSectionData(const std::string& sectionName) const
	{
		return mConfigData.at(sectionName);
	}

	void ConfigDataLoader::CreateDataMap(const std::string& filename)
	{
		std::ifstream file;
		file.open(filename);
		if (!file.is_open())
		{
			throw std::runtime_error("Unable to read module config file: " + filename);
		}

		std::string line;
		std::string sectionName;
		while (std::getline(file, line))
		{
			std::smatch matches;
			if (std::regex_search(line, matches, CommentPattern))
			{
				continue;
			}
			
			if (std::regex_search(line, matches, SectionTagPattern))
			{
				sectionName = matches.str(1);
			}
			else if (std::regex_search(line, matches, AttributeLinePattern))
			{
				mConfigDataMap[sectionName].insert({matches.str(1), matches.str(2)});
			}
		}
		file.close();
	}

	void ConfigDataLoader::CreateDataObject()
	{
		for (const auto& sectionEntry : mConfigDataMap)
		{
			const auto& section = sectionEntry.second;
			mConfigData.insert({sectionEntry.first, {
				std::stof(section.at("MeanDistance")),
				std::stof(section.at("RotationPeriod")),
				std::stof(section.at("OrbitalPeriod")),
				std::stof(section.at("AxialTilt")),
				std::stof(section.at("Diameter")),
				std::stof(section.at("Albedo")),
				section.at("OrbitCenter")
			}});
		}
	}
}
