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

#include "TimeStampConverter.hpp"
#include <chrono>

namespace etherkitten::gui
{

	TimeStampConverter::TimeStampConverter() {}

	void TimeStampConverter::setStart(datatypes::TimeStamp start) { this->start = start; }

	datatypes::TimeStamp TimeStampConverter::getStart() const { return start; }

	datatypes::TimeSeries TimeStampConverter::milliToTimeSeries(long startTime, long origStep) const
	{
		datatypes::TimeStamp stamp = milliToTimeStamp(startTime);
		std::chrono::duration<long, std::milli> d(origStep);
		datatypes::TimeStep step = std::chrono::duration_cast<datatypes::TimeStep>(d);
		return { stamp, step };
	}

	datatypes::TimeStamp TimeStampConverter::milliToTimeStamp(long time) const
	{
		std::chrono::duration<long, std::milli> d(time);
		return start + std::chrono::duration_cast<datatypes::TimeStep>(d);
	}

	long TimeStampConverter::timeToMilli(datatypes::TimeStamp time) const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(time - start).count();
	}

} // namespace etherkitten::gui
