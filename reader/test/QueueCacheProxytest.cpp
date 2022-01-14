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

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/reader/BusQueues.hpp>
#include <etherkitten/reader/NewestValueView.hpp>
#include <etherkitten/reader/QueueCacheProxy.hpp>
#include <memory>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

bool isRequestedPDO;
bool isRequestedRegReset;
bool isRequestedHalt;

class BusQueuesMock : public MessageQueues
{
public:
	BusQueuesMock() { produceErrorMessage(); }

	bool postCoEUpdateRequest(const ekdatatypes::CoEObject& object,
	    std::shared_ptr<ekdatatypes::AbstractDataPoint> value, bool readRequest) override
	{
		if (!readRequest)
		{
			return false;
		}

		if (object.getType() == ekdatatypes::EtherCATDataTypeEnum::REAL64)
		{
			ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::REAL64>& newValue
			    = dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::REAL64>&>(
			        *value);
			newValue = ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::REAL64>(
			    42.0, ekdatatypes::now());
		}
		else
		{
			++coeValueFactor;
			ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>& newValue
			    = dynamic_cast<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(
			        *value);
			newValue = ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>(
			    coeValueFactor * 5321, ekdatatypes::now());
		}
		return true;
	}

	void postPDOWriteRequest(
	    const ekdatatypes::PDO& pdo, std::unique_ptr<ekdatatypes::AbstractDataPoint> value) override
	{
		(void)pdo;
		(void)value;
		isRequestedPDO = true;
	}

	std::shared_ptr<DataView<ekdatatypes::ErrorMessage>> getErrors() override
	{
		ekdatatypes::TimeSeries timeSeries{ ekdatatypes::TimeStamp::min(),
			ekdatatypes::TimeStep{ 0 } };
		return errorMessages.getView(timeSeries, false);
	}

	void postErrorRegisterResetRequest(unsigned int slave) override
	{
		(void)slave;
		isRequestedRegReset = true;
	}

	void produceErrorMessage()
	{
		ekdatatypes::TimeStamp timeStamp = ekdatatypes::now();
		errorMessages.append(ekdatatypes::ErrorMessage{ "You are testing my patience... #"
		                             + std::to_string(++errorMessageID),
		                         ekdatatypes::ErrorSeverity::MEDIUM },
		    timeStamp);
	}

	ekdatatypes::CoEObject* requestedCoEObject{ nullptr };
	ekdatatypes::PDO* requestedPDO{ nullptr };
	unsigned int slaveResetRequested{ std::numeric_limits<unsigned int>::max() };

private:
	ekdatatypes::EtherCATDataType::INTEGER64 coeValueFactor{ 0 };
	SearchList<ekdatatypes::ErrorMessage> errorMessages;
	unsigned int errorMessageID{ 0 };
};

SCENARIO("QueueCacheProxy works generally as intended", "[QueueCacheProxy]")
{
	GIVEN("MessageQueues and a QueueCacheProxy")
	{
		isRequestedPDO = false;
		isRequestedRegReset = false;
		isRequestedHalt = false;
		std::unique_ptr<MessageQueues> busQueuesPtr = std::make_unique<BusQueuesMock>();
		BusQueuesMock& busQueues = dynamic_cast<BusQueuesMock&>(*busQueuesPtr);
		QueueCacheProxy queueCacheProxy{ std::move(busQueuesPtr) };
		WHEN("I request a CoEObject write in QueueCacheProxy")
		{
			ekdatatypes::CoEObject coeObject{ 1, "Some CoE-Object",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
			bool success = queueCacheProxy.updateCoEObject(coeObject,
			    std::make_shared<
			        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(),
			    false);
			THEN("The write fails due to the consumer") { REQUIRE(!success); }
		}
		WHEN("I request a PDO write in QueueCacheProxy")
		{
			ekdatatypes::PDO pdo{ 1, "Test", ekdatatypes::EtherCATDataTypeEnum::BOOLEAN, 0,
				ekdatatypes::PDODirection::INPUT };
			queueCacheProxy.setPDOValue(pdo,
			    std::make_unique<
			        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>());
			THEN("The PDO request is handed over to the BusQueues") { REQUIRE(isRequestedPDO); }
		}
		WHEN("I request a Register reset in QueueCacheProxy")
		{
			queueCacheProxy.resetErrorRegisters(1);
			THEN("The PDO request is handed over to the BusQueues")
			{
				REQUIRE(isRequestedRegReset);
			}
		}
		WHEN("I request a NewestValueView before a CoEObject was read")
		{
			ekdatatypes::CoEObject coeObject{ 1, "Some CoE-Object",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> view
			    = queueCacheProxy.getNewest(coeObject);
			THEN("I get an empty NewestValueView") { REQUIRE(view->isEmpty()); }
		}
		WHEN("I request a CoEObject read in QueueCacheProxy")
		{
			ekdatatypes::CoEObject coeObject{ 1, "Some CoE-Object",
				ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };

			queueCacheProxy.updateCoEObject(coeObject,
			    std::make_shared<
			        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(),
			    true);
			THEN("I recieve the correct value in a view created by QueueCacheProxy")
			{
				std::unique_ptr<ekdatatypes::AbstractNewestValueView> newestValueView
				    = queueCacheProxy.getNewest(coeObject);
				std::string newestCoEValue
				    = (**newestValueView).asString(ekdatatypes::NumberFormat::DECIMAL);
				REQUIRE(newestCoEValue == "5321");
				THEN("I can read the same CoEObject again and the view updates")
				{
					queueCacheProxy.updateCoEObject(coeObject,
					    std::make_shared<
					        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(),
					    true);
					newestCoEValue
					    = (**newestValueView).asString(ekdatatypes::NumberFormat::DECIMAL);
					REQUIRE(newestCoEValue == "10642");
				}
				THEN("I can read a different CoEObject and the view on the original still "
				     "works")
				{
					ekdatatypes::CoEObject otherCoEObject{ 1, "Some other CoE-Object",
						ekdatatypes::EtherCATDataTypeEnum::REAL64, 0, 0, 0b110110 };
					queueCacheProxy.updateCoEObject(otherCoEObject,
					    std::make_shared<
					        ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::REAL64>>(),
					    true);
					newestCoEValue
					    = (**newestValueView).asString(ekdatatypes::NumberFormat::DECIMAL);
					REQUIRE(newestCoEValue == "5321");
				}
			}
		}
		WHEN("I request the errors")
		{
			std::shared_ptr<ekdatatypes::ErrorIterator> errorIterator = queueCacheProxy.getErrors();
			THEN("The recieved ErrorIterator behaves like an DataView<ErrorMessage>")
			{
				busQueues.produceErrorMessage();
				// Now there should be two ErrorMessages in the MessageQueues
				REQUIRE(errorIterator->hasNext());
				REQUIRE((**errorIterator).getValue().getMessage()
				    == "You are testing my patience... #1");
				INFO("Advancing one step - there should be no next message")
				++(*errorIterator);
				REQUIRE_FALSE(errorIterator->hasNext());
				REQUIRE((**errorIterator).getValue().getMessage()
				    == "You are testing my patience... #2");
			}
		}
	}
}
