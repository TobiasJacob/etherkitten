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

#include "etherkitten/datatypes/time.hpp"
#include <catch2/catch.hpp>
#include <chrono>
#include <test/test_globals.hpp>

#include <filesystem>
#include <fstream>
#include <thread>

#include <etherkitten/reader/BusReader.hpp>

#include <etherkitten/datatypes/dataobjects.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("The BusReader can read data from the EtherCAT bus", "[BusReader],[.BUS]")
{
	GIVEN("An initialized bus")
	{
		int socket = etherCATSocket;
		BusSlaveInformant bSInformant(socket);
		BusQueues queues;
		std::unordered_map<ekdatatypes::RegisterEnum, bool> regMap = {
			{ ekdatatypes::RegisterEnum::BUILD, true },
			{ ekdatatypes::RegisterEnum::LOST_LINK_COUNTER_PORT_0, true },
			{ ekdatatypes::RegisterEnum::CONFIGURED_STATION_ADDRESS, true },
			{ ekdatatypes::RegisterEnum::REVISION, true },
		};

		WHEN("I construct a BusReader and stop it after a while")
		{
			using namespace std::chrono_literals;
			BusReader reader(bSInformant, queues, regMap);
			std::vector<std::pair<double, double>> frequencies;
			ekdatatypes::TimeStamp start(ekdatatypes::now());
			ekdatatypes::TimeStamp current(ekdatatypes::now());
			while (current - start < 5s)
			{
				frequencies.push_back({ reader.getPDOFrequency(), reader.getRegisterFrequency() });
				std::this_thread::sleep_for(100ms);
				current = ekdatatypes::now();
			}
			reader.messageHalt();

			THEN("The BusReader will have a lot of data available")
			{
				std::filesystem::path testDir("./test_results");
				std::filesystem::create_directory(testDir);
				std::fstream dumpFile(testDir / "busReaderDump", std::ios::out);
				for (unsigned int slave = 1; slave <= bSInformant.getSlaveCount(); ++slave)
				{
					ekdatatypes::Register reg(slave, ekdatatypes::RegisterEnum::BUILD);
					std::shared_ptr<ekdatatypes::AbstractDataView> buildView
					    = reader.getView(reg, ekdatatypes::TimeSeries{ {}, 0s });
					while (buildView->hasNext())
					{
						dumpFile << "Value: " << buildView->asDouble() << ", Time: "
						         << std::chrono::duration_cast<std::chrono::microseconds>(
						                buildView->getTime().time_since_epoch())
						                .count()
						         << "\n";
						++*buildView;
					}
				}
				for (auto freq : frequencies)
				{
					dumpFile << "PDO Freq: " << freq.first << ", Register Freq: " << freq.second
					         << "\n";
				}
				dumpFile.close();
			}
		}
	}
}
