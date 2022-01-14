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
 *
 * Contains helper functions to improve the realtime behavior of the BusReader.
 */

extern "C"
{
#include <sched.h>
}

namespace etherkitten::reader::bReader
{
	/*!
	 * \brief Pin the calling thread to the given CPU.
	 * \param cpu the CPU to pin the thread to
	 * \retval true iff the pinning was successful
	 * \retval false iff the pinning was not successful
	 */
	bool pinThreadToCPU(unsigned int cpu);

	/*!
	 * \brief Set the priority of this thread.
	 * \param priority the priority to set for this thread
	 * \retval true iff setting the priority was successful
	 * \retval false iff setting the priority was not successful
	 */
	bool setThreadPriority(int priority);

} // namespace etherkitten::reader::bReader
