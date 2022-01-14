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

#include <atomic>
#include <thread>

#include <etherkitten/reader/LogCache.hpp>

#include "ThreadContainer.hpp"

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

const ekdatatypes::CoEObject coe{ 1, "Some CoE-Object",
	ekdatatypes::EtherCATDataTypeEnum::INTEGER64, 0, 0, 0b110110 };

void readLog(LogCache* cache)
{
	std::unique_ptr<ekdatatypes::AbstractDataPoint> point
	    = std::make_unique<ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>>(
	        5, ekdatatypes::now());
	cache->setCoEValue(coe, std::move(point));

	cache->postError(ekdatatypes::ErrorMessage("Failed", ekdatatypes::ErrorSeverity::MEDIUM));
}

SCENARIO("A LogCache holds the read CoEObjects and erros from a log file", "[LogCache],[LogReader]")
{
	GIVEN("A LogCache and a thread posting the read values")
	{
		std::unique_ptr<LogCache> logCache = std::make_unique<LogCache>();
		ThreadContainer<LogCache*> logReader(readLog, logCache.get());
		WHEN("I request the errors")
		{
			std::this_thread::sleep_for(10ms);
			std::shared_ptr<ekdatatypes::ErrorIterator> errors = logCache->getErrors();
			THEN("I get the read erros")
			{
				while (errors->hasNext())
				{
					++(*errors);
				}
				REQUIRE((**errors).getValue().getMessage() == "Failed");
			}
		}
		WHEN("I request a view of a CoEObject")
		{
			std::unique_ptr<ekdatatypes::AbstractNewestValueView> abstractView
			    = logCache->getNewest(coe);
			CoENewestValueView& view = dynamic_cast<CoENewestValueView&>(*abstractView);
			while (view.isEmpty())
			{
				std::this_thread::sleep_for(10ms);
			}
			const ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>& point
			    = dynamic_cast<
			        const ekdatatypes::DataPoint<ekdatatypes::EtherCATDataType::INTEGER64>&>(*view);
			THEN("The view holds the newest value") { REQUIRE(point.getValue() == 5); }
		}
	}
}
