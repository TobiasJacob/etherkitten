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
 * \brief Defines the SearchList, a class that holds an unrolled linked list data structure
 * and offers O(1) searching operations on that list.
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "DataView.hpp"
#include "LLNode.hpp"

namespace etherkitten::reader
{
	using namespace std::chrono_literals;

	using TimePointMin = std::chrono::time_point<std::chrono::steady_clock, std::chrono::minutes>;

	/*!
	 * \brief The SearchList class is an implementation of a singly linked list.
	 *
	 * It allows O(1) access to any node in the list via auxiliary searching structures.
	 * A SearchList can be appended to in parallel to being read.
	 * \tparam Type which type of values the SearchList holds
	 * \tparam NodeSize the size of the LLNodes in the list
	 */
	template<typename Type, size_t NodeSize = 1>
	class SearchList
	{
	public:
		/*!
		 * \brief The type of values this SearchList holds.
		 */
		using contained = Type;

		SearchList()
		    : head(nullptr)
		    , tail(nullptr)
		{
		}

		SearchList(const SearchList&) = delete;

		SearchList(SearchList&&) noexcept = default;

		SearchList& operator=(const SearchList&) = delete;

		SearchList& operator=(SearchList&&) noexcept = default;

		~SearchList()
		{
			LLNode<Type, NodeSize>* current = head.load(std::memory_order_acquire);
			while (current != nullptr)
			{
				LLNode<Type, NodeSize>* prev = current;
				current = current->next.load(std::memory_order_acquire);
				delete prev; // NOLINT
			}
		}

		/*!
		 * \brief Append the given value and time to a new node at the end of the SearchList.
		 *
		 * IMPORTANT: The TimeStamps appended to the list MUST increase monotonically.
		 * If they are not, the behavior of the searching algorithms is unpredictable and
		 * likely not what you want.
		 * \param value the value to be appended
		 * \param time the time to be appended
		 */
		void append(Type value, datatypes::TimeStamp time)
		{
			TimePointMin timeFloor = std::chrono::floor<std::chrono::minutes>(time);
			std::lock_guard lg(appendMutex);
			auto* lastNode = tail.load(std::memory_order_acquire);
			size_t count
			    = lastNode != nullptr ? lastNode->count.load(std::memory_order_acquire) : 0;
			// We still have a node that isn't full yet
			if (lastNode != nullptr && count < NodeSize)
			{
				lastNode->values[count] = std::move(value);
				lastNode->times[count] = time;
				lastNode->count.fetch_add(1);
			}
			// We need to make a new node
			else
			{
				LLNode<Type, NodeSize>* temp // NOLINT
				    = new LLNode<Type, NodeSize>(std::move(value), time, nullptr);
				// List is empty
				if (head.load(std::memory_order_acquire) == nullptr)
				{
					head.store(temp, std::memory_order_release);
					tail.store(temp, std::memory_order_release);
				}
				// List has entries
				else
				{
					tail.load(std::memory_order_acquire)
					    ->next.store(temp, std::memory_order_release);
					tail.store(temp, std::memory_order_release);
				}
			}

			// New map entry has to be added
			if (lastTimeInMap.time_since_epoch().count() == 0 || timeFloor >= lastTimeInMap + 1min)
			{
				setNodeLocked(timeFloor, { tail.load(std::memory_order_acquire), count });
				lastTimeInMap = timeFloor;
			}
		}

		/*!
		 * \brief Get the location of the newest data point in the list.
		 * \return the location of the newest data point
		 */
		ListLocation<Type, NodeSize> getNewest() const
		{
			if (head.load(std::memory_order_acquire) == nullptr)
			{
				return { nullptr, 0 };
			}
			auto* lastNode = tail.load(std::memory_order_acquire);
			return { lastNode, lastNode->count.load(std::memory_order_acquire) - 1 };
		}

