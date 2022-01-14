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


#include "time.hpp"

namespace etherkitten::datatypes
{
	TimeStamp now() { return std::chrono::steady_clock::now(); };

	uint64_t timeStampToInt(TimeStamp timeStamp)
	{
		return std::chrono::time_point_cast<std::chrono::microseconds>(timeStamp)
		    .time_since_epoch()
		    .count();
	}

	TimeStamp intToTimeStamp(uint64_t timestamp)
	{
		return TimeStamp(std::chrono::microseconds(timestamp));
	}
} // namespace etherkitten::datatypes
