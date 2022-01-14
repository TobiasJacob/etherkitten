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

#include <etherkitten/reader/ErrorRingBuffer.hpp>

// clazy:excludeall=non-pod-global-static

using namespace etherkitten::reader;
namespace ekdatatypes = etherkitten::datatypes;

SCENARIO("ErrorRingBuffer behaves like a normal RingBuffer (i.e. FIFO)", "[ErrorRingBuffer]")
{
	GIVEN("An ErrorRingBuffer with the default capacity")
	{
		ErrorRingBuffer<int> buffer{};
		WHEN("I add less elements than the capacity")
		{
			int bufferCapacity = buffer.getCapacity();
			for (int i = 0; i < bufferCapacity - 1; ++i)
			{
				buffer.add(i);
			}
			THEN("The Buffer is not empty") { REQUIRE(!buffer.isEmpty()); }
			THEN("I can access all of these elements and the oldest is the first and the newest "
			     "the last inserted")
			{
				REQUIRE(buffer.getOldestElement() == 0);
				for (int i = 0; i <= bufferCapacity - 2; ++i)
				{
					REQUIRE(buffer.at(i) == i);
				}
				REQUIRE(buffer.getNewestElement() == bufferCapacity - 2);
			}
			THEN("The size of the buffer is still less than the capacity")
			{
				REQUIRE(buffer.getSize() < buffer.getCapacity());
			}
		}
		WHEN("I addd more elements than the capacity")
		{
			int bufferCapacity = buffer.getCapacity();
			for (int i = 0; i <= bufferCapacity + 3; ++i)
			{
				buffer.add(i);
			}
			THEN("The values are overwritten in FIFO style")
			{
				REQUIRE(buffer.getOldestElement() == 4);
				for (int i = 0; i <= 3; ++i)
				{
					REQUIRE(buffer.at(i) == i + bufferCapacity);
				}
				for (int i = 4; i < bufferCapacity; ++i)
				{
					REQUIRE(buffer.at(i) == i);
				}
				REQUIRE(buffer.getNewestElement() == bufferCapacity + 3);
			}
		}
	}
}
