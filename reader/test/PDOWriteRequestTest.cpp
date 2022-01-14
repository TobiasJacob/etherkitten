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
#include <chrono>
#include <test/test_globals.hpp>

#include <etherkitten/reader/PDOWriteRequest.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("The PDOWriteRequest stores and provides data for writing to a PDO", "[PDOWriteRequest]")
{
	GIVEN("A PDO and the value which shall be written to it")
	{
		ekdatatypes::PDO pdo{ 0, "", ekdatatypes::EtherCATDataTypeEnum::UNSIGNED32, 0,
			ekdatatypes::PDODirection::INPUT };
		ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::UNSIGNED32> dataPoint{ 32,
			ekdatatypes::now() };
		std::unique_ptr<ekdatatypes::AbstractDataPoint> value
		    = std::make_unique<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::UNSIGNED32>>(
		        dataPoint);

		WHEN("I create a PDOWriteRequest")
		{
			PDOWriteRequest request{ pdo, std::move(value) };
			THEN("I can retrieve the PDO object")
			{
				REQUIRE(request.getPDO().getName() == pdo.getName());
				REQUIRE(request.getPDO().getDirection() == pdo.getDirection());
				REQUIRE(request.getPDO().getIndex() == pdo.getIndex());
				REQUIRE(request.getPDO().getSlaveID() == pdo.getSlaveID());
				REQUIRE(request.getPDO().getType() == pdo.getType());
			}
			THEN("I can retrieve the value which shall be written")
			{
				REQUIRE(request.getValue()->getTime() == dataPoint.getTime());
				REQUIRE(
				    dynamic_cast<
				        const ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::UNSIGNED32>&>(
				        *request.getValue())
				        .getValue()
				    == dataPoint.getValue());
			}
		}
	}
}
