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
 * \brief Defines the RegisterScheduler, a class that constructs and RR-schedules EtherCATFrames.
 */

#include <atomic>
#include <map>
#include <utility>
#include <vector>

#include <etherkitten/datatypes/dataobjects.hpp>

#include "EtherCATFrame.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The RegisterScheduler class constructs and RR-schedules EtherCATFrames.
	 */
	class RegisterScheduler
	{
	public:
		/*!
		 * \brief Construct a new RegisterScheduler that schedules the given registers
		 * for all of the given slaves.
		 * \param slaveConfiguredAddresses the configured addresses of the slaves to
		 * schedule register readings for
		 * \param toRead the register selection to schedule readings for
		 */
		RegisterScheduler(const std::vector<uint16_t>& slaveConfiguredAddresses,
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		/*!
		 * \brief Construct a new RegisterScheduler that schedules the same registers
		 * as the other.
		 * \param other the other RegisterScheduler
		 */
		RegisterScheduler(const RegisterScheduler& other);

		/*!
		 * \brief Construct a new RegisterScheduler that schedules the same registers
		 * as the other.
		 *
		 * This constructor keeps all the references acquired via the other
		 * RegisterScheduler valid.
		 * \param other the other RegisterScheduler
		 */
		RegisterScheduler(RegisterScheduler&& other) noexcept;

		/*!
		 * \brief Make this RegisterScheduler schedule the same registers as the other.
		 *
		 * This operator will keep all references acquired via this RegisterScheduler valid.
		 *
		 * This method is NOT thread safe.
		 * \param other the other RegisterScheduler
		 */
		RegisterScheduler& operator=(const RegisterScheduler& other);

		/*!
		 * \brief Make this RegisterScheduler schedule the same registers as the other.
		 *
		 * This operator will keep all references acquired via this RegisterScheduler
		 * or the other RegisterScheduler valid.
		 * \param other the other RegisterScheduler
		 */
		RegisterScheduler& operator=(RegisterScheduler&& other) noexcept;

		~RegisterScheduler();

		/*!
		 * \brief Get the number of different EtherCATFrames scheduled by this RegisterScheduler.
		 *
		 * This method may be called simultaneously to getNextFrames, changeRegisterSettings,
		 * and itself.
		 * \return the number of different EtherCATFrames round-robin-scheduled by this
		 * RegisterScheduler.
		 */
		size_t getFrameCount();

		/*!
		 * \brief Get an iterator over the given number of EtherCATFrames with metadata.
		 *
		 * When this method is called for the first time, the iterator will start at
		 * the first available frame. For every call after that, the iterator will start
		 * at the frame after the end of the last iterator's range. The iterators wrap
		 * around to the first available frame round-robin-style once all frames have
		 * been visited.
		 *
		 * If changeRegisterSettings is called between calls to this method,
		 * the iterator will once again start at the first available frame given the
		 * new register settings.
		 *
		 * This method may be called simultaneously to changeRegisterSettings, but not
		 * to itself.
		 * \param frameCount the number of EtherCATFrames to iterate over
		 * \return an iterator over `frameCount` EtherCATFrames
		 */
		EtherCATFrameIterator getNextFrames(int frameCount);

		/*!
		 * \brief Change the selection of registers to schedule readings for.
		 *
		 * This method may be called simultaneously to getNextFrames, but not to itself.
		 * \param toRead the register selection to schedule readings for
		 */
		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

	private:
		/*!
		 * \brief The EtherCATFrameList that iterators are currently being handed out for.
		 */
		std::atomic<EtherCATFrameList*> currentFrameList;

		/*!
		 * \brief All EtherCATFrameLists that have been created by changeRegisterSettings.
		 *
		 * I haven't found a good way to determine when these can be deleted yet,
		 * so for now, they just accumulate. Shared pointers seem like they would incur
		 * too much overhead in the realtime or data storage threads of the BusReader.
		 */
		std::vector<EtherCATFrameList*> frameLists;

		std::vector<uint16_t> slaveConfiguredAddresses;

		EtherCATFrameList* createEtherCATFrameList(
		    const std::vector<std::pair<int, int>>& pduIntervals);
	};
} // namespace etherkitten::reader
