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

#include <etherkitten/reader/CoEUpdateRequest.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("The CoEUpdateRequest stores and provides data for writing and reading of a CoE",
    "[CoEUpdateRequest]")
{
	GIVEN("A CoEObject and the value which shall be written to it")
	{
		ekdatatypes::CoEObject coeObject{ 1, "Some CoE-Object",
			ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
		ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::UNSIGNED32> dataPoint{ 32,
			ekdatatypes::now() };
		std::shared_ptr<ekdatatypes::AbstractDataPoint> value
		    = std::make_shared<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::UNSIGNED32>>(
		        dataPoint);

		WHEN("I create a CoEUpdateRequest")
		{
			CoEUpdateRequest request{ coeObject, value, false };
			THEN("I can retrieve the CoE object")
			{
				REQUIRE(request.getObject()->getIndex() == coeObject.getIndex());
				REQUIRE(request.getObject()->getName() == coeObject.getName());
				REQUIRE(request.getObject()->getSlaveID() == coeObject.getSlaveID());
				REQUIRE(request.getObject()->getSubIndex() == coeObject.getSubIndex());
				REQUIRE(request.getObject()->getType() == coeObject.getType());
				REQUIRE(request.getObject()->isReadableInOp() == coeObject.isReadableInOp());
				REQUIRE(
				    request.getObject()->isReadableInSafeOp() == coeObject.isReadableInSafeOp());
				REQUIRE(request.getObject()->isWritableInOp() == coeObject.isWritableInOp());
				REQUIRE(
				    request.getObject()->isWritableInSafeOp() == coeObject.isWritableInSafeOp());
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
			THEN("I can set that the request is processed and check if it was processed")
			{
				REQUIRE(!request.isProcessed());
				request.setProcessed();
				REQUIRE(request.isProcessed());
			}
			THEN("I can set that the request failed and check if it failed")
			{
				REQUIRE(!request.hasFailed());
				request.setFailed();
				REQUIRE(request.hasFailed());
			}
			THEN("I can verify that this is a write request") { REQUIRE(!request.isReadRequest()); }
		}
	}
}
