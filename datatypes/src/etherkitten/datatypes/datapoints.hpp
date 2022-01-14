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
 * \brief Defines DataPoints, containers for arbitrary data associated with TimeStamps
 * and formattable as strings.
 */

#include <memory>
#include <string>
#include <type_traits>

#include "EtherCATTypeStringFormatter.hpp"
#include "dataobjects.hpp"
#include "ethercatdatatypes.hpp"
#include "time.hpp"
#include <memory>

namespace etherkitten::datatypes
{

	/*!
	 * \brief The AbstractDataPoint class is the base class for all DataPoints.
	 *
	 * It allows non-templated code to pass DataPoint references and format
	 * the contained values as strings.
	 *
	 * An AbstractDataPoint (and, by extension, a DataPoint) is always associated with
	 * a specific value and TimeStamp.
	 */
	class AbstractDataPoint // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		/*!
		 * \brief Construct a new AbstractDataPoint with the given parameters.
		 * \param time the TimeStamp for this AbstractDataPoint
		 */
		AbstractDataPoint(const TimeStamp& time);

		virtual ~AbstractDataPoint(){};

		/*!
		 * \brief Create a clone of this AbstractDataPoint.
		 * \return a pointer to a clone of this point, or nullptr on error
		 */
		virtual std::unique_ptr<AbstractDataPoint> clone() const = 0;

		/*!
		 * \brief Get the TimeStamp of this AbstractDataPoint.
		 * \return the TimeStamp of this AbstractDataPoint
		 */
		TimeStamp getTime() const;

		/*!
		 * \brief Create a string representation of the value in this AbstractDataPoint.
		 *
		 * If the type of the value in this AbstractDataPoint is arithmetic, it will be formatted
		 * according to the given base. If it is not numerical, the base will be ignored.
		 * \param base the base to format the value in, if applicable
		 * \return the value formatted as a string
		 */
		virtual std::string asString(NumberFormat base) const = 0;

	private:
		TimeStamp time;
	};

	template<class T>
	struct is_unique_ptr : std::false_type // NOLINT(readability-identifier-naming)
	{
	};

	template<class T, class D>
	struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
	{
	};

	/*!
	 * \brief The DataPoint class holds a value of a specified type along with a TimeStamp.
	 *
	 * \tparam T the type of the value the DataPoint holds
	 */
	template<typename T>
	class DataPoint : public AbstractDataPoint
	{
	public:
		/*!
		 * \brief Default constructor to reserve memory for a DataPoint.
		 */
		DataPoint()
		    : AbstractDataPoint(now())
		{
		}

		/*!
		 * \brief Construct a new DataPoint with the given parameters.
		 * \param value the value to store in this DataPoint
		 * \param time the TimeStamp to store in this DataPoint
		 */
		DataPoint(T value, const TimeStamp& time)
		    : AbstractDataPoint(time)
		    , value(std::move(value))
		{
		}

		/*!
		 * \brief Clone this AbstractDataPoint and return a pointer to the clone.
		 *
		 * This method will return nullptr if T is already a unique_ptr.
		 * \return a pointer to a clone of this point if the type is not a unique_ptr
		 */
		std::unique_ptr<AbstractDataPoint> clone() const override
		{
			if constexpr (is_unique_ptr<T>())
			{
				return std::unique_ptr<AbstractDataPoint>();
			}
			else
			{
				return std::make_unique<DataPoint>(*this);
			}
		}

		std::string asString(NumberFormat base) const override
		{
			if constexpr (!is_unique_ptr<T>())
			{
				return EtherCATTypeStringFormatter<T>::asString(this->value, base);
			}
			return "";
		}

		/*!
		 * \brief Get the value stored in this DataPoint.
		 *
		 * If T is a unique_ptr, a raw pointer to the data will be returned instead.
		 * \return the value in this DataPoint
		 */
		template<typename Q = T>
		std::enable_if_t<!is_unique_ptr<Q>(), Q> getValue() const
		{
			return this->value;
		}

		template<typename Q = T>
		std::enable_if_t<!!is_unique_ptr<Q>(), typename Q::pointer> getValue() const
		{
			return this->value.get();
		}

	private:
		T value;
	};
} // namespace etherkitten::datatypes