		/*!
		 * \brief Remove the oldest nodes of the SearchList.
		 * \param count the amount of nodes to remove
		 * \return amount of actually removed nodes
		 */
		int removeOldest(unsigned int count)
		{
			std::lock_guard<std::mutex> lg(modificationMutex);

			if (!head)
			{
				// if the SearchList is empty we cannot remove anything
				return 0;
			}

			deleteUnusedDataViews();

			// Calculate the earliest TimeStamp of any used DataView
			datatypes::TimeStamp earliestViewTime = datatypes::TimeStamp::max();
			for (auto& dataView : usedDataViews)
			{
				if (dataView->getTime() < earliestViewTime)
				{
					earliestViewTime = dataView->getTime();
				}
			}

			LLNode<Type, NodeSize>* oldHead = head.load(std::memory_order_acquire);
			// Find the node where either count is reached or a DataView still requires it
			LLNode<Type, NodeSize>* headAfterRemoval
			    = findHeadAfterRemoval(oldHead, count, earliestViewTime);
			head.store(headAfterRemoval, std::memory_order_release);

			// Correct node map
			TimePointMin headSec
			    = std::chrono::floor<std::chrono::minutes>(headAfterRemoval->times[0]);
			for (TimePointMin i = std::chrono::floor<std::chrono::minutes>(oldHead->times[0]);
			     i < headSec; i += 1min)
			{
				eraseNodeLocked(i);
			}
			setNodeLocked(headSec, { headAfterRemoval, 0 });

			// Actually remove all nodes until the new head
			int actuallyRemoved = 0;
			while (oldHead != headAfterRemoval)
			{
				LLNode<Type, NodeSize>* nodeToRemove = oldHead;
				oldHead = oldHead->next.load(std::memory_order_acquire);
				delete nodeToRemove; // NOLINT
				++actuallyRemoved;
			}
			return actuallyRemoved;
		}

		/*!
		 * \brief Get a DataView that moves in the given time increments.
		 * \tparam Output the type the underlying values should be converted to
		 * \param timeStep the TimeStep the DataView should increment in
		 * \param flipBytes whether to flip the bytes of the data on output on big endian hosts
		 * \return the new DataView
		 */
		template<typename Output = Type>
		std::shared_ptr<DataView<Type, NodeSize, Output>> getView(
		    datatypes::TimeSeries timeSeries, bool flipBytes)
		{
			return getView<Output>(timeSeries, 0, 0, flipBytes);
		}

		/*!
		 * \brief Get a DataView that moves in the given time increments and
		 * applies bit operations to its values.
		 * \tparam Output the type the underlying values should be converted to
		 * \param timeStep the TimeStep the DataView should increment in
		 * \param bitOffset the offset of the desired output in the values
		 * \param bitLength the length of the desired output in the values
		 * \param flipBytes whether to flip the bytes of the data on output on big endian hosts
		 * \return the new DataView
		 */
		template<typename Output = Type>
		std::shared_ptr<DataView<Type, NodeSize, Output>> getView(
		    datatypes::TimeSeries timeSeries, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			std::lock_guard<std::mutex> lg(modificationMutex);
			std::optional<ListLocation<Type, NodeSize>> location
			    = findAfterTimeStamp(timeSeries.startTime);
			std::shared_ptr<DataView<Type, NodeSize, Output>> view;
			if (location.has_value())
			{
				view = std::make_shared<DataView<Type, NodeSize, Output>>(
				    location.value(), timeSeries.microStep, bitOffset, bitLength, flipBytes);
			}
			else
			{
				view = std::make_shared<DataView<Type, NodeSize, Output>>(
				    &head, timeSeries.microStep, bitOffset, bitLength, flipBytes);
			}
			usedDataViews.push_back(view);

			return view;
		}

	private:
		std::atomic<LLNode<Type, NodeSize>*> head;

		std::atomic<LLNode<Type, NodeSize>*> tail;

		struct TimePointHash;
		struct TimePointEqual;

		std::unordered_map<TimePointMin, ListLocation<Type, NodeSize>, TimePointHash,
		    TimePointEqual>
		    nodes;

		std::vector<std::shared_ptr<datatypes::AbstractDataView>> usedDataViews;

		std::vector<std::shared_ptr<datatypes::AbstractDataView>> toDeleteDataViews;

		TimePointMin lastTimeInMap;

		std::mutex modificationMutex;

		std::mutex appendMutex;

		std::mutex nodeMapMutex;

