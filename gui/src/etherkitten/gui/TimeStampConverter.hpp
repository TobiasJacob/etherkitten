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

#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::gui
{

	/*!
	 * \brief Converts between absolute timestamps and the relative
	 * millisecond times used by the plots.
	 */
	class TimeStampConverter
	{
	public:
		/*!
		 * \brief Create a new TimeStampConverter.
		 */
		TimeStampConverter();
		/*!
		 * \brief Set the start time that converted times are relative to.
		 * \param start The start time.
		 */
		void setStart(datatypes::TimeStamp start);
		/*!
		 * \brief Return the start time that converted times are relative to.
		 * \return The start time.
		 */
		datatypes::TimeStamp getStart() const;
		/*!
		 * \brief Convert relative milliseconds to a TimeSeries.
		 * \param start The relative milliseconds of the start time.
		 * \param step The milliseconds of the step.
		 * \return The generated TimeSeries.
		 */
		datatypes::TimeSeries milliToTimeSeries(long start, long step) const;
		/*!
		 * \brief Convert relative milliseconds to an absolute TimeStamp.
		 * \param time The relative milliseconds to convert.
		 * \return The generated absolute TimeStamp.
		 */
		datatypes::TimeStamp milliToTimeStamp(long time) const;
		/*!
		 * \brief Convert an absolute TimeStamp to relative milliseconds.
		 * \param time The absolute TimeStamp.
		 * \return The converted relative milliseconds.
		 */
		long timeToMilli(datatypes::TimeStamp time) const;

	private:
		datatypes::TimeStamp start;
	};

} // namespace etherkitten::gui
