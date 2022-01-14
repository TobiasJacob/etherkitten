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
 * \brief Defines some basic time classes based on the Linux API.
 */

#include <chrono>
#include <cstdint>

namespace etherkitten::datatypes
{

	/*!
	 * \brief TimeStamp should use CLOCK_MONOTONIC internally on Linux.
	 *
	 * If not, we can switch back to using the C API.
	 * The monotonic clock on Linux (see *clock_gettime(2)*, especially *CLOCK_MONOTONIC*)
	 * starts counting when the system boots. Thus, the times held in TimeStamp instances
	 * should not be used absolutely, but only in relation to other TimeStamp instances
	 * from the same clock.
	 */
	typedef std::chrono::time_point<std::chrono::steady_clock> TimeStamp;

	/*!
	 * \brief Get the current time.
	 */
	TimeStamp now();

	/*!
	 * \brief TimeStep represents a step in time of 1us (one microsecond).
	 *
	 * Can be initialized like this: `TimeStep(int)`.
	 */
	typedef std::chrono::microseconds TimeStep;

	/*!
	 * \brief The TimeSeries struct defines a series of points in time.
	 *
	 * The series of points is defined via a starting point and a step size in microseconds.
	 */
	struct TimeSeries
	{
		/*!
		 * \brief The time for the series of points to start.
		 */
		TimeStamp startTime;

		/*!
		 * \brief The difference between two points in the series.
		 */
		TimeStep microStep;
	};

	/*!
	 * \brief Convert a TimeStamp to int
	 * \param timeStamp the TimeStamp to convert
	 * \return the int representing the TimeStamp
	 */
	uint64_t timeStampToInt(TimeStamp timeStamp);

	/*!
	 * \brief Convert int to TimeStamp
	 * \param timestamp the int to convert
	 * \return the TimeStamp representing the int
	 */
	TimeStamp intToTimeStamp(uint64_t timestamp);
} // namespace etherkitten::datatypes
