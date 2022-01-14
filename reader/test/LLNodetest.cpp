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

#include <etherkitten/datatypes/time.hpp>
#include <etherkitten/reader/LLNode.hpp>

#include "ThreadContainer.hpp"

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

struct LLContainer
{
	LLNode<int>* head;

	static const int listMax = 10;

	LLContainer()
	{
		head = new LLNode<int>{ 0, ekdatatypes::TimeStamp(), nullptr };

		for (int i = 1; i <= listMax; ++i)
		{
			head = new LLNode<int>{ i, ekdatatypes::TimeStamp(), head };
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

// The linker requires this for... reasons.
const int LLContainer::listMax;

SCENARIO("An LLNode<int> behaves like a linked list in a single thread", "[LLNode]")
{
	GIVEN("A linked list with several nodes")
	{
		LLContainer container;

		WHEN("I dereference the head")
		{
			LLNode<int> head = *(container.head);

			THEN("I get the value at the front")
			{
				REQUIRE(head.values[0] == LLContainer::listMax);
			}
		}
		WHEN("I loop over the list")
		{
			std::vector<int> elements;

			LLNode<int>* current = container.head;
			while (current != nullptr)
			{
				elements.push_back(current->values[0]);
				current = current->next;
			}

			THEN("I get all the values in the list")
			{
				std::vector<int> expected;
				for (int i = LLContainer::listMax; i >= 0; --i)
				{
					expected.push_back(i);
				}
				REQUIRE(elements.size() == 11);
				REQUIRE(elements == expected);
			}
		}
	}
}

void append11ToList(LLContainer* container)
{
	using namespace std::chrono_literals;
	LLNode<int>* current = container->head;
	while (current->next != nullptr)
	{
		current = current->next;
		std::this_thread::sleep_for(500ns);
	}
	current->next = new LLNode<int>{ LLContainer::listMax + 1, ekdatatypes::TimeStamp(), nullptr };
}

SCENARIO("An LLNode<int> allows multiple threads to work on it at once", "[LLNode],[parallel]")
{
	GIVEN("A linked list with several nodes")
	{
		LLContainer container;

		WHEN("I reading from the list while another thread appends to it")
		{
			ThreadContainer<LLContainer*> writer(append11ToList, &container);

			THEN("I read a consistent list in the meantime")
			{
				while (true)
				{
					LLNode<int>* current = container.head;
					int i = LLContainer::listMax;
					while (current->next != nullptr)
					{
						INFO("Current index: " << i)
						REQUIRE(current->values[0] == i);
						current = current->next;
						--i;
					}
					if (i == 0)
					{
						REQUIRE(current->values[0] == i);
					}
					else if (i == -1)
					{
						int expected = LLContainer::listMax + 1;
						REQUIRE(current->values[0] == expected);
						break;
					}
					else
					{
						FAIL("The list reached an inconsistent state");
						break;
					}
				}
			}
		}
	}
}
