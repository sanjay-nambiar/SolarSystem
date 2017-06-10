#pragma once

#include <regex>
#include <unordered_map>

namespace Rendering
{
	struct SectionData
	{
		std::string mTextureName;
		float mMeanDistance;
		float mRotationPeriod;
		float mOrbitalPeriod;
		float mAxialTilt;
		float mDiameter;
		float mAlbedo;
		std::string mOrbitCenter;
	};

	class ConfigData
	{
	public:
		void LoadConfigData(const std::string& filename);
		const SectionData& GetConstantsData() const;
		const SectionData& GetSectionData(const std::string& sectionName) const;
		const std::unordered_map<std::string, SectionData>& GetAllData() const;
	private:
		void CreateDataMap(const std::string& filename);
		void CreateDataObject();

		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mConfigDataMap;
		std::unordered_map<std::string, SectionData> mConfigData;
		SectionData mConstantsData;

		static const std::regex CommentPattern;
		static const std::regex SectionTagPattern;
		static const std::regex AttributeLinePattern;
		static const std::string ConstantsSection;
	};
}
