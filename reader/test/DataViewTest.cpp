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

#include "etherkitten/reader/DataView.hpp"
#include <etherkitten/datatypes/time.hpp>
#include <etherkitten/reader/LLNode.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

struct LLContainer
{
	LLNode<int>* head;

	LLContainer(int listMax, ekdatatypes::TimeStep step)
	{
		head = new LLNode<int>{ 0, ekdatatypes::TimeStamp(), nullptr };
		ekdatatypes::TimeStamp time = head->times[0];
		for (int i = 1; i <= listMax; ++i)
		{
			time -= step;
			head = new LLNode<int>{ i, time, head };
		}
	}

	~LLContainer()
	{
		LLNode<int>* current = head;
		while (current != nullptr)
		{
			LLNode<int>* prev = current;
			current = current->next;
			delete prev;
		}
	}
};

SCENARIO("A DataView<int> behaves like an iterator over a linked list")
{
	GIVEN("A DataView pointing to the head of a linked list and containing a step size of 5us")
	{
		LLContainer container(10, ekdatatypes::TimeStep(5));
		DataView<int> dataView({ container.head, 0 }, ekdatatypes::TimeStep(1), false);

		WHEN("I request the current value")
		{
			double value = dataView.asDouble();
			THEN("I get the value of the first linked list node as double")
			{
				REQUIRE(value == container.head->values[0]);
			}
		}
		WHEN("I request the current time")
		{
			ekdatatypes::TimeStamp time = dataView.getTime();
			THEN("I get the time of the first linked list node")
			{
				REQUIRE(time == container.head->times[0]);
			}
		}
		WHEN("I request the current value")
		{
			int point = *dataView;
			THEN("I get a reference to the value of the first linked list node")
			{
				REQUIRE(point == 10);
			}
		}
		WHEN("I check if the DataView can be incremented")
		{
			bool hasNext = dataView.hasNext();
			THEN("I get true") { REQUIRE(hasNext == true); }
		}
		WHEN("I increment the iterator")
		{
			++dataView;
			THEN("I get a reference to the DataView pointing to the node which is 5us after the "
			     "first")
			{
				REQUIRE(dataView.getTime() >= container.head->times[0]);
			}
		}
	}
	GIVEN("A DataView pointing to the head of a linked list with two nodes where the step of the "
	      "DataView is bigger than the difference of time to the second node")
	{
		LLContainer container(1, ekdatatypes::TimeStep(1));
		DataView<int> dataView({ container.head, 0 }, ekdatatypes::TimeStep(10), false);

		WHEN("I check if the DataView can be incremented")
		{
			bool hasNext = dataView.hasNext();
			THEN("I get false") { REQUIRE(hasNext == false); }
		}
		WHEN("I increment the iterator")
		{
			++dataView;
			THEN("I get a reference to the DataView pointing to the last node of the linked list")
			{
				LLNode<int>* tail(container.head->next);
				REQUIRE(dataView.asDouble() == tail->values[0]);
				REQUIRE(dataView.getTime() == tail->times[0]);
			}
		}
	}
}
