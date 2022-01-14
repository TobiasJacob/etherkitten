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
 * \brief Defines DataViews that can be used to iterate over or view data stored
 * in arbitrary containers in a type-agnostic way.
 */

#include "datapoints.hpp"
#include "time.hpp"

namespace etherkitten::datatypes
{

	/*!
	 * \brief The AbstractDataView class defines an iterator over generic values associated
	 * with TimeStamps.
	 *
	 * Note that AbstractDataView is not a template class and only allows access to values
	 * by converting them to double. If you need to preserve the value's type, define
	 * additional methods in an implementation of this interface.
	 */
	class AbstractDataView // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~AbstractDataView() {}

		/*!
		 * \brief Get the value that is currently pointed to as a double.
		 * \return the value converted to a double
		 * \exception std::out_of_range iff isEmpty() would return true
		 */
		virtual double asDouble() const = 0;

		/*!
		 * \brief Move this AbstractDataView to the next value in the range.
		 *
		 * This operator must not be used if `hasNext()` would return false.
		 * \return a reference to this AbstractDataView
		 */
		virtual AbstractDataView& operator++() = 0;

		/*!
		 * \brief Check whether the range this AbstractDataView iterates over has another value.
		 * \return whether another value is available
		 */
		virtual bool hasNext() const = 0;

		/*!
		 * \brief Check whether this AbstractDataView points to no data.
		 *
		 * This method will never return true after ++ has been applied to this AbstractDataView
		 * at least once.
		 * \retval true iff this AbstractDataView may not be dereferenced
		 * \retval false iff this AbstractDataView may be dereferenced
		 */
		virtual bool isEmpty() const = 0;

		/*!
		 * \brief Get the TimeStamp of the value that is currently pointed to.
		 * \return the TimeStamp of the value
		 * \exception std::out_of_range iff isEmpty() would return true
		 */
		virtual TimeStamp getTime() = 0;
	};

	/*!
	 * \brief The AbstractNewestValueView class defines an interface for a pointer to an
	 * AbstractDataPoint.
	 *
	 * This class always points to the AbstractDataPoint with the greatest TimeStamp belonging to
	 * a given DataObject.
	 */
	class AbstractNewestValueView // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~AbstractNewestValueView() {}

		/*!
		 * \brief Check if there is a newest value to get.
		 * \retval true if there is no newest value
		 * \retval false if there is a newest value
		 */
		virtual bool isEmpty() const = 0;

		/*!
		 * \brief Get the AbstractDataPoint that currently has the greatest TimeStamp.
		 * \return the AbstractDataPoint with the greatest TimeStamp
		 * \exception std::runtime_error if isEmpty() would return true
		 */
		virtual const AbstractDataPoint& operator*() = 0;
	};

} // namespace etherkitten::datatypes
