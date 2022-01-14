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

#include <etherkitten/datatypes/EtherCATTypeParser.hpp>
#include <etherkitten/datatypes/EtherCATTypeStringFormatter.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::datatypes;

SCENARIO("String parsing gives expected results", "[EtherCATTypeParser]")
{
	GIVEN("A formatted integer as a string")
	{
		std::string str = GENERATE("4569", "0x11d9", "0b00000000000000000001000111011001");
		INFO("The formatted string is " + str);

		WHEN("I parse it")
		{
			int parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<int>::parse(str));

			THEN("The parsed value matches the integer") { REQUIRE(parsed == 4569); }
		}
	}

	GIVEN("A negative integer formatted as a decimal string")
	{
		std::string str = "-123098531";

		WHEN("I parse it")
		{
			double parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<double>::parse(str));

			THEN("The parsed value matches the integer") { REQUIRE(parsed == -123098531); }
		}
	}

	GIVEN("A formatted large unsigned int as a string")
	{
		// 2 ^ 40, large enough to only be representable by an unsigned 64-bit int
		std::string str = GENERATE(
		    "1099511627776", "0x10000000000", "0b10000000000000000000000000000000000000000");
		WHEN("I parse it")
		{
			uint64_t parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<uint64_t>::parse(str));
			THEN("The parsed value matches the integer") { REQUIRE(parsed == 1099511627776); }
		}
	}

	GIVEN("A formatted float as a string")
	{
		std::string str = GENERATE("8953.12", "0x1.17c9p+13", "0b01000110000010111110010010000000");
		INFO("The formatted string is " + str);

		WHEN("I parse it")
		{
			float parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<float>::parse(str));

			THEN("The parsed value matches the floating point number")
			{
				REQUIRE_THAT(parsed, Catch::Matchers::WithinRel(8953.12, 0.00001));
			}
		}
	}

	GIVEN("A negative floating point number formatted as a decimal string")
	{
		std::string str = "-123098531.1239812";

		WHEN("I parse it")
		{
			double parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<double>::parse(str));

			THEN("The parsed value matches the floating point number")
			{
				REQUIRE_THAT(parsed, Catch::Matchers::WithinRel(-123098531.123981, 0.0000000001));
			}
		}
	}

	GIVEN("A formatted floating point number (that does not fit into single precision) as a string")
	{
		// 2 ^ 140, well above representable by a single precision float
		std::string str = GENERATE("1393796574908163946345982392040522594123776",
		    "0x100000000000000000000000000000000000",
		    "0b0100100010110000000000000000000000000000000000000000000000000000");
		INFO("The formatted string is " + str);

		WHEN("I parse it")
		{
			double parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<double>::parse(str));
			THEN("The parsed value matches the floating point number")
			{
				REQUIRE_THAT(parsed,
				    Catch::Matchers::WithinRel(
				        1393796574908163946345982392040522594123776.0, 0.0000000001));
			}
		}
	}

	GIVEN("A bitset formatted as a string")
	{
		std::string str = GENERATE("650", "0x28a", "0b1010001010");
		WHEN("I parse it")
		{
			std::bitset<10> parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<std::bitset<10>>::parse(str));
			THEN("The parsed value matches the bitset") { REQUIRE(parsed == std::bitset<10>(650)); }
		}
	}

	GIVEN("An octet string formatted as a string (yes, that is correct)")
	{
		std::string str = GENERATE("128 54 32 4 83", "0x80 0x36 0x20 0x4 0x53",
		    "0b10000000 0b00110110 0b00100000 0b00000100 0b01010011");
		WHEN("I parse it")
		{
			std::vector<uint8_t> parsed;
			REQUIRE_NOTHROW(parsed = EtherCATTypeParser<std::vector<uint8_t>>::parse(str));
			THEN("The parsed value matches the octet string")
			{
				std::vector<uint8_t> octetStr = { 128, 54, 32, 4, 83 };
				REQUIRE(octetStr.size() == parsed.size());
				for (size_t i = 0; i < octetStr.size(); ++i)
				{
					REQUIRE(parsed[i] == octetStr[i]);
				}
			}
		}
	}

	GIVEN("An integer that is too large to fit in an int")
	{
		std::string str = "0xFFFFFFFFFF";
		WHEN("I parse it")
		{
			THEN("A range exception is thrown")
			{
				REQUIRE_THROWS_WITH(EtherCATTypeParser<int>::parse(str),
				    Catch::Matchers::Contains("has to be in range"));
			}
		}
	}

	GIVEN("An integer that is too large to fit in an unsigned char")
	{
		std::string str = "0xFFFF";
		WHEN("I parse it")
		{
			THEN("A range exception is thrown")
			{
				REQUIRE_THROWS_WITH(EtherCATTypeParser<uint8_t>::parse(str),
				    Catch::Matchers::Contains("has to be in range"));
			}
		}
	}

	GIVEN("A binary number that is not 32 or 64 bits long")
	{
		std::string str = "0b10101";
		WHEN("I parse it as a floating point number")
		{
			THEN("A TypeParserException is thrown")
			{
				REQUIRE_THROWS_WITH(EtherCATTypeParser<float>::parse(str),
				    Catch::Matchers::Contains("illegal binary representation"));
			}
		}
	}

	GIVEN("A string and a completely new type")
	{
		struct CrazyThing
		{
		};
		std::string str = "I love democracy. I love the republic.";
		WHEN("I parse it as that type")
		{
			THEN("A TypeParserException is thrown")
			{
				REQUIRE_THROWS_WITH(EtherCATTypeParser<CrazyThing>::parse(str),
				    Catch::Matchers::Contains("unsupported type"));
			}
		}
	}

	GIVEN("An octet string with one octet that is too large")
	{
		std::string str = "75 86 23 8657 08";
		WHEN("I parse it as an octet string")
		{
			THEN("A TypeParserException is thrown")
			{
				REQUIRE_THROWS_WITH(EtherCATTypeParser<std::vector<uint8_t>>::parse(str),
				    Catch::Matchers::Contains("An error occurred")
				        && Catch::Matchers::Contains("3"));
			}
		}
	}
}

