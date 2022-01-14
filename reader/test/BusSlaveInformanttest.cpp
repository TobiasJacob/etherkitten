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
#include <test/test_globals.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <etherkitten/reader/BusSlaveInformant.hpp>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/errors.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO(
    "The BusSlaveInformant can read the correct data from a test bus", "[BusSlaveInformant],[.BUS]")
{
	GIVEN("A bus interface")
	{
		int socket = etherCATSocket;

		WHEN("I construct a BusSlaveInformant")
		{
			BusSlaveInformant bSInformant(socket);

			THEN("I get a bunch of data")
			{
				std::filesystem::path testDir("./test_results");
				std::filesystem::create_directory(testDir);
				std::fstream dumpFile(testDir / "busSlaveInfoDump", std::ios::out);
				dumpFile << "Slave count: " << bSInformant.getSlaveCount() << "\n\n";
				dumpFile << "Initialization errors:\n";
				for (const ekdatatypes::ErrorMessage& e : bSInformant.getInitializationErrors())
				{
					switch (e.getSeverity())
					{
					case ekdatatypes::ErrorSeverity::FATAL:
						dumpFile << "FATAL: ";
						break;
					case ekdatatypes::ErrorSeverity::MEDIUM:
						dumpFile << "MEDIUM: ";
						break;
					case ekdatatypes::ErrorSeverity::LOW:
						dumpFile << "LOW: ";
						break;
					}
					dumpFile << "[Slave " << e.getAssociatedSlaves().first << "] ";
					dumpFile << e.getMessage() << "\n";
				}
				dumpFile << "\n";
				dumpFile << "Slave information:\n";
				for (size_t i = 1; i <= bSInformant.getSlaveCount(); ++i)
				{
					const ekdatatypes::SlaveInfo& si = bSInformant.getSlaveInfo(i);
					dumpFile << "Slave " << si.getID() << "\n";
					dumpFile << "PDOs:\n";
					for (const ekdatatypes::PDO& p : si.getPDOs())
					{
						dumpFile << "\tSlave ID: " << p.getSlaveID() << ", Name: " << p.getName()
						         << ", Type: " << static_cast<int>(p.getType())
						         << ", Index: " << p.getIndex()
						         << ", Dir: " << static_cast<int>(p.getDirection()) << "\n";
					}

					dumpFile << "CoEs:\n";
					for (const ekdatatypes::CoEEntry& c : si.getCoEs())
					{
						dumpFile << "\tSlave ID: " << c.getSlaveID() << ", Name: " << c.getName()
						         << ", Code: " << static_cast<int>(c.getObjectCode())
						         << ", Index: " << c.getIndex() << "\n";
						for (const ekdatatypes::CoEObject& co : c.getObjects())
						{
							dumpFile << "\t\tSlave ID: " << co.getSlaveID()
							         << ", Name: " << co.getName()
							         << ", Type: " << static_cast<int>(co.getType())
							         << ", Index: " << co.getIndex()
							         << ", Subindex: " << co.getSubIndex()
							         << ", Access: " << co.isReadableInSafeOp() << " "
							         << co.isWritableInSafeOp() << " " << co.isReadableInOp() << " "
							         << co.isWritableInOp() << "\n";
						}
					}

					std::fstream esiFile(testDir / ("slave" + std::to_string(i) + "-esi"),
					    std::ios::out | std::ios::binary);
					esiFile.write(reinterpret_cast<const char*>(si.getESIBinary().data()), // NOLINT
					    si.getESIBinary().size());
					esiFile.close();
				}
				dumpFile << "BusInfo:\n";
				dumpFile << "Actual IOMap size: " << bSInformant.getBusInfo().ioMapUsedSize << "\n";
				dumpFile << "PDO offsets:\n";
				for (const auto& it : bSInformant.getBusInfo().pdoOffsets)
				{
					dumpFile << "[Slave " << it.first.getSlaveID() << ", Index "
					         << it.first.getIndex() << ", Name " << it.first.getName()
					         << "]: Offset " << it.second.bitOffset << ", Length "
					         << it.second.bitLength << "\n";
				}
				dumpFile.close();
				WARN("This test succeeded because BusSlaveInformant didn't throw an exception."
				     " However, this does not mean the information read is necessarily correct."
				     " Inspect the written files manually to ensure they are correct.");
				SUCCEED();
			}
		}
	}
}
