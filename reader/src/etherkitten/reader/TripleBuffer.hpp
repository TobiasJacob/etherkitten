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

#pragma once
/*!
 * \file
 * \brief Defines the TripleBuffer class, which allows passing arbitrary data between threads.
 */

#include <array>
#include <atomic>

#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::reader
{

	/*!
	 * \brief The AnnotatedData struct holds data along with some metadata.
	 * \tparam T the type of data to hold
	 */
	template<typename T>
	struct AnnotatedData
	{
		bool valid{};
		datatypes::TimeStamp time;
		T value;
	};

	/*!
	 * \brief The TripleBuffer class implements a triple buffer in which each buffer holds
	 * length instances of AnnotatedData<T>.
	 * \tparam T the type of data to hold
	 * \tparam length the length of each of the buffers
	 */
	template<typename T, size_t length>
	class TripleBuffer
	{
	public:
		TripleBuffer()
		    : one{}
		    , two{}
		    , three{}
		    , producer(&one)
		    , center(&two)
		    , consumer(&three)
		{
		}

		/*!
		 * \brief Get a slot that the producer can write data into in the current producer buffer.
		 *
		 * If i >= length, the behavior is undefined.
		 * \param i the desired slot in the producer buffer
		 */
		AnnotatedData<T>* getProducerSlot(size_t i) { return producer->data() + i; }

		/*!
		 * \brief Get a slot that the consumer can read data from in the current consumer buffer.
		 *
		 * If i >= length, the behavior is undefined.
		 * \param i the desired slot in the consumer buffer
		 */
		AnnotatedData<T>* getConsumerSlot(size_t i) { return consumer->data() + i; }

		/*!
		 * \brief Swap the producer buffer with the currently inaccessible one.
		 */
		void swapProducer()
		{
			auto tmp = center.exchange(producer, std::memory_order_acq_rel);
			producer = tmp;
		}

		/*!
		 * \brief Swap the consumer buffer with the currently inaccessible one.
		 */
		void swapConsumer()
		{
			auto tmp = center.exchange(consumer, std::memory_order_acq_rel);
			consumer = tmp;
		}

	private:
		std::array<AnnotatedData<T>, length> one, two, three;
		std::array<AnnotatedData<T>, length>* producer;
		std::atomic<std::array<AnnotatedData<T>, length>*> center;
		std::array<AnnotatedData<T>, length>* consumer;
	};
} // namespace etherkitten::reader
