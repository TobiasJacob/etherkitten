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

#include <etherkitten/reader/TripleBuffer.hpp>

#include <thread>

#include "ThreadContainer.hpp"

using namespace etherkitten::reader;

SCENARIO("The TripleBuffer can swap data back and forth", "[TripleBuffer]")
{
	GIVEN("A TripleBuffer")
	{
		TripleBuffer<int, 4> buf; // NOLINT

		WHEN("I place some data in at the producer side and swap the buffer")
		{
			buf.getProducerSlot(0)->value = 37; // NOLINT
			buf.getProducerSlot(1)->value = 87; // NOLINT
			buf.getProducerSlot(2)->value = 63; // NOLINT
			buf.getProducerSlot(3)->value = 92; // NOLINT

			buf.swapProducer();

			THEN("I get the same data on the consumer side after swapping")
			{
				buf.swapConsumer();

				REQUIRE(buf.getConsumerSlot(0)->value == 37);
				REQUIRE(buf.getConsumerSlot(1)->value == 87);
				REQUIRE(buf.getConsumerSlot(2)->value == 63);
				REQUIRE(buf.getConsumerSlot(3)->value == 92);
			}
		}
	}
}

void produceForTripleBuffer(TripleBuffer<int, 4>* buf)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(2ms);

	for (int i = 1; i <= 1000; ++i)
	{
		buf->getProducerSlot(0)->value = i;
		buf->getProducerSlot(1)->value = i + 1;
		buf->getProducerSlot(2)->value = i + 2;
		buf->getProducerSlot(3)->value = i + 3;

		buf->swapProducer();
	}
}

SCENARIO("The TripleBuffer transports valid data in multi-threaded applications", "[TripleBuffer]")
{
	GIVEN("A TripleBuffer")
	{
		TripleBuffer<int, 4> buf;

		WHEN("I start a thread that produces data on one side and consume data on the other")
		{
			ThreadContainer<TripleBuffer<int, 4>*> t(produceForTripleBuffer, &buf);

			THEN("I get the same data that is produced on the consumer side (though some data may "
			     "be missing)")
			{

				for (int i = 0; i < 1000; ++i)
				{
					buf.swapConsumer();
					if (buf.getConsumerSlot(0)->value != 0)
					{
						int base = buf.getConsumerSlot(0)->value;
						REQUIRE(base >= 1);
						REQUIRE(base <= 1000);
						REQUIRE(buf.getConsumerSlot(1)->value == base + 1);
						REQUIRE(buf.getConsumerSlot(2)->value == base + 2);
						REQUIRE(buf.getConsumerSlot(3)->value == base + 3);
					}
				}
			}
		}
	}
}
