#include "pch.h"
#include "ConfigData.h"
#include <regex>

namespace Rendering
{
	const std::regex ConfigData::CommentPattern = std::regex("^\\s*#.*$");
	const std::regex ConfigData::SectionTagPattern = std::regex("^\\s*\\[\\s*([A-Za-z0-9_]+)\\s*\\]\\s*$");
	const std::regex ConfigData::AttributeLinePattern = std::regex("^\\s*([^=]+)=(.*)$");
	const std::string ConfigData::ConstantsSection = "Constants";

	void ConfigData::LoadConfigData(const std::string& filename)
	{
		CreateDataMap(filename);
		CreateDataObject();
	}

	const SectionData& ConfigData::GetConstantsData() const
	{
		return mConstantsData;
	}

	const SectionData& ConfigData::GetSectionData(const std::string& sectionName) const
	{
		return mConfigData.at(sectionName);
	}

	const std::unordered_map<std::string, SectionData>& ConfigData::GetAllData() const
	{
		return mConfigData;
	}

	void ConfigData::CreateDataMap(const std::string& filename)
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

	void ConfigData::CreateDataObject()
	{
		for (const auto& sectionEntry : mConfigDataMap)
		{
			const auto& section = sectionEntry.second;

			if (sectionEntry.first == ConstantsSection)
			{
				mConstantsData = {
					section.at("Texture"),
					std::stof(section.at("MeanDistance")),
					std::stof(section.at("RotationPeriod")),
					std::stof(section.at("OrbitalPeriod")),
					std::stof(section.at("AxialTilt")),
					std::stof(section.at("Diameter")),
					std::stof(section.at("Lit")),
					section.at("OrbitCenter")
				};
			}
			else
			{
				mConfigData.insert({sectionEntry.first,{
					section.at("Texture"),
					std::stof(section.at("MeanDistance")),
					std::stof(section.at("RotationPeriod")),
					std::stof(section.at("OrbitalPeriod")),
					std::stof(section.at("AxialTilt")),
					std::stof(section.at("Diameter")),
					std::stof(section.at("Lit")),
					section.at("OrbitCenter")
				}});
			}
		}
	}
}
