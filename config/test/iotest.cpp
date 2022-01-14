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

#include <catch2/catch.hpp>

#include <thread>

#include "ConfigObserverDummy.hpp"
#include <etherkitten/config/ConfigIO.hpp>
#include <fstream>
#include <iostream>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::config;

SCENARIO("ConfigIO creates the config directories as soon as it is created", "[ConfigIO]")
{
	GIVEN("a path")
	{
		std::filesystem::path configPath{ "./testconfig" };

		WHEN("I create the ConfigIO")
		{
			ConfigIO configIO{ configPath };

			THEN("the directory is created") { REQUIRE(std::filesystem::exists(configPath)); }

			THEN("the busses subdirectory is created")
			{
				REQUIRE(std::filesystem::exists(configPath / "busses"));
			}

			THEN("the file config.json is created")
			{
				REQUIRE(std::filesystem::exists(configPath / "config.json"));
			}

			// do cleanup
			std::filesystem::remove_all(configPath);
		}
	}
}

void requireFileContentIs(std::filesystem::path& filePath, std::string& expected);

SCENARIO("ConfigIO can handle exisiting config directories when it is created", "[ConfigIO]")
{
	GIVEN("a path with existing config directories and files")
	{
		std::filesystem::path configPath{ "./testconfig" };
		std::filesystem::path busPath{ configPath / "busses" };
		std::filesystem::path logJsonPath{ configPath / "config.json" };
		std::filesystem::create_directory(configPath);
		std::filesystem::create_directory(busPath);

		// Legal JSON pointing to a folder for the logs
		std::string logJson = "{\n    \"log_folder\": \"logs\"\n}\n";
		{
			std::fstream logFile{};
			logFile.open(logJsonPath, std::ios::out);
			if (logFile)
				logFile << logJson;
			else
				FAIL();
		}
		requireFileContentIs(logJsonPath, logJson);
		WHEN("I create the ConfigIO")
		{
			ConfigIO configIO{ configPath };
			std::filesystem::path configPath{ "./testconfig" };

			THEN("the directory is still there") { REQUIRE(std::filesystem::exists(configPath)); }

			THEN("the busses subdirectory is still there")
			{
				REQUIRE(std::filesystem::exists(busPath));
			}

			THEN("the file config.json is still intact")
			{
				REQUIRE(std::filesystem::exists(logJsonPath));
				requireFileContentIs(logJsonPath, logJson);
			}

			// do cleanup
			std::filesystem::remove_all(configPath);
		}
	}
}

SCENARIO("ConfigIO can read and write the log folder path", "[ConfigIO]")
{
	GIVEN("a ConfigIO instance and a log folder path")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		std::string logFolderPath{ "logs" };
		WHEN("I tell ConfigIO to write the log folder path")
		{
			configIO.writeLogFolderPath(logFolderPath);
			THEN("the file config.json (still) exists")
			{
				REQUIRE(std::filesystem::exists(configPath / "config.json"));
			}
			THEN("ConfigObservers have been notified") { REQUIRE(observer.logFolderPathNotified); }
			THEN("ConfigObservers have the correct path")
			{
				REQUIRE(observer.logFolderPathNotified);
				REQUIRE(observer.logFolderPath == logFolderPath);
			}
		}

		// do cleanup
		std::filesystem::remove_all(configPath);
	}
}

SCENARIO("ConfigIO can read and write the maximum memory", "[ConfigIO]")
{
	GIVEN("a ConfigIO instance and a memory limit")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		size_t maximumMemory = 321876;
		WHEN("I tell ConfigIO to write the log folder path")
		{
			configIO.writeMaximumMemory(maximumMemory);
			THEN("the file config.json (still) exists")
			{
				REQUIRE(std::filesystem::exists(configPath / "config.json"));
			}
			THEN("ConfigObservers have been notified") { REQUIRE(observer.maximumMemoryNotified); }
			THEN("ConfigObservers have the correct limit")
			{
				REQUIRE(observer.maximumMemoryNotified);
				REQUIRE(observer.maximumMemory == maximumMemory);
			}
		}

		// do cleanup
		std::filesystem::remove_all(configPath);
	}
}

SCENARIO("ConfigIO can read the default BusConfig", "[ConfigIO]")
{
	GIVEN("a ConfigIO instance")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		std::string logFolderPath{ "logs" };
		WHEN("I tell ConfigIO to read the default BusConfig")
		{
			configIO.readDefaultConfig();
			THEN("the file default_bus_config.json is created")
			{
				REQUIRE(std::filesystem::exists(configPath / "default_bus_config.json"));
			}
			THEN("ConfigObservers have been notified")
			{
				REQUIRE(observer.configNotified);
				REQUIRE_FALSE(observer.busId.has_value());
			}
		}
	}
}