		std::optional<ListLocation<Type, NodeSize>> findAfterTimeStamp(
		    datatypes::TimeStamp& timeStamp)
		{
			if (head == nullptr)
			{
				return {};
			}
			auto* lastNode = tail.load(std::memory_order_acquire);
			if (timeStamp > lastNode->times[lastNode->count.load(std::memory_order_acquire) - 1])
			{
				return { { lastNode, lastNode->count.load(std::memory_order_acquire) - 1 } };
			}
			if (timeStamp < head.load(std::memory_order_acquire)->times[0])
			{
				return { { head, 0 } };
			}
			ListLocation<Type, NodeSize> location = { nullptr, 0 };
			TimePointMin time = std::chrono::floor<std::chrono::minutes>(timeStamp);
			while (location.node == nullptr)
			{
				if (countNodeLocked(time) != 0)
				{
					location = getNodeLocked(time);
				}
				else
				{
					time += 1min;
				}
			}
			auto* currentNode = location.node;
			size_t currentIndex = location.index;
			auto* nextNode = location.node->next.load(std::memory_order_acquire);
			while (nextNode != nullptr && nextNode->times[0] <= timeStamp)
			{
				currentNode = nextNode;
				currentIndex = 0;
				nextNode = nextNode->next.load(std::memory_order_acquire);
			}
			while (currentNode->times[currentIndex] < timeStamp)
			{
				if (currentIndex == NodeSize - 1)
				{
					currentNode = currentNode->next.load(std::memory_order_acquire);
					currentIndex = 0;
				}
				else
				{
					++currentIndex;
				}
			}
			return { { currentNode, currentIndex } };
		}

		void setNodeLocked(TimePointMin min, ListLocation<Type, NodeSize> node)
		{
			std::lock_guard guard(nodeMapMutex);
			nodes[min] = node;
		}

		ListLocation<Type, NodeSize> getNodeLocked(TimePointMin min)
		{
			std::lock_guard guard(nodeMapMutex);
			return nodes.at(min);
		}

		size_t countNodeLocked(TimePointMin min)
		{
			std::lock_guard guard(nodeMapMutex);
			return nodes.count(min);
		}

		void eraseNodeLocked(TimePointMin min)
		{
			std::lock_guard guard(nodeMapMutex);
			nodes.erase(min);
		}

		struct TimePointHash
		{
			std::size_t operator()(const TimePointMin& timePoint) const
			{
				return static_cast<size_t>(timePoint.time_since_epoch().count());
			}
		};

		struct TimePointEqual
		{
			bool operator()(const TimePointMin& lhs, const TimePointMin& rhs) const
			{
				return lhs.time_since_epoch().count() == rhs.time_since_epoch().count();
			}
		};

		void deleteUnusedDataViews()
		{
			// Delete all DataViews on the graveyard
			for (auto dataViewPtr : toDeleteDataViews)
			{
				dataViewPtr.reset();
			}
			toDeleteDataViews.clear();
			// Move the DataViews that have become unused since the last call to the graveyard
			size_t i = 0;
			while (i < usedDataViews.size())
			{
				if (usedDataViews.at(i).unique())
				{
					// Move unused DataView to graveyard
					toDeleteDataViews.push_back(usedDataViews.at(i));
					// If the unused DataView was the last entry in the list
					if (i == usedDataViews.size() - 1)
					{
						// Just remove it
						usedDataViews.pop_back();
					}
					else
					{
						// Save last entry from usedDataViews
						std::shared_ptr<datatypes::AbstractDataView> temp = usedDataViews.back();
						// Remove it
						usedDataViews.pop_back();
						// And put it where the unused DataView was
						usedDataViews.at(i) = temp;
					}
					// Do not increment the index, since a new (unchecked) DataView is at [i] now
				}
				else
				{
					i++;
				}
			}
		}

		LLNode<Type, NodeSize>* findHeadAfterRemoval(LLNode<Type, NodeSize>* oldHead,
		    unsigned int count, datatypes::TimeStamp earliestViewTime)
		{
			LLNode<Type, NodeSize>* headAfterRemoval = oldHead;
			for (unsigned int i = 0; i < count
			     && headAfterRemoval
			             ->times[headAfterRemoval->count.load(std::memory_order_acquire) - 1]
			         < earliestViewTime;
			     ++i)
			{
				if (!headAfterRemoval->next)
				{
					// We keep the newest DataPoint so we don't invalidate
					// NewestValueViews.
					break;
				}
				headAfterRemoval = headAfterRemoval->next;
			}
			return headAfterRemoval;
		}
	};
} // namespace etherkitten::reader
