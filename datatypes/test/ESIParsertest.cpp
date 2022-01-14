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

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <etherkitten/datatypes/esiparser.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

std::optional<ESIPDOEntry> findPDOEntry(ESIData& esiData, size_t index, size_t subIndex)
{
	for (auto& pdoObject : esiData.rxPDO)
		for (auto pdoEntry : pdoObject.entries)
			if (pdoEntry.index == index && pdoEntry.subIndex == subIndex)
				return std::make_optional<ESIPDOEntry>(pdoEntry);

	for (auto& pdoObject : esiData.txPDO)
		for (auto pdoEntry : pdoObject.entries)
			if (pdoEntry.index == index && pdoEntry.subIndex == subIndex)
				return std::make_optional<ESIPDOEntry>(pdoEntry);
	return std::nullopt;
}

SCENARIO("ESIParser can parse ESI", "[ESIParser]")
{
	GIVEN("an empty byte buffer")
	{
		std::vector<std::byte> empty;
		WHEN("I feed it to the ESIParser")
		{
			THEN("it throws a ParseException")
			{
				REQUIRE_THROWS_AS(esiparser::parseESI(empty), esiparser::ParseException);
			}
		}
	}
	GIVEN("an illegal byte buffer")
	{
		std::vector<std::byte> illegal;
		illegal.emplace_back(static_cast<std::byte>(0));
		illegal.emplace_back(static_cast<std::byte>(1));
		illegal.emplace_back(static_cast<std::byte>(2));
		illegal.emplace_back(static_cast<std::byte>(3));
		illegal.emplace_back(static_cast<std::byte>(4));
		illegal.emplace_back(static_cast<std::byte>(5));
		illegal.emplace_back(static_cast<std::byte>(6));
		illegal.emplace_back(static_cast<std::byte>(7));
		WHEN("I feed it to the ESIParser")
		{
			THEN("it throws a ParseException")
			{
				REQUIRE_THROWS_AS(esiparser::parseESI(illegal), esiparser::ParseException);
			}
		}
	}
	GIVEN("an ESI binary file")
	{
		std::fstream binFile;
		std::filesystem::path testRes1{ "./test_resources" };
		std::filesystem::path testRes2{ "../test_resources" };
		std::filesystem::path esiFileName{ "exampleESI.bin" };
		binFile.open(testRes1 / esiFileName, std::ios::in | std::ios::binary);
		if (binFile.fail())
		{
			binFile.open(testRes2 / esiFileName, std::ios::in | std::ios::binary);
			if (binFile.fail())
			{
				throw std::runtime_error(
				    "Could not find the test resource 'test_resources/exampleESI.bin'."
				    " Please run this test in the 'build/' or 'build/bin' directories.");
			}
		}
		std::vector<std::byte> esiBinary(8192);
		binFile.read((char*)&esiBinary[0], esiBinary.size());
		binFile.close();

		WHEN("I parse it with the ESIParser")
		{
			ESIData esiData = esiparser::parseESI(esiBinary);
			THEN("the ESI header is parsed properly")
			{
				REQUIRE(esiData.header.vendorID == 0x000007bc);
				REQUIRE(esiData.header.productCode == 0x00000102);
				REQUIRE(esiData.header.revisionNumber == 0x00000002);
				REQUIRE(esiData.header.version == 0x0001);
			}
			THEN("the ESI string category is parsed properly")
			{
				bool containsName = false;
				for (auto& str : esiData.strings)
				{
					if (str == "SSC_Device")
					{
						containsName = true;
						break;
					}
				}
				REQUIRE(containsName);
			}
			THEN("the ESI general category is parsed properly")
			{
				REQUIRE(esiData.general.has_value());
				REQUIRE(esiData.general.value().coEDetails == 0x03);
				REQUIRE(esiData.general.value().foEDetails == 0x00);
				REQUIRE(esiData.general.value().eoEDetails == 0x00);
				REQUIRE(esiData.general.value().currentOnEBus == 0);
				// port 0 used -> 4 lowest bits have to be non-zero
				REQUIRE((esiData.general.value().physicalPort & 0b00000000000001111) == 1);
				// port 1 used -> second 4-bit block has to be non-zero
				REQUIRE((esiData.general.value().physicalPort & 0b00000000011110000) == 0b10000);
				// port 2 and 3 unused -> 8 highest bits have to be zero
				REQUIRE((esiData.general.value().physicalPort & 0b11111111000000000) == 0);
			}
			THEN("the ESI FMMU category is parsed properly")
			{
				REQUIRE(esiData.fmmu.at(0) == 0x01);
				REQUIRE(esiData.fmmu.at(1) == 0x02);
				REQUIRE(esiData.fmmu.at(2) == 0x03);
				REQUIRE(esiData.fmmu.at(3) == 0xff);
			}
			THEN("the ESI SyncM category is parsed properly")
			{
				REQUIRE(esiData.syncM.at(0).syncManagerType == 0x01);
				REQUIRE(esiData.syncM.at(0).physicalStartAddress == 0x1000);
				REQUIRE(esiData.syncM.at(1).syncManagerType == 0x02);
				REQUIRE(esiData.syncM.at(1).physicalStartAddress == 0x1080);
				REQUIRE(esiData.syncM.at(2).syncManagerType == 0x03);
				REQUIRE(esiData.syncM.at(2).physicalStartAddress == 0x1100);
				REQUIRE(esiData.syncM.at(3).syncManagerType == 0x04);
				REQUIRE(esiData.syncM.at(3).physicalStartAddress == 0x1400);
			}
			THEN("the PDO categories are parsed properly")
			{
				REQUIRE(findPDOEntry(esiData, 0x7000, 0x01).has_value());
				auto nameIdx = findPDOEntry(esiData, 0x7000, 0x01).value().nameIdx;
				REQUIRE(static_cast<int>(esiData.strings.size()) > nameIdx - 1);
				REQUIRE(esiData.strings.at(nameIdx - 1) == "LED and Speaker");
				REQUIRE(findPDOEntry(esiData, 0x6000, 0x02).has_value());
				REQUIRE(findPDOEntry(esiData, 0x6000, 0x02).value().bitLength == 0x08);
				REQUIRE(findPDOEntry(esiData, 0x6000, 0x02).value().dataType == 0x001e);
				REQUIRE(findPDOEntry(esiData, 0x6001, 0x01).has_value());
				nameIdx = findPDOEntry(esiData, 0x6001, 0x01).value().nameIdx;
				REQUIRE(static_cast<int>(esiData.strings.size()) > nameIdx - 1);
				REQUIRE(esiData.strings.at(nameIdx - 1) == "Temperature Value Robodrive");
				REQUIRE(findPDOEntry(esiData, 0x6001, 0x01).value().bitLength == 0x10);
			}
		}
	}
}
