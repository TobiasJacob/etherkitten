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

#include <bitset>
#include <string>
#include <vector>

#include <etherkitten/datatypes/EtherCATTypeStringFormatter.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

SCENARIO("String formatting gives expected results", "[EtherCATTypeStringFormatter]")
{
	GIVEN("An integer")
	{
		const int testInt = 4569;

		WHEN("I format it as a string")
		{
			std::string decStr
			    = EtherCATTypeStringFormatter<int>::asString(testInt, NumberFormat::DECIMAL);
			std::string hexStr
			    = EtherCATTypeStringFormatter<int>::asString(testInt, NumberFormat::HEXADECIMAL);
			std::string binStr
			    = EtherCATTypeStringFormatter<int>::asString(testInt, NumberFormat::BINARY);
			std::string undefStr = EtherCATTypeStringFormatter<int>::asString(
			    testInt, static_cast<NumberFormat>(8)); // NOLINT

			THEN("The string matches the expected conversion")
			{
				REQUIRE(decStr == "4569");
				REQUIRE(hexStr == "0x11d9");
				REQUIRE(binStr == "0b00000000000000000001000111011001");
				REQUIRE(undefStr == "4569");
			}
		}
	}

	GIVEN("A floating point number")
	{
		const float testFloat = 8953.125;

		WHEN("I format it as a string")
		{
			std::string decStr
			    = EtherCATTypeStringFormatter<float>::asString(testFloat, NumberFormat::DECIMAL);
			std::string hexStr = EtherCATTypeStringFormatter<float>::asString(
			    testFloat, NumberFormat::HEXADECIMAL);
			std::string binStr
			    = EtherCATTypeStringFormatter<float>::asString(testFloat, NumberFormat::BINARY);
			std::string undefStr = EtherCATTypeStringFormatter<float>::asString(
			    testFloat, static_cast<NumberFormat>(8)); // NOLINT

			THEN("The string matches the expected conversion")
			{
				REQUIRE(decStr == "8953.12");
				REQUIRE(hexStr == "0x1.17c9p+13");
				REQUIRE(binStr == "0b01000110000010111110010010000000");
				REQUIRE(undefStr == "8953.12");
			}
		}
	}

	GIVEN("A bitset")
	{
		const std::bitset<10> testSet = std::bitset<10>("1010001010");

		WHEN("I format it as a string")
		{
			std::string decStr = EtherCATTypeStringFormatter<std::bitset<10>>::asString( // NOLINT
			    testSet, NumberFormat::DECIMAL);
			std::string hexStr = EtherCATTypeStringFormatter<std::bitset<10>>::asString( // NOLINT
			    testSet, NumberFormat::HEXADECIMAL);
			std::string binStr = EtherCATTypeStringFormatter<std::bitset<10>>::asString( // NOLINT
			    testSet, NumberFormat::BINARY);
			std::string undefStr = EtherCATTypeStringFormatter<std::bitset<10>>::asString( // NOLINT
			    testSet, static_cast<NumberFormat>(8)); // NOLINT

			THEN("The string matches the expected conversion")
			{
				REQUIRE(decStr == "650");
				REQUIRE(hexStr == "0x28a");
				REQUIRE(binStr == "0b1010001010");
				REQUIRE(undefStr == "0b1010001010");
			}
		}
	}

	GIVEN("An octet string")
	{
		const std::vector<uint8_t> testStr{ 128, 54, 32, 4, 83 };

		WHEN("I format it as a string")
		{
			std::string decStr = EtherCATTypeStringFormatter<std::vector<uint8_t>>::asString(
			    testStr, NumberFormat::DECIMAL);
			std::string hexStr = EtherCATTypeStringFormatter<std::vector<uint8_t>>::asString(
			    testStr, NumberFormat::HEXADECIMAL);
			std::string binStr = EtherCATTypeStringFormatter<std::vector<uint8_t>>::asString(
			    testStr, NumberFormat::BINARY);
			std::string undefStr = EtherCATTypeStringFormatter<std::vector<uint8_t>>::asString(
			    testStr, static_cast<NumberFormat>(8)); // NOLINT

			THEN("The string matches the expected conversion")
			{
				REQUIRE(decStr == "128 54 32 4 83");
				REQUIRE(hexStr == "0x80 0x36 0x20 0x4 0x53");
				REQUIRE(binStr == "0b10000000 0b00110110 0b00100000 0b00000100 0b01010011");
				REQUIRE(undefStr == "128 54 32 4 83");
			}
		}
	}

	GIVEN("A byte")
	{
		const EtherCATDataType::BYTE testByte = 128;

		WHEN("I format it as a string")
		{
			std::string decStr = EtherCATTypeStringFormatter<EtherCATDataType::BYTE>::asString(
			    testByte, NumberFormat::DECIMAL);
			std::string hexStr = EtherCATTypeStringFormatter<EtherCATDataType::BYTE>::asString(
			    testByte, NumberFormat::HEXADECIMAL);
			std::string binStr = EtherCATTypeStringFormatter<EtherCATDataType::BYTE>::asString(
			    testByte, NumberFormat::BINARY);

			THEN("The string matches the expected conversion")
			{
				REQUIRE(decStr == "128");
				REQUIRE(hexStr == "0x80");
				REQUIRE(binStr == "0b10000000");
			}
		}
	}

	GIVEN("A string")
	{
		const std::string testStr = "Hello? Is anyone there?";

		WHEN("I format it as a string")
		{
			std::string result = EtherCATTypeStringFormatter<std::string>::asString(
			    testStr, NumberFormat::DECIMAL);

			THEN("The string matches the original") { REQUIRE(result == testStr); }
		}
	}
}
