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
 * \brief Defines the DataView, an iterator-like class that steps over a list of LLNodes.
 */

#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <variant>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "Converter.hpp"
#include "IOMap.hpp"
#include "LLNode.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Represents the location of a single point of data in a list of LLNodes.
	 * \tparam Type the type of data contained in the list
	 * \tparam NodeSize the size of the LLNodes
	 */
	template<typename Type, size_t NodeSize = 1>
	struct ListLocation
	{
		LLNode<Type, NodeSize>* node;
		size_t index;
	};

	/*!
	 * \brief Implements the AbstractDataView interface for a specific EtherCATDataType.
	 *
	 * Note that Converter<Type, Output> must be a valid instantiation of Converter for
	 * the type combination to compile.
	 * \tparam Type the type that the related DataPoints have
	 * \tparam NodeSize the size of the nodes in the LLNode list
	 * \tparam Output the type the view should output when asked
	 */
	template<class Type, size_t NodeSize = 1, class Output = Type>
	class DataView : public datatypes::AbstractDataView
	{
	public:
		/*!
		 * \brief Create a new DataView that starts at the given node
		 * and advances with the given TimeStep.
		 *
		 * A DataView initialized this way will never return true from isEmpty().
		 * \param location the location to start the DataView on
		 * \param timeStep the TimeStep to advance the DataView with
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 */
		DataView(ListLocation<Type, NodeSize> location, datatypes::TimeStep timeStep,
		    bool flipBytes) // NOLINT
		    : DataView(location, timeStep, 0, 0, flipBytes)
		{
		}

		/*!
		 * \brief Create a new DataView that starts at the given node
		 * and advances with the given TimeStep.
		 *
		 * A DataView initialized this way will never return true from isEmpty().
		 * \param location the location to start the DataView on
		 * \param timeStep the TimeStep to advance the DataView with
		 * \param bitOffset the offset of the desired output in the values
		 * \param bitLength the length of the desired output in the values
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 */
		DataView(ListLocation<Type, NodeSize> location, datatypes::TimeStep timeStep,
		    size_t bitOffset, size_t bitLength, bool flipBytes)
		    : node(location.node)
		    , index(location.index)
		    , timeStep(timeStep)
		    , bitOffset(bitOffset)
		    , bitLength(bitLength)
		    , flipBytes(flipBytes)
		{
		}

		/*!
		 * \brief Create a new DataView that starts at the head of a list.
		 *
		 * A DataView initialized this way will return false from isEmpty() until advanced.
		 * \param head the head of the list to start the DataView on
		 * \param timeStep the TimeStep to advance the DataView with
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 */
		DataView(std::atomic<LLNode<Type, NodeSize>*>* head, datatypes::TimeStep timeStep,
		    bool flipBytes) // NOLINT
		    : DataView(head, timeStep, 0, 0, flipBytes)
		{
		}

		/*!
		 * \brief Create a new DataView that starts at the head of a list.
		 *
		 * A DataView initialized this way will return false from isEmpty() until advanced.
		 * \param head the head of the list to start the DataView on
		 * \param timeStep the TimeStep to advance the DataView with
		 * \param bitOffset the offset of the desired output in the values
		 * \param bitLength the length of the desired output in the values
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 */
		DataView(std::atomic<LLNode<Type, NodeSize>*>* head, datatypes::TimeStep timeStep,
		    size_t bitOffset, size_t bitLength, bool flipBytes)
		    : node(head)
		    , index(0)
		    , timeStep(timeStep)
		    , bitOffset(bitOffset)
		    , bitLength(bitLength)
		    , flipBytes(flipBytes)
		{
		}

		double asDouble() const override
		{
			if (node.index() == 0)
			{
				throw std::out_of_range("This DataView is empty");
			}

			// This fails on bitsets (or does it? further testing required)
			if constexpr (std::is_arithmetic_v<Type>)
			{
				return Converter<Type, double>::shiftAndConvert(
				    std::get<1>(node)->values[index], bitOffset, bitLength, flipBytes);
			}
			else if constexpr (std::is_same<Type, std::unique_ptr<IOMap>>())
			{
				if constexpr (datatypes::is_bitset<Output>())
				{
					return static_cast<double>(Converter<IOMap*, Output>::shiftAndConvert(
					    std::get<1>(node)->values[index].get(), bitOffset, bitLength, flipBytes)
					                               .to_ulong());
				}
				else if constexpr (std::is_same<Output, IOMap*>())
				{
					return Converter<IOMap*, double>::shiftAndConvert(
					    std::get<1>(node)->values[index].get(), bitOffset, bitLength, flipBytes);
				}
				else
				{
					return static_cast<double>(Converter<IOMap*, Output>::shiftAndConvert(
					    std::get<1>(node)->values[index].get(), bitOffset, bitLength, flipBytes));
				}
			}
			return NAN;
		}

		DataView& operator++() override
		{
			std::lock_guard guard(timeMutex);
			if (node.index() == 0)
			{
				LLNode<Type, NodeSize>* cachedPointer
				    = (*std::get<0>(node)).load(std::memory_order_acquire);
				if (cachedPointer == nullptr)
				{
					return *this;
				}
				node = cachedPointer;
				index = 0;
				return *this;
			}
			ListLocation next = findNextLocation();
			node = next.node;
			index = next.index;
			return *this;
		}

		bool hasNext() const override
		{
			if (node.index() == 0)
			{
				return (*std::get<0>(node)).load(std::memory_order_acquire) != nullptr;
			}
			ListLocation next = findNextLocation();
			if (timeStep == datatypes::TimeStep(0))
			{
				return (next.node != std::get<1>(node) || next.index != index);
			}
			return (next.node->times[next.index] - std::get<1>(node)->times[index]) >= timeStep;
		}

		bool isEmpty() const override { return node.index() == 0; }

		datatypes::TimeStamp getTime() override
		{
			std::lock_guard guard(timeMutex);
			if (node.index() == 0)
			{
				throw std::out_of_range("This DataView is empty");
			}

			return std::get<1>(node)->times[index];
		}

		/*!
		 * \brief Return the value of the current DataPoint converted to Output
		 * \return the value of the current DataPoint as an Output
		 * \exception std::out_of_range iff isEmpty() would return true
		 */
		Output operator*() const
		{
			if (node.index() == 0)
			{
				throw std::out_of_range("This DataView is empty");
			}

			if constexpr (std::is_same<Type, std::unique_ptr<IOMap>>())
			{
				return Converter<IOMap*, Output>::shiftAndConvert(
				    std::get<1>(node)->values[index].get(), bitOffset, bitLength, flipBytes);
			}
			else
			{
				return Converter<Type, Output>::shiftAndConvert(
				    std::get<1>(node)->values[index], bitOffset, bitLength, flipBytes);
			}
		}

	private:
		std::variant<std::atomic<LLNode<Type, NodeSize>*>*, LLNode<Type, NodeSize>*> node;
		size_t index;
		datatypes::TimeStep timeStep;
		std::mutex timeMutex;
		size_t bitOffset;
		size_t bitLength;
		bool flipBytes;

		/*!
		 * \brief Find the next location in the list that this DataView would move to.
		 *
		 * If TimeStep is 0, returns the same location it is currently on if there is no next.
		 * If TimeStep is >0, returns the earliest location after `currentTime + timeStep` if
		 * available, or the latest location in the list if not.
		 * Must not be called if `node.index() == 0`.
		 * \return the next location, or the current one, or the latest location (see description)
		 * \exception std::bad_variant_access if called when `node.index() == 0`.
		 */
		ListLocation<Type, NodeSize> findNextLocation() const
		{
			ListLocation<Type, NodeSize> resultLocation{ std::get<1>(node), index };
			if (timeStep == datatypes::TimeStep(0)
			    && (index < std::get<1>(node)->count.load(std::memory_order_acquire) - 1
			           || std::get<1>(node)->next != nullptr))
			{
				if (index < std::get<1>(node)->count.load(std::memory_order_acquire) - 1)
				{
					++resultLocation.index;
				}
				else
				{
					resultLocation = { std::get<1>(node)->next.load(std::memory_order_acquire), 0 };
				}
			}
			else
			{
				LLNode<Type, NodeSize>* temp = std::get<1>(node);
				size_t tempCount = index;
				datatypes::TimeStamp oldTime = std::get<1>(node)->times[index];
				auto* nextNode = temp->next.load(std::memory_order_acquire);
				// Jump nodes until we hit the one that may contain our TimeStamp
				while (nextNode != nullptr && nextNode->times[0] <= oldTime + timeStep)
				{
					temp = nextNode;
					tempCount = 0;
					nextNode = nextNode->next.load(std::memory_order_acquire);
				}
				// Find largest index in node that is smaller than our TimeStamp
				while (temp->times[tempCount] < oldTime + timeStep
				    && tempCount < std::get<1>(node)->count.load(std::memory_order_acquire) - 1)
				{
					++tempCount;
				}
				// The first TimeStamp larger than ours may be in the next node
				if (tempCount == NodeSize - 1 && nextNode != nullptr
				    && temp->times[tempCount] < oldTime + timeStep)
				{
					temp = nextNode;
					tempCount = 0;
				}
				resultLocation = { temp, tempCount };
			}

			return resultLocation;
		}
	};
} // namespace etherkitten::reader
