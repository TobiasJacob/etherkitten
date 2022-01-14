/*
 * Copyright 2021 Niklas Arlt, Matthias Becht, Florian Bossert, Marwin Madsen, and Philip Scherer
 *
 * This file is part of EtherKITten.
 *
 * EtherKITten is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * EtherKITten is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EtherKITten.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "ConfigIO.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace etherkitten::config
{

	ConfigIO::ConfigIO(std::filesystem::path configPath)
	    : configPath(configPath)
	{
		createConfigDirectory();
	}

	void ConfigIO::writeBusConfig(const BusConfig& busConfig, const std::string& busId)
	{
		std::filesystem::path filePath(configPath / "busses" / busId / "bus_config.json");
		nlohmann::json json;
		if (std::filesystem::exists(filePath))
			json = readJsonFromFile(filePath);
		else
			std::filesystem::create_directories(configPath / "busses" / busId);

		serializeBusConfig(busConfig, json);
		writeJsonToFile(json, filePath);

		for (auto observer : activeObservers)
			observer->onBusConfigChanged(busConfig, busId);
	}

	void ConfigIO::serializeBusConfig(const BusConfig& busConfig, nlohmann::json& json)
	{
		std::vector<std::string> registerList;
		for (uint32_t visibleReg : busConfig.getVisibleRegisters())
		{
			std::stringstream stream;
			stream << "0x" << std::setfill('0') << std::setw(4) << std::hex << visibleReg;
			registerList.push_back(stream.str());
		}

		json["show_registers"] = registerList;
	}

	void ConfigIO::readDefaultConfig()
	{
		auto defaultConfig = getDefaultConfig();
		for (auto observer : activeObservers)
		{
			observer->onBusConfigChanged(defaultConfig, std::nullopt);
		}
	}

	void ConfigIO::readBusConfig(const std::string& busId)
	{
		std::filesystem::path filePath(configPath / "busses" / busId / "bus_config.json");
		if (std::filesystem::exists(filePath))
		{
			nlohmann::json json = readJsonFromFile(filePath);
			BusConfig busConfig{ parseBusConfig(json) };

			for (auto observer : activeObservers)
				observer->onBusConfigChanged(busConfig, busId);
		}
		else
		{
			BusConfig busConfig = getDefaultConfig();
			for (auto observer : activeObservers)
				observer->onBusConfigChanged(busConfig, busId);
		}
	}

	BusConfig ConfigIO::parseBusConfig(nlohmann::json& json)
	{
		std::vector<std::string> regStringList = json["show_registers"];
		BusConfig busConfig;
		for (std::string s : regStringList)
		{
			busConfig.setRegisterVisible(std::stoul(s, 0, 16), true);
		}
		return busConfig;
	}

	BusConfig ConfigIO::getDefaultConfig()
	{
		std::filesystem::path filePath(configPath / "default_bus_config.json");
		if (std::filesystem::exists(filePath))
		{
			nlohmann::json json = readJsonFromFile(filePath);
			return parseBusConfig(json);
		}
		else
		{
			BusConfig defaultConfig;
			defaultConfig.setRegisterVisible(0x0111, true);
			defaultConfig.setRegisterVisible(0x0120, true);
			defaultConfig.setRegisterVisible(0x0130, true);
			defaultConfig.setRegisterVisible(0x0300, true);
			defaultConfig.setRegisterVisible(0x0301, true);
			defaultConfig.setRegisterVisible(0x0302, true);
			defaultConfig.setRegisterVisible(0x0303, true);
			defaultConfig.setRegisterVisible(0x0304, true);
			defaultConfig.setRegisterVisible(0x0305, true);
			defaultConfig.setRegisterVisible(0x0306, true);
			defaultConfig.setRegisterVisible(0x0307, true);
			defaultConfig.setRegisterVisible(0x0308, true);
			defaultConfig.setRegisterVisible(0x0309, true);
			defaultConfig.setRegisterVisible(0x030A, true);
			defaultConfig.setRegisterVisible(0x030B, true);
			defaultConfig.setRegisterVisible(0x030C, true);
			defaultConfig.setRegisterVisible(0x030D, true);
			defaultConfig.setRegisterVisible(0x0310, true);
			defaultConfig.setRegisterVisible(0x0311, true);
			defaultConfig.setRegisterVisible(0x0312, true);
			defaultConfig.setRegisterVisible(0x0313, true);

			nlohmann::json json;
			serializeBusConfig(defaultConfig, json);
			writeJsonToFile(json, configPath / "default_bus_config.json");

			return defaultConfig;
		}
	}

	void ConfigIO::writeBusLayout(const BusLayout& busLayout, const std::string& busId)
	{
		std::filesystem::path filePath(configPath / "busses" / busId / "bus_config.json");
		nlohmann::json json;
		if (std::filesystem::exists(filePath))
			json = readJsonFromFile(filePath);
		else
			std::filesystem::create_directories(configPath / "busses" / busId);

		serializeBusLayout(busLayout, json);
		writeJsonToFile(json, filePath);

		for (auto observer : activeObservers)
			observer->onBusLayoutChanged(busLayout, busId);
	}

	void ConfigIO::serializeBusLayout(const BusLayout& busLayout, nlohmann::json& json)
	{
		std::vector<nlohmann::json> posVec;
		for (size_t i = 0; i <= busLayout.getSlaveCount(); ++i)
		{
			Position slavePos = busLayout.getPosition(i);
			nlohmann::json posJson;
			posJson["x"] = slavePos.x;
			posJson["y"] = slavePos.y;
			posVec.push_back(posJson);
		}
		json["graph"]["slaves"] = posVec;
	}

	void ConfigIO::readBusLayout(const std::string& busId)
	{
		std::filesystem::path filePath(configPath / "busses" / busId / "bus_config.json");
		BusLayout busLayout{};
		if (std::filesystem::exists(filePath))
		{
			nlohmann::json json = readJsonFromFile(filePath);
			busLayout = parseBusLayout(json);
		}
		for (auto observer : activeObservers)
			observer->onBusLayoutChanged(busLayout, busId);
	}

	BusLayout ConfigIO::parseBusLayout(const nlohmann::json& json)
	{
		BusLayout layout;
		std::vector<nlohmann::json> slavePosJson = json["graph"]["slaves"];
		for (size_t i = 0; i < slavePosJson.size(); ++i)
		{
			Position slavePos(slavePosJson[i]["x"], slavePosJson[i]["y"]);
			layout.setPosition(i, slavePos);
		}
		return layout;
	}

	void ConfigIO::writeLogFolderPath(const std::filesystem::path logFolderPath)
	{
		std::filesystem::path filePath(configPath / "config.json");
		nlohmann::json json;

		if (std::filesystem::exists(filePath))
		{
			json = readJsonFromFile(filePath);
		}

		json["log_folder"] = logFolderPath;
		writeJsonToFile(json, filePath);

		// Read log again so that observers are notified and substitutions are done
		readLogFolderPath();
	}

	void ConfigIO::readLogFolderPath()
	{
		std::filesystem::path filePath(configPath / "config.json");
		if (std::filesystem::exists(filePath))
		{
			nlohmann::json json = readJsonFromFile(filePath);
			if (json.contains("log_folder"))
			{
				std::filesystem::path logFolderPath
				    = std::filesystem::path{ json["log_folder"].get<std::string>() };

				// Replace ~ because path cannot handle it
				if (logFolderPath.string()[0] == '~')
				{
					if (logFolderPath.string().size() > 1)
					{
						logFolderPath = std::filesystem::path(getHomeDir())
						    / logFolderPath.string().substr(2);
					}
					else // filePath == '~'
					{
						logFolderPath = std::filesystem::path(getHomeDir());
					}
				}

				// Notify observers
				for (auto observer : activeObservers)
					observer->onLogPathChanged(logFolderPath);
				return;
			}
		}
		// Writing log also triggers a read
		writeLogFolderPath(defaultLogFolderPath);
	}

	void ConfigIO::writeMaximumMemory(size_t maximumMemory)
	{
		std::filesystem::path filePath(configPath / "config.json");
		nlohmann::json json;

		if (std::filesystem::exists(filePath))
		{
			json = readJsonFromFile(filePath);
		}

		json["maximum_memory"] = maximumMemory;
		writeJsonToFile(json, filePath);

		// Read log again so that observers are notified and substitutions are done
		readMaximumMemory();
	}

	void ConfigIO::readMaximumMemory()
	{
		std::filesystem::path filePath(configPath / "config.json");
		if (std::filesystem::exists(filePath))
		{
			nlohmann::json json = readJsonFromFile(filePath);
			if (json.contains("maximum_memory"))
			{
				size_t maximumMemory = json["maximum_memory"].get<size_t>();

				// Notify observers
				for (auto observer : activeObservers)
					observer->onMaximumMemoryChanged(maximumMemory);
				return;
			}
		}
		// Writing the maximum memory also triggers a read
		// 0 means unlimited
		writeMaximumMemory(0);
	}

	void ConfigIO::registerObserver(ConfigObserver& observer)
	{
		if (std::find(activeObservers.begin(), activeObservers.end(), &observer)
		    == activeObservers.end())
			activeObservers.push_back(&observer);
	}

	void ConfigIO::removeObserver(const ConfigObserver& observer)
	{
		auto it = std::find(activeObservers.begin(), activeObservers.end(), &observer);
		if (it != activeObservers.end())
			activeObservers.erase(it);
	}

	const std::vector<std::string> ConfigIO::getConfiguredBusses()
	{
		std::vector<std::string> vec;
		std::filesystem::directory_iterator iterator(configPath / "busses");
		for (const auto& entry : iterator)
			if (entry.is_directory())
			{
				vec.push_back(entry.path().filename());
			}
		return vec;
	}

	void ConfigIO::createConfigDirectory()
	{
		std::filesystem::create_directories(configPath);
		std::filesystem::create_directories(configPath / "busses");
		readLogFolderPath(); // Creates the default log folder config file if it did not exist
		readDefaultConfig(); // Creates the default config file if it did not exist
	}

	void ConfigIO::writeJsonToFile(
	    const nlohmann::json& json, const std::filesystem::path& filePath)
	{
		std::ofstream file(filePath);
		file << std::setw(4) << json << std::endl;
	}

	nlohmann::json ConfigIO::readJsonFromFile(const std::filesystem::path& filePath)
	{
		std::ifstream file(filePath);
		return nlohmann::json::parse(file);
	}

} // namespace etherkitten::config
