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
 * \brief Defines the NewestValueView, a view that always returns the newest data point in a
 * SearchList.
 */

#include <memory>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>

#include "Converter.hpp"
#include "SearchList.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Implements the AbstractNewestValueView for an EtherCATDataTypeEnum.
	 * \tparam Type the type from datatypes::EtherCATDataTypeEnum that the related DataPoints have
	 * \tparam NodeSize the size of the LLNodes in the related list
	 * \tparam Output the type the view should output when asked
	 */
	template<class Type, size_t NodeSize = 1, class Output = Type>
	class NewestValueView : public datatypes::AbstractNewestValueView
	{
	public:
		/*!
		 * \brief Create a new NewestValueView that will always output the newest value in the list.
		 * \param list the list to get the values from
		 * \param bitOffset the offset of the desired output in the values
		 * \param bitLength the length of the desired output in the values
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 */
		NewestValueView(const SearchList<Type, NodeSize>& list, size_t bitOffset, size_t bitLength,
		    bool flipBytes)
		    : list(list)
		    , bitOffset(bitOffset)
		    , bitLength(bitLength)
		    , flipBytes(flipBytes)
		{
		}

		bool isEmpty() const override { return list.getNewest().node == nullptr; }

		const datatypes::AbstractDataPoint& operator*() override
		{
			ListLocation<Type, NodeSize> location = list.getNewest();
			if constexpr (datatypes::is_unique_ptr<Type>())
			{
				dataPointCopy = datatypes::DataPoint{
					Converter<typename Type::pointer, Output>::shiftAndConvert(
					    location.node->values[location.index].get(), bitOffset, bitLength,
					    flipBytes),
					location.node->times[location.index]
				};
			}
			else
			{
				dataPointCopy = datatypes::DataPoint{ Converter<Type, Output>::shiftAndConvert(
					                                      location.node->values[location.index],
					                                      bitOffset, bitLength, flipBytes),
					location.node->times[location.index] };
			}
			return dataPointCopy;
		}

	private:
		const SearchList<Type, NodeSize>& list;
		datatypes::DataPoint<Output> dataPointCopy;

		size_t bitOffset;
		size_t bitLength;
		bool flipBytes;
	};
} // namespace etherkitten::reader
