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

#include <etherkitten/reader/CoENewestValueView.hpp>

#include <memory>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#include <etherkitten/reader/SearchList.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("CoENewestValueView shows the newest value for DataPoints", "[CoENewestValueView]")
{
	GIVEN("A SearchList")
	{
		std::shared_ptr<std::shared_ptr<ekdatatypes::AbstractDataPoint>> dataPtr
		    = std::make_shared<std::shared_ptr<ekdatatypes::AbstractDataPoint>>(
		        std::make_shared<ekdatatypes::DataPoint<uint64_t>>(4312908743, ekdatatypes::now()));
		WHEN("I construct a CoENewestValueView for it")
		{
			CoENewestValueView newestValueView{ dataPtr };
			THEN("It returns the same point if dereferenced")
			{
				REQUIRE(newestValueView.isEmpty() == false);
				REQUIRE((*newestValueView).asString(ekdatatypes::NumberFormat::BINARY)
				    == (**dataPtr).asString(ekdatatypes::NumberFormat::BINARY));
				REQUIRE((*newestValueView).getTime() == (**dataPtr).getTime());
			}
		}
	}
	GIVEN("A SearchList and a CoENewestValueView on it")
	{
		std::shared_ptr<std::shared_ptr<ekdatatypes::AbstractDataPoint>> dataPtr
		    = std::make_shared<std::shared_ptr<ekdatatypes::AbstractDataPoint>>(
		        std::make_shared<ekdatatypes::DataPoint<uint64_t>>(4312908743, ekdatatypes::now()));
		CoENewestValueView newestValueView{ dataPtr };
		WHEN("I update the original DataPoint")
		{
			dataPtr->reset(new ekdatatypes::DataPoint<uint64_t>(123409874, ekdatatypes::now()));
			THEN("The CoENewestValueView is updated as well")
			{
				REQUIRE((*newestValueView).asString(ekdatatypes::NumberFormat::BINARY)
				    == "0b0000000000000000000000000000000000000111010110110001010111010010");
			}
		}
	}
	GIVEN("Nothing")
	{
		WHEN("I construct an empty NewestValueView")
		{
			std::shared_ptr<ekdatatypes::DataPoint<uint64_t>> emptyPtr;
			CoENewestValueView newestValueView{
				std::make_shared<std::shared_ptr<ekdatatypes::AbstractDataPoint>>(emptyPtr)
			};
			THEN("The newest value view reports that it is empty")
			{
				REQUIRE(newestValueView.isEmpty());
			}
		}
	}
}
