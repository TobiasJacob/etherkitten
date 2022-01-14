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

#include <memory>
#include <thread>
#include <vector>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/time.hpp>
#include <etherkitten/reader/LLNode.hpp>
#include <etherkitten/reader/SearchList.hpp>

#include "ThreadContainer.hpp"

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("A SearchList<int> behaves like a linked list in a single thread", "[SearchList]")
{
	GIVEN("A SearchList<int> with some data points in it and a list of values with times")
	{
		SearchList<int> searchList;
		std::vector<int> values;
		ekdatatypes::TimeStamp startTime{ ekdatatypes::now() };
		// 0.5 seconds
		ekdatatypes::TimeStep timeStep{ 500000 };
		std::vector<ekdatatypes::TimeStamp> times;
		for (int i = 0; i < 10; ++i)
		{
			values.push_back(i + 1);
			times.push_back(startTime + i * timeStep);
			searchList.append(values[i], times[i]);
		}
		WHEN("I append values and times and get a DataView of the first TimeStamp")
		{
			// 1.5 = 3 * 0.5 seconds, so a step size of three values
			ekdatatypes::TimeSeries timeSeries{ times[0], ekdatatypes::TimeStep(1500000) };
			std::shared_ptr<DataView<int>> dataView = searchList.getView(timeSeries, false);

			THEN("The DataView points to the first Node of the SearchList")
			{
				REQUIRE(dataView->getTime() == times[0]);
				REQUIRE(dataView->asDouble() == values[0]);
				REQUIRE(dataView->hasNext() == true);
			}

			THEN("The DataView skips values as specified")
			{
				for (int i = 3; i < 10; i += 3)
				{
					++(*dataView);
					REQUIRE(dataView->getTime() == times[i]);
					REQUIRE(dataView->asDouble() == values[i]);
				}
			}
		}

		WHEN("I request a DataView of the third node and delete the 5 oldest nodes")
		{
			ekdatatypes::TimeSeries timeSeries{ times[2], ekdatatypes::TimeStep(0) };
			std::shared_ptr<DataView<int>> dataView = searchList.getView(timeSeries, false);
			searchList.removeOldest(5);

			THEN("Just the two first nodes of the DataView are deleted")
			{
				ekdatatypes::TimeSeries timeSeries{ times[0], ekdatatypes::TimeStep(0) };
				auto newDataView = searchList.getView(timeSeries, false);
				REQUIRE((*newDataView).asDouble() == (*dataView).asDouble());
				REQUIRE((*newDataView).getTime() == (*dataView).getTime());
			}
		}
	}
}

void appendValues(SearchList<int>* list)
{
	std::vector<ekdatatypes::TimeStamp> times;
	for (int i = 0; i < 10; ++i)
	{
		times.push_back(ekdatatypes::now());
		list->append(i + 2, times.back());
		std::this_thread::sleep_for(500ns);
	}
}

void removeValues(SearchList<int>* list)
{
	size_t i = 0;
	while (i < 99)
	{
		i += list->removeOldest(1);
		std::this_thread::sleep_for(500ns);
	}
}

SCENARIO(
    "A SearchList<int> is well-behaved in a multi-threaded environment", "[SearchList],[parallel]")
{
	GIVEN("A SearchList<int> with a single data point")
	{
		SearchList<int> searchList;
		ekdatatypes::TimeStamp startTime{ ekdatatypes::now() };
		searchList.append(1, startTime);
		WHEN("I add data points in a different thread")
		{
			ThreadContainer<SearchList<int>*> thread{ appendValues, &searchList };
			double lastValue = 0;
			THEN("I have a consistent DataView in the main thread")
			{
				ekdatatypes::TimeSeries timeSeries{ ekdatatypes::TimeStamp::min(),
					ekdatatypes::TimeStep(0) };
				auto dataView = searchList.getView(timeSeries, false);
				while (lastValue < 10)
				{
					if (!dataView->hasNext())
					{
						std::this_thread::sleep_for(100ns);
						continue;
					}
					double value = dataView->asDouble();
					REQUIRE_THAT(value, Catch::Matchers::WithinRel(lastValue + 1, 0.0001));
					lastValue = value;
					++(*dataView);
				}
			}
		}
	}

	GIVEN("A SearchList<int> with multiple data points")
	{
		SearchList<int> searchList;
		ekdatatypes::TimeStamp startTime{ ekdatatypes::now() };
		// 0.5 seconds
		ekdatatypes::TimeStep timeStep{ 100000 };
		std::vector<ekdatatypes::TimeStamp> times;
		for (int i = 0; i < 100; ++i)
		{
			times.push_back(startTime + i * timeStep);
			searchList.append(i + 1, times[i]);
		}
		WHEN("I remove data points in a different thread")
		{
			ThreadContainer<SearchList<int>*> thread{ removeValues, &searchList };
			THEN("I can safely read DataViews on the SearchList, even if I immediately destroy "
			     "them afterwards")
			{
				double value = 0;
				ekdatatypes::TimeSeries timeSeries{ ekdatatypes::TimeStamp::min(),
					ekdatatypes::TimeStep(0) };
				do
				{
					std::shared_ptr<DataView<int>> dataView = searchList.getView(timeSeries, false);
					std::this_thread::sleep_for(500ns);
					REQUIRE_NOTHROW(value = dataView->asDouble());
					// vvv the shared_ptr will be released here vvv
				} while (value < 99.9);
			}
		}
	}
}
