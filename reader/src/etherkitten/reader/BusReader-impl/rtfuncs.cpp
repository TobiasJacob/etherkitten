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


#include "rtfuncs.hpp"

namespace etherkitten::reader::bReader
{
	bool pinThreadToCPU(unsigned int cpu)
	{
		cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(cpu, &mask); // NOLINT

		int status = sched_setaffinity(0, sizeof(mask), &mask);
		if (status != 0)
		{
			return false;
		}

		cpu_set_t validationMask;
		CPU_ZERO(&validationMask);
		CPU_SET(cpu, &validationMask); // NOLINT

		sched_getaffinity(0, sizeof(validationMask), &validationMask);
		return !CPU_EQUAL(&mask, &validationMask);
	}

	bool setThreadPriority(int priority)
	{
		struct sched_param parameters
		{
			.sched_priority = priority
		};
		int status = sched_setscheduler(0, SCHED_FIFO | SCHED_RESET_ON_FORK, &parameters);
		return status != -1;
	}

} // namespace etherkitten::reader::bReader
