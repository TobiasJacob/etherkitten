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

#include <array>
#include <bitset>

#include <etherkitten/reader/Converter.hpp>

#include <etherkitten/datatypes/errors.hpp>

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

template<typename T>
const T convTestValue;

// clang-format off
template<> const int convTestValue<int> = 57;
template<> const double convTestValue<double> = 1237.6584;
template<> const uint64_t convTestValue<uint64_t> = 0x1010101010101010;
template<> const std::bitset<23> convTestValue<std::bitset<23>> = 0b10111100101010;
template<> const std::string convTestValue<std::string> = "We'll make him a string he cannot refuse.";
template<> const std::vector<uint8_t> convTestValue<std::vector<uint8_t>> = { 42, 78, 12, 'b', 129 };
template<> const ekdatatypes::ErrorMessage convTestValue<ekdatatypes::ErrorMessage> =
	{ "Just like the simulations.", { 23, 76 }, ekdatatypes::ErrorSeverity::MEDIUM };
// clang-format on

TEMPLATE_TEST_CASE("The Converter can pass values through untouched", "[Converter]", int, // NOLINT
    double, uint64_t, (std::bitset<23>), (std::string), (std::vector<uint8_t>),
    (ekdatatypes::ErrorMessage))
{
	if constexpr (std::is_same<TestType, ekdatatypes::ErrorMessage>())
	{
		TestType result
		    = Converter<TestType, TestType>::shiftAndConvert(convTestValue<TestType>, 0, 0, false);
		REQUIRE(convTestValue<TestType>.getMessage() == result.getMessage());
		REQUIRE(convTestValue<TestType>.getAssociatedSlaves() == result.getAssociatedSlaves());
		REQUIRE(convTestValue<TestType>.getSeverity() == result.getSeverity());
	}
	else
	{
		// clang-format off
		REQUIRE(convTestValue<TestType>
				== Converter<TestType, TestType>::shiftAndConvert(convTestValue<TestType>, 0, 0, false));
		// clang-format on
	}
}

SCENARIO("The Converter can extract shorter types from UNSIGNEDXX types", "[Converter]")
{
	GIVEN("An UNSIGNED64")
	{
		constexpr uint64_t value = 0x1234567890214365;
		WHEN("I convert it to shorter UNSIGNEDXX with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(Converter<uint64_t, uint8_t>::shiftAndConvert(value, 0, 8, true) == 0x65);
				REQUIRE(Converter<uint64_t, uint8_t>::shiftAndConvert(value, 24, 8, true) == 0x90);
				REQUIRE(Converter<uint64_t, uint8_t>::shiftAndConvert(value, 56, 8, true) == 0x12);
				REQUIRE(
				    Converter<uint64_t, uint16_t>::shiftAndConvert(value, 0, 16, true) == 0x4365);
				REQUIRE(Converter<uint64_t, uint32_t>::shiftAndConvert(value, 8, 32, true)
				    == 0x78902143);
			}
		}

		WHEN("I convert it to shorter INTEGERXX with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(Converter<uint64_t, int8_t>::shiftAndConvert(value, 0, 8, true) == 0x65);
				REQUIRE(Converter<uint64_t, int8_t>::shiftAndConvert(value, 24, 8, true) == -0x70);
				REQUIRE(Converter<uint64_t, int8_t>::shiftAndConvert(value, 56, 8, true) == 0x12);
				REQUIRE(
				    Converter<uint64_t, int16_t>::shiftAndConvert(value, 0, 16, true) == 0x4365);
				REQUIRE(Converter<uint64_t, int32_t>::shiftAndConvert(value, 4, 32, true)
				    == -0x76fdebca);
			}
		}

		WHEN("I convert it to shorter bitsets with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(
				    Converter<uint64_t, std::bitset<4>>::shiftAndConvert(value, 0, 4, true) == 0x5);
				REQUIRE(Converter<uint64_t, std::bitset<24>>::shiftAndConvert(value, 24, 24, true)
				    == 0x567890);
				REQUIRE(Converter<uint64_t, std::bitset<48>>::shiftAndConvert(value, 16, 48, true)
				    == 0x123456789021);
				REQUIRE(Converter<uint64_t, std::bitset<7>>::shiftAndConvert(value, 5, 7, true)
				    == 0x1b);
				REQUIRE(
				    Converter<uint64_t, std::bitset<1>>::shiftAndConvert(value, 63, 1, true) == 0);
			}
		}
	}
}

