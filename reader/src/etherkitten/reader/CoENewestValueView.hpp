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
 * \brief Defines the CoENewestValueView, an adaptation of AbstractNewestValueViews for the values
 * of CoEObjects.
 */

#include <memory>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>

#include "Converter.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Implements the AbstractNewestValueView for the values of CoEObjects.
	 *
	 * This AbstractNewestValueView gets its data via an indirect pointer rather than a SearchList.
	 * \tparam the datatypes::EtherCATDataTypeEnum type that the related DataPoints have
	 */
	class CoENewestValueView : public datatypes::AbstractNewestValueView
	{
	public:
		/*!
		 * \brief Construct a new CoENewestValueView that observes the given indirect pointer
		 * for its data source.
		 * \param dataPoint the indirect pointer to get the data from
		 */
		CoENewestValueView(std::shared_ptr<std::shared_ptr<datatypes::AbstractDataPoint>> dataPoint)
		    : dataPoint(std::move(dataPoint))
		    , dataPointCopy(nullptr)
		{
		}

		bool isEmpty() const override { return *dataPoint == nullptr; }

		/*!
		 * \brief Return a reference to a copy of the DataPoint behind the indirect pointer
		 * \return a reference to a copy of the DataPoint this class is pointing at
		 */
		const datatypes::AbstractDataPoint& operator*() override
		{
			dataPointCopy = (**dataPoint).clone();
			return *dataPointCopy;
		}

	private:
		std::shared_ptr<std::shared_ptr<datatypes::AbstractDataPoint>> dataPoint;
		std::unique_ptr<datatypes::AbstractDataPoint> dataPointCopy;
	};
} // namespace etherkitten::reader
