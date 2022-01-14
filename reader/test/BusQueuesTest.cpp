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

#include <thread>
#include <vector>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/reader/BusQueues.hpp>

#include "ThreadContainer.hpp"

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

bool isPDOWriteIdentical;
bool areRegistersReset;
bool isBusHalt;

void processRequests(BusQueues* queues)
{
	queues->postError(ekdatatypes::ErrorMessage("Failed1", ekdatatypes::ErrorSeverity::MEDIUM));
	queues->postError(ekdatatypes::ErrorMessage("Failed2", ekdatatypes::ErrorSeverity::LOW));
	queues->postError(ekdatatypes::ErrorMessage("Failed3", ekdatatypes::ErrorSeverity::LOW));
	queues->postError(ekdatatypes::ErrorMessage("Failed4", ekdatatypes::ErrorSeverity::MEDIUM));

	int coeFactor = 0;
	while (!isBusHalt)
	{
		// CoE
		std::shared_ptr<CoEUpdateRequest> coeRequest = queues->getCoERequest();
		if (coeRequest)
		{
			if (coeRequest->isReadRequest())
			{
				++coeFactor;
				ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>& newValue
				    = dynamic_cast<
				        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *(coeRequest->getValue()));
				newValue = ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>(
				    coeFactor * 10, ekdatatypes::now());
				coeRequest->setProcessed();
				queues->postCoERequestReply(std::move(coeRequest));
			}
			else
			{
				coeRequest->setFailed();
				queues->postCoERequestReply(std::move(coeRequest));
			}
		}

		// PDO
		std::shared_ptr<PDOWriteRequest> pdoRequest = queues->getPDORequest();
		if (pdoRequest)
		{
			if (pdoRequest->getPDO().getName() == "Test"
			    && dynamic_cast<
			           const ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
			           *pdoRequest->getValue())
			            .getValue()
			        == 55)
			{
				isPDOWriteIdentical = true;
			}
		}
		// RegisterReset
		if (queues->getRegisterResetRequest())
		{
			areRegistersReset = true;
		}
		std::this_thread::sleep_for(10ms);
	}
}

SCENARIO("A BusQueues allows one thread to produce requests and another to consume them",
    "[BusQueues],[parallel]")
{
	GIVEN("A BusQueues and a thread which consumes the requests and fails on all write "
	      "requests to "
	      "CoE")
	{
		isBusHalt = false;
		isPDOWriteIdentical = false;
		areRegistersReset = false;
		std::unique_ptr<BusQueues> queues = std::make_unique<BusQueues>();
		ThreadContainer<BusQueues*> reader(processRequests, queues.get());

		WHEN("I request to read a CoEObject and wait")
		{
			const ekdatatypes::CoEObject coeObject{ 1, "Some CoE-Object",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
			std::shared_ptr<ekdatatypes::AbstractDataPoint> value = std::make_shared<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>();
			queues->postCoEUpdateRequest(coeObject, value, true);
			std::this_thread::sleep_for(10ms);

			THEN("The request is processed from the consumer and my DataPoint is changed")
			{
				REQUIRE(
				    dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *value)
				        .getValue()
				    == 10);
			}
			// required to stop the timer thread
			isBusHalt = true;
		}
		WHEN("I request several CoEUpdates")
		{
			const ekdatatypes::CoEObject coeObject1{ 1, "Some CoE-Object 1",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
			const ekdatatypes::CoEObject coeObject2{ 2, "Some CoE-Object 2",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
			std::shared_ptr<ekdatatypes::AbstractDataPoint> readValue1 = std::make_shared<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
			    static_cast<ekdatatypes::EtherCATDataType::INTEGER64>(999), ekdatatypes::now());
			std::shared_ptr<ekdatatypes::AbstractDataPoint> writeValue = std::make_shared<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
			    static_cast<ekdatatypes::EtherCATDataType::INTEGER64>(100), ekdatatypes::now());
			std::shared_ptr<ekdatatypes::AbstractDataPoint> readValue2 = std::make_shared<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
			    static_cast<ekdatatypes::EtherCATDataType::INTEGER64>(999), ekdatatypes::now());
			std::shared_ptr<ekdatatypes::AbstractDataPoint> readValue3 = std::make_shared<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
			    static_cast<ekdatatypes::EtherCATDataType::INTEGER64>(999), ekdatatypes::now());
			bool success = true;

			// read 1
			queues->postCoEUpdateRequest(coeObject1, readValue1, true);
			// write
			success = queues->postCoEUpdateRequest(coeObject1, writeValue, false);
			// read 2
			queues->postCoEUpdateRequest(coeObject2, readValue2, true);
			// read 3
			queues->postCoEUpdateRequest(coeObject1, readValue3, true);

			THEN("The requests are handled in the given order")
			{
				std::this_thread::sleep_for(15ms);
				REQUIRE(
				    dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *readValue1)
				        .getValue()
				    == 10);
				REQUIRE(!success);
				REQUIRE(
				    dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *readValue2)
				        .getValue()
				    == 20);
				REQUIRE(
				    dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
				        *readValue3)
				        .getValue()
				    == 30);
			}
			isBusHalt = true;
		}
		WHEN("I request a write to a PDO Object")
		{
			ekdatatypes::PDO pdo = ekdatatypes::PDO(1, "Test",
			    ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, ekdatatypes::PDODirection::INPUT);
			std::unique_ptr<ekdatatypes::AbstractDataPoint> value = std::make_unique<
			    ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
			    static_cast<ekdatatypes::EtherCATDataType::INTEGER64>(55), ekdatatypes::now());
			queues->postPDOWriteRequest(pdo, std::move(value));
			THEN("The consumer consumes the request after some time")
			{
				std::this_thread::sleep_for(15ms);
				REQUIRE(isPDOWriteIdentical == true);
			}
			isBusHalt = true;
		}
		WHEN("I request to reset the registers")
		{
			queues->postErrorRegisterResetRequest(1);
			THEN("The consumer consumes the request after some time")
			{
				std::this_thread::sleep_for(15ms);
				REQUIRE(areRegistersReset == true);
			}
			isBusHalt = true;
		}
		WHEN("I request the errors")
		{
			std::shared_ptr<DataView<ekdatatypes::ErrorMessage>> errors = queues->getErrors();
			THEN("The returned view contains the submitted errors")
			{
				while (errors->isEmpty())
				{
					std::this_thread::sleep_for(15ms);
					if (errors->hasNext())
					{
						++*errors;
					}
				}
				REQUIRE((**errors).getMessage() == "Failed1");
				REQUIRE((**errors).getSeverity() == ekdatatypes::ErrorSeverity::MEDIUM);
				REQUIRE(errors->hasNext());
				++*errors;
				REQUIRE((**errors).getMessage() == "Failed2");
				REQUIRE((**errors).getSeverity() == ekdatatypes::ErrorSeverity::LOW);
				REQUIRE(errors->hasNext());
				++*errors;
				REQUIRE((**errors).getMessage() == "Failed3");
				REQUIRE((**errors).getSeverity() == ekdatatypes::ErrorSeverity::LOW);
				REQUIRE(errors->hasNext());
				++*errors;
				REQUIRE((**errors).getMessage() == "Failed4");
				REQUIRE((**errors).getSeverity() == ekdatatypes::ErrorSeverity::MEDIUM);
			}
			isBusHalt = true;
		}
	}
}
