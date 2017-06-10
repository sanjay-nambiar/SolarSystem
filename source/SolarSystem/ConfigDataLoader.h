#pragma once

#include <regex>
#include <unordered_map>

namespace Rendering
{
	struct ConfigData
	{
		float MeanDistance;
		float RotationPeriod;
		float OrbitalPeriod;
		float AxialTilt;
		float Diameter;
		float Albedo;
		std::string OrbitCenter;
	};

	class ConfigDataLoader
	{
	public:
		void LoadConfigData(const std::string& filename);
		const ConfigData& GetSectionData(const std::string& sectionName) const;
	private:
		void CreateDataMap(const std::string& filename);
		void CreateDataObject();

		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mConfigDataMap;
		std::unordered_map<std::string, ConfigData> mConfigData;

		static const std::regex CommentPattern;
		static const std::regex SectionTagPattern;
		static const std::regex AttributeLinePattern;
	};
}