SCENARIO("The Converter can extract types from an IOMap", "[Converter]")
{
	GIVEN("An IOMap")
	{
		std::unique_ptr<IOMap> ioMap(new (20) IOMap{ 20, {} }); // NOLINT
		std::array<uint8_t, 20> ioMapContents{
			// NOLINT
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, // NOLINT
			0x10, 0x11, 0x12, 0x33, 0x74, 0x95, 0x16, 0x17, 0x18, 0x19, // NOLINT
		};
		std::memcpy(ioMap->ioMap, ioMapContents.data(), 20); // NOLINT

		WHEN("I extract an UNSIGNEDXX with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(
				    Converter<IOMap*, uint8_t>::shiftAndConvert(ioMap.get(), 0, 8, true) == 0x00);
				REQUIRE(
				    Converter<IOMap*, uint8_t>::shiftAndConvert(ioMap.get(), 24, 8, true) == 0x03);
				REQUIRE(
				    Converter<IOMap*, uint8_t>::shiftAndConvert(ioMap.get(), 56, 8, true) == 0x07);
				REQUIRE(Converter<IOMap*, uint16_t>::shiftAndConvert(ioMap.get(), 0, 16, true)
				    == 0x0100);
				REQUIRE(Converter<IOMap*, uint32_t>::shiftAndConvert(ioMap.get(), 8, 32, true)
				    == 0x04030201);
			}
		}

		WHEN("I extract an INTEGERXX with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(
				    Converter<IOMap*, int8_t>::shiftAndConvert(ioMap.get(), 0, 8, true) == 0x00);
				REQUIRE(
				    Converter<IOMap*, int8_t>::shiftAndConvert(ioMap.get(), 120, 8, true) == -0x6b);
				REQUIRE(
				    Converter<IOMap*, int8_t>::shiftAndConvert(ioMap.get(), 56, 8, true) == 0x07);
				REQUIRE(Converter<IOMap*, int16_t>::shiftAndConvert(ioMap.get(), 0, 16, true)
				    == 0x0100);
				REQUIRE(Converter<IOMap*, int32_t>::shiftAndConvert(ioMap.get(), 96, 32, true)
				    == -0x6a8bccee);
			}
		}

		WHEN("I extract bitsets with an offset and matching length")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(Converter<IOMap*, std::bitset<4>>::shiftAndConvert(ioMap.get(), 0, 4, true)
				    == 0x0);
				REQUIRE(
				    Converter<IOMap*, std::bitset<24>>::shiftAndConvert(ioMap.get(), 24, 24, true)
				    == 0x050403);
				REQUIRE(
				    Converter<IOMap*, std::bitset<48>>::shiftAndConvert(ioMap.get(), 16, 48, true)
				    == 0x070605040302);
				REQUIRE(Converter<IOMap*, std::bitset<7>>::shiftAndConvert(ioMap.get(), 5, 7, true)
				    == 0x08);
				REQUIRE(Converter<IOMap*, std::bitset<1>>::shiftAndConvert(ioMap.get(), 58, 1, true)
				    == 1);
			}
		}

		WHEN("I extract types that run over 9 bytes")
		{
			THEN("I get the correct value including endianness considerations")
			{
				REQUIRE(Converter<IOMap*, uint64_t>::shiftAndConvert(ioMap.get(), 89, 64, true)
				    == 0x8c0b8b4aba198908);
				REQUIRE(Converter<IOMap*, int64_t>::shiftAndConvert(ioMap.get(), 89, 64, true)
				    == -0x73f474b545e676f8);
			}
		}
	}
}
