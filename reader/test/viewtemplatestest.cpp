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

#include <etherkitten/reader/viewtemplates.hpp>

#include <memory>
#include <unordered_map>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

template<typename T>
const T testValue;

// clang-format off
template<> const int testValue<int> = 57;
template<> const double testValue<double> = 1237.6584;
template<> const uint64_t testValue<uint64_t> = 0x1010101010101010;
template<> const std::bitset<7> testValue<std::bitset<7>> = 0b1011110;
// clang-format on

template<typename T>
const ekdatatypes::EtherCATDataTypeEnum enumValue{};

// clang-format off
template<> const ekdatatypes::EtherCATDataTypeEnum enumValue<int>
	= ekdatatypes::EtherCATDataTypeEnum::INTEGER32;
template<> const ekdatatypes::EtherCATDataTypeEnum enumValue<double>
	= ekdatatypes::EtherCATDataTypeEnum::REAL64;
template<> const ekdatatypes::EtherCATDataTypeEnum enumValue<uint64_t>
	= ekdatatypes::EtherCATDataTypeEnum::UNSIGNED64;
template<> const ekdatatypes::EtherCATDataTypeEnum enumValue<std::bitset<7>>
	= ekdatatypes::EtherCATDataTypeEnum::BIT7;
// clang-format on

TEMPLATE_TEST_CASE("The NewestValueViewRetriever hands out valid NewestValueViews", // NOLINT
    "[NewestValueViewRetriever]", int, uint64_t)
{
	GIVEN("A SearchList containing some data")
	{
		SearchList<TestType> list;
		list.append(testValue<TestType>, ekdatatypes::now());

		WHEN("I get a NewestValueView on the SearchList")
		{
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> view
			    = ekdatatypes::dataTypeMap<bReader::NewestValueViewRetriever, TestType,
				  bReader::SizeT2Type<1>>.at(
			        enumValue<TestType>)(list, 0, 0);

			THEN("The view returns the value one would expect")
			{
				REQUIRE(dynamic_cast<const ekdatatypes::DataPoint<TestType>&>(**view).getValue()
				    == testValue<TestType>);
			}
		}
	}
}

TEMPLATE_TEST_CASE( // NOLINT
    "The DataViewRetriever hands out valid DataViews", "[DataViewRetriever]", int, uint64_t)
{
	GIVEN("A SearchList containing some data")
	{
		SearchList<TestType> list;
		list.append(testValue<TestType>, ekdatatypes::now());

		WHEN("I get a DataView on the SearchList")
		{
			std::shared_ptr<ekdatatypes::AbstractDataView> view
			    = ekdatatypes::dataTypeMap<bReader::DataViewRetriever, TestType,
				  bReader::SizeT2Type<1>>.at(
			        enumValue<TestType>)(list, { ekdatatypes::TimeStamp(0s), 0s }, 0, 0);

			THEN("The view returns the value one would expect")
			{
				REQUIRE(*dynamic_cast<const DataView<TestType>&>(*view) == testValue<TestType>);
			}
		}
	}
}

SCENARIO(
    "makeRegisterView returns appropriate views", "[NewestValueViewRetriever],[DataViewRetriever]")
{
	GIVEN("A map from slave addresses to a map of registers to SearchLists")
	{
		const ekdatatypes::RegisterEnum reg = ekdatatypes::RegisterEnum::FRAME_ERROR_COUNTER_PORT_0;
		std::unordered_map<uint16_t,
		    std::unordered_map<ekdatatypes::RegisterEnum, bReader::RegTypesVariant<1>>>
		    map;
		map[1].emplace(reg, std::in_place_type<SearchList<uint8_t>>);
		std::get<0>(map[1][reg]).append(67, ekdatatypes::now()); // NOLINT

		WHEN("I get a DataView on a SearchList")
		{
			std::shared_ptr<ekdatatypes::AbstractDataView> view
			    = bReader::makeRegisterView<std::shared_ptr<ekdatatypes::AbstractDataView>,
			        bReader::SizeT2Type<1>>(
			        map, ekdatatypes::Register(1, reg), { ekdatatypes::TimeStamp(0s), 0s }, 1);

			THEN("The view returns the value one would expect")
			{
				REQUIRE(*dynamic_cast<const DataView<uint8_t>&>(*view) == 67);
			}
		}

		WHEN("I get a NewestValueView on a SearchList")
		{
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> view
			    = bReader::makeRegisterView<std::unique_ptr<ekdatatypes::AbstractNewestValueView>,
			        bReader::SizeT2Type<1>>(
			        map, ekdatatypes::Register(1, reg), { ekdatatypes::TimeStamp(0s), 0s }, 1);

			THEN("The view returns the value one would expect")
			{
				REQUIRE(
				    dynamic_cast<const ekdatatypes::DataPoint<uint8_t>&>(**view).getValue() == 67);
			}
		}
	}
}