SCENARIO("Roundtrips for EtherCATType string parsing and formatting are possible",
    "[EtherCATTypeParser][EtherCATTypeStringFormatter]")
{
	GIVEN("An integer")
	{
		int64_t integer = 421987429;
		WHEN("I format it")
		{
			NumberFormat numberFormat
			    = GENERATE(NumberFormat::BINARY, NumberFormat::DECIMAL, NumberFormat::HEXADECIMAL);
			std::string formattedString
			    = EtherCATTypeStringFormatter<int64_t>::asString(integer, numberFormat);
			THEN("I can parse it again and it is the same as before")
			{
				int64_t parsed;
				REQUIRE_NOTHROW(parsed = EtherCATTypeParser<int64_t>::parse(formattedString));
				REQUIRE(parsed == integer);
			}
		}
	}

	GIVEN("A floating point number")
	{
		double fpNumber = 421987429.321;
		WHEN("I format it")
		{
			NumberFormat numberFormat
			    = GENERATE(NumberFormat::BINARY, NumberFormat::DECIMAL, NumberFormat::HEXADECIMAL);
			std::string formattedString
			    = EtherCATTypeStringFormatter<double>::asString(fpNumber, numberFormat);
			THEN("I can parse it again and it is the same as before")
			{
				double parsed;
				REQUIRE_NOTHROW(parsed = EtherCATTypeParser<double>::parse(formattedString));
				REQUIRE_THAT(parsed, Catch::Matchers::WithinRel(fpNumber, 0.00001));
			}
		}
	}

	GIVEN("A bitset")
	{
		std::bitset<24> bitSet = 5395026;
		WHEN("I format it")
		{
			NumberFormat numberFormat
			    = GENERATE(NumberFormat::BINARY, NumberFormat::DECIMAL, NumberFormat::HEXADECIMAL);
			std::string formattedString
			    = EtherCATTypeStringFormatter<std::bitset<24>>::asString(bitSet, numberFormat);
			THEN("I can parse it again and it is the same as before")
			{
				std::bitset<24> parsed;
				REQUIRE_NOTHROW(
				    parsed = EtherCATTypeParser<std::bitset<24>>::parse(formattedString));
				REQUIRE(parsed == bitSet);
			}
		}
	}

	GIVEN("An octet string")
	{
		std::vector<uint8_t> octetStr = { 0xff, 0xab, 0xbc, 0xbe, 0xef };
		WHEN("I format it")
		{
			NumberFormat numberFormat
			    = GENERATE(NumberFormat::BINARY, NumberFormat::DECIMAL, NumberFormat::HEXADECIMAL);
			std::string formattedString
			    = EtherCATTypeStringFormatter<std::vector<uint8_t>>::asString(
			        octetStr, numberFormat);
			THEN("I can parse it again and it is the same as before")
			{
				std::vector<uint8_t> parsed;
				REQUIRE_NOTHROW(
				    parsed = EtherCATTypeParser<std::vector<uint8_t>>::parse(formattedString));
				REQUIRE(octetStr.size() == parsed.size());
				for (size_t i = 0; i < octetStr.size(); ++i)
				{
					REQUIRE(octetStr[i] == parsed[i]);
				}
			}
		}
	}
}
