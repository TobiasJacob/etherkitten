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
 * \brief Defines the LLNode, a node of an unrolled linked list.
 */

#include <array>
#include <atomic>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief A node of an unrolled singly linked list which stores values and timestamps.
	 *
	 * This node can store any number of elements (that's why it's "unrolled") based on Size.
	 * Type must be move-constructible.
	 * \tparam Type the type of value to store in the LLNode.
	 * \tparam Size the number of elements to store in one node of the list.
	 */
	template<class Type, size_t Size = 1>
	struct LLNode
	{
	public:
		/*!
		 * \brief The values stored in this node.
		 */
		std::array<Type, Size> values;

		/*!
		 * \brief The TimeStamps stored in this node.
		 */
		std::array<datatypes::TimeStamp, Size> times;

		/*!
		 * \brief The next node in the list.
		 */
		std::atomic<LLNode<Type, Size>*> next{ nullptr };

		/*!
		 * \brief How many array slots are occupied in this node.
		 */
		std::atomic_size_t count;

		/*!
		 * \brief LLNode Construct a new node with the given element, timestamp, and succesor.
		 * \param element the element to store at position 0 in the node
		 * \param next the node that should follow this one in the list
		 */
		LLNode(Type value, datatypes::TimeStamp time, LLNode* next)
		    : values({ std::move(value) })
		    , times({ time })
		    , next(next)
		    , count(1)
		{
		}

		/*!
		 * \brief LLNode Copy construct a new node from another.
		 *
		 * This constructor does not copy the list that other is the head of.
		 * The new node will thus point at the same list as the old one.
		 * \param other the node to copy from
		 */
		LLNode(const LLNode<Type, Size>& other)
		    : values(other.values)
		    , times(other.times)
		    , next(other.next.load(std::memory_order_acquire))
		    , count(other.count.load(std::memory_order_acquire))
		{
		}
	};
} // namespace etherkitten::reader