void requireFileContentIs(std::filesystem::path& filePath, std::string& expected)
{
	std::fstream file{};
	file.open(filePath, std::ios::in);
	if (!file)
		FAIL();
	std::ostringstream fileContents;
	fileContents << file.rdbuf();
	REQUIRE(expected == fileContents.str());
}

SCENARIO("ConfigIO can deal with BusConfigs and BusLayouts", "[ConfigIO]")
{
	GIVEN("a ConfigIO instance and a BusConfig")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		BusConfig busConfig{};
		busConfig.setRegisterVisible(15, true);
		busConfig.setRegisterVisible(48, false);
		busConfig.setRegisterVisible(95, true);

		WHEN("I tell ConfigIO to write the BusConfig")
		{
			std::string busId = "busConfigTestBus";
			configIO.writeBusConfig(busConfig, busId);
			std::filesystem::path expectedPath{ configPath / "busses" / busId / "bus_config.json" };

			THEN("a file is created at the expected position")
			{
				REQUIRE(std::filesystem::exists(expectedPath));
			}
			THEN("ConfigObservers have been notified") { REQUIRE(observer.configNotified); }
			THEN("the BusConfig can be read again and is intact")
			{
				configIO.readBusConfig(busId);
				REQUIRE(observer.configNotified);
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(0));
				REQUIRE(observer.busConfig.isRegisterVisible(15));
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(48));
				REQUIRE(observer.busConfig.isRegisterVisible(95));
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(99));
			}

			// do cleanup
			std::filesystem::remove_all(configPath);
		}
	}

	GIVEN("a ConfigIO instance and a BusLayout")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		BusLayout busLayout{};
		Position position0{ 1, -41 };
		busLayout.setPosition(0, position0);
		Position position1{ -41, 4 };
		busLayout.setPosition(1, position1);

		WHEN("I tell ConfigIO to write the BusLayout")
		{
			std::string busId = "busLayoutTestBus";
			configIO.writeBusLayout(busLayout, busId);
			std::filesystem::path expectedPath{ configPath / "busses" / busId / "bus_config.json" };

			THEN("a file is created at the expected position")
			{
				REQUIRE(std::filesystem::exists(expectedPath));
			}
			THEN("ConfigObservers have been notified") { REQUIRE(observer.layoutNotified); }
			THEN("the BusLayout can be read again and is intact")
			{
				configIO.readBusLayout(busId);
				REQUIRE(observer.layoutNotified);
				REQUIRE(observer.busLayout.getPosition(0).x == position0.x);
				REQUIRE(observer.busLayout.getPosition(0).y == position0.y);
				REQUIRE(observer.busLayout.getPosition(1).x == position1.x);
				REQUIRE(observer.busLayout.getPosition(1).y == position1.y);
			}

			// do cleanup
			std::filesystem::remove_all(configPath);
		}
	}

	GIVEN("a ConfigIO instance, a BusConfig and a BusLayout")
	{
		std::filesystem::path configPath{ "./testconfig" };
		ConfigIO configIO{ configPath };
		ConfigObserverDummy observer;
		configIO.registerObserver(observer);
		BusConfig busConfig{};
		busConfig.setRegisterVisible(15, true);
		busConfig.setRegisterVisible(48, false);
		busConfig.setRegisterVisible(95, true);
		BusLayout busLayout{};
		Position position0{ 1, -41 };
		busLayout.setPosition(0, position0);
		Position position1{ -41, 4 };
		busLayout.setPosition(1, position1);
		WHEN("I tell ConfigIO to write the BusConfig and the BusLayout")
		{
			std::string busId = "testBus";
			configIO.writeBusConfig(busConfig, busId);
			configIO.writeBusLayout(busLayout, busId);
			std::filesystem::path expectedPath{ configPath / "busses" / busId / "bus_config.json" };

			THEN("a file is created at the expected position")
			{
				REQUIRE(std::filesystem::exists(expectedPath));
			}

			THEN("the BusConfig can be read again and is intact")
			{
				configIO.readBusConfig(busId);
				REQUIRE(observer.configNotified);
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(0));
				REQUIRE(observer.busConfig.isRegisterVisible(15));
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(48));
				REQUIRE(observer.busConfig.isRegisterVisible(95));
				REQUIRE_FALSE(observer.busConfig.isRegisterVisible(99));
			}

			THEN("the BusLayout can be read again and is intact")
			{
				configIO.readBusLayout(busId);
				REQUIRE(observer.layoutNotified);
				REQUIRE(observer.busLayout.getPosition(0).x == position0.x);
				REQUIRE(observer.busLayout.getPosition(0).y == position0.y);
				REQUIRE(observer.busLayout.getPosition(1).x == position1.x);
				REQUIRE(observer.busLayout.getPosition(1).y == position1.y);
			}

			// do cleanup
			std::filesystem::remove_all(configPath);
		}
	}
}
