#pragma once

#include <regex>
#include <unordered_map>
#include "CeledtialBodyData.h"

namespace Rendering
{
	class ConfigData
	{
	public:
		void LoadConfigData(const std::string& filename);
		const CelestialBodyData& GetConstantsData() const;
		const CelestialBodyData& GetCelestialBodyData(const std::string& sectionName) const;
		const std::vector<CelestialBodyData>& GetAllData() const;
	private:
		typedef std::unordered_map<std::string, std::unordered_map<std::string, std::string>> ConfigDataMapType;

		static void PopulateDataMap(const std::string& filename, ConfigDataMapType& dataMap);
		void PopulateDataObject(const ConfigDataMapType& configDataMap);

		std::vector<CelestialBodyData> mConfigData;
		CelestialBodyData mConstantsData;

		static const std::regex CommentPattern;
		static const std::regex SectionTagPattern;
		static const std::regex AttributeLinePattern;
		static const std::string ConstantsSection;
	};
}
