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

#include <memory>

#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/reader/NewestValueView.hpp>

#include <etherkitten/reader/SearchList.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("NewestValueView shows the newest value for DataPoints", "[NewestValueView]")
{
	GIVEN("A SearchList")
	{
		SearchList<ekdatatypes::EtherCATDataType::INTEGER64> list;
		ekdatatypes::TimeStamp time(ekdatatypes::now());
		list.append(4312908743, time);
		ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64> dataPoint{ 4312908743,
			time };

		WHEN("I construct a NewestValueView for it")
		{
			NewestValueView<ekdatatypes::EtherCATDataType::INTEGER64> newestValueView{ list, 0, 0,
				false };
			THEN("It returns the same point if dereferenced")
			{
				REQUIRE(newestValueView.isEmpty() == false);
				REQUIRE((*newestValueView).asString(ekdatatypes::NumberFormat::BINARY)
				    == (dataPoint.asString(ekdatatypes::NumberFormat::BINARY)));
				REQUIRE((*newestValueView).getTime() == dataPoint.getTime());
			}
		}
	}
	GIVEN("A SearchList and a NewestValueView on it")
	{
		SearchList<ekdatatypes::EtherCATDataType::INTEGER64> list;
		list.append(4312908743, ekdatatypes::now());
		NewestValueView<ekdatatypes::EtherCATDataType::INTEGER64> newestValueView{ list, 0, 0,
			false };
		WHEN("I append to the SearchList and derefence the NewestValueView again")
		{
			ekdatatypes::TimeStamp time(ekdatatypes::now());
			list.append(123409874, time);
			ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64> updatedPoint{
				123409874, time
			};
			THEN("The NewestValueView is updated as well")
			{
				REQUIRE((*newestValueView).asString(ekdatatypes::NumberFormat::BINARY)
				    == "0b0000000000000000000000000000000000000111010110110001010111010010");
			}
		}
	}
	GIVEN("A SearchList of unique_ptr (which does not provide all information of its DataPoints "
	      "directly) and a NewestValueView on it")
	{
		SearchList<std::unique_ptr<IOMap>> list;
		std::unique_ptr<IOMap> ioMap(new (20) IOMap{ 20, {} });
		std::array<uint8_t, 20> ioMapContents{
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, // NOLINT
			0x10, 0x11, 0x12, 0x33, 0x74, 0x95, 0x16, 0x17, 0x18, 0x19, // NOLINT
		};
		std::memcpy(ioMap->ioMap, ioMapContents.data(), 20);
		list.append(std::move(ioMap), ekdatatypes::now());
		WHEN("I construct a NewestValueView for it")
		{
			NewestValueView<std::unique_ptr<IOMap>, 1, ekdatatypes::EtherCATDataType::INTEGER64>
			    newestValueView{ list, 0, 64, false };
			THEN("The NewestValueView is updated as well and I get all the information of its "
			     "DataPoint")
			{
				ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64> dataPoint
				    = dynamic_cast<
				        const ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *newestValueView);
				REQUIRE(newestValueView.isEmpty() == false);
				REQUIRE(dataPoint.getValue() == 0x706050403020100);
				// holds a unique pointer so its string should be empty
				REQUIRE(dataPoint.asString(ekdatatypes::NumberFormat::BINARY)
				    == "0b0000011100000110000001010000010000000011000000100000000100000000");
			}
		}
	}
}
