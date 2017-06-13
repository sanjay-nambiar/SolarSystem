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
		ConfigDataMapType dataMap;
		PopulateDataMap(filename, dataMap);
		PopulateDataObject(dataMap);
	}

	const CelestialBodyData& ConfigData::GetConstantsData() const
	{
		return mConstantsData;
	}

	const CelestialBodyData& ConfigData::GetCelestialBodyData(const std::string& sectionName) const
	{
		for (auto& data : mConfigData)
		{
			if (data.mName == sectionName)
			{
				return data;
			}
		}
		throw std::runtime_error("Invalid section name");
	}

	const std::vector<CelestialBodyData>& ConfigData::GetAllData() const
	{
		return mConfigData;
	}

	void ConfigData::PopulateDataMap(const std::string& filename, ConfigDataMapType& dataMap)
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
				dataMap[sectionName].insert({matches.str(1), matches.str(2)});
			}
		}
		file.close();
	}

	void ConfigData::PopulateDataObject(const ConfigDataMapType& configDataMap)
	{
		for (const auto& sectionEntry : configDataMap)
		{
			const auto& section = sectionEntry.second;

			if (sectionEntry.first == ConstantsSection)
			{
				mConstantsData = {
					sectionEntry.first,
					std::stoul(section.at("Ordinal")),
					section.at("Texture"),
					std::stof(section.at("MeanDistance")),
					std::stof(section.at("RotationPeriod")),
					std::stof(section.at("OrbitalPeriod")),
					std::stof(section.at("AxialTilt")),
					std::stof(section.at("Diameter")),
					std::stof(section.at("Albeido")), // reflectance
					std::stof(section.at("IsLit")),
					section.at("Parent")
				};
			}
			else
			{
				mConfigData.push_back({
					sectionEntry.first,
					std::stoul(section.at("Ordinal")),
					section.at("Texture"),
					std::stof(section.at("MeanDistance")),
					std::stof(section.at("RotationPeriod")),
					std::stof(section.at("OrbitalPeriod")),
					std::stof(section.at("AxialTilt")),
					std::stof(section.at("Diameter")),
					std::stof(section.at("Albeido")),	// reflectance
					std::stof(section.at("IsLit")),
					section.at("Parent")
				});
			}
		}
		std::sort(mConfigData.begin(), mConfigData.end(), [](const CelestialBodyData& a, const CelestialBodyData& b)
		{
			return a.mOrdinal <= b.mOrdinal;
		});
	}
}
