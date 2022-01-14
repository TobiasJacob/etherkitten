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


#include "RegisterScheduler.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include <etherkitten/datatypes/dataobjects.hpp>

#include "endianness.hpp"

namespace etherkitten::reader
{
	RegisterScheduler::RegisterScheduler(const std::vector<uint16_t>& slaveConfiguredAddresses,
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	    : currentFrameList(new EtherCATFrameList())
	    , slaveConfiguredAddresses(slaveConfiguredAddresses)
	{
		changeRegisterSettings(toRead);
	}

	RegisterScheduler::RegisterScheduler(const RegisterScheduler& other)
	    : currentFrameList(other.currentFrameList.load(std::memory_order_acquire))
	    , frameLists{ new EtherCATFrameList(*currentFrameList) }
	    , slaveConfiguredAddresses(other.slaveConfiguredAddresses)
	{
	}

	RegisterScheduler::RegisterScheduler(RegisterScheduler&& other) noexcept
	    : currentFrameList(other.currentFrameList.load(std::memory_order_acquire))
	    , frameLists(std::move(other.frameLists))
	    , slaveConfiguredAddresses(std::move(other.slaveConfiguredAddresses))
	{
	}

	RegisterScheduler& RegisterScheduler::operator=(const RegisterScheduler& other)
	{
		this->currentFrameList.store(
		    other.currentFrameList.load(std::memory_order_acquire), std::memory_order_release);
		this->frameLists.push_back(new EtherCATFrameList(*currentFrameList)); // NOLINT
		this->slaveConfiguredAddresses = other.slaveConfiguredAddresses;
		return *this;
	}

	RegisterScheduler& RegisterScheduler::operator=(RegisterScheduler&& other) noexcept
	{
		this->currentFrameList.store(
		    other.currentFrameList.load(std::memory_order_acquire), std::memory_order_release);
		auto tmp = std::move(other.frameLists);
		this->frameLists.insert(this->frameLists.end(), tmp.begin(), tmp.end());
		this->slaveConfiguredAddresses = std::move(other.slaveConfiguredAddresses);
		return *this;
	}

	RegisterScheduler::~RegisterScheduler()
	{
		for (EtherCATFrameList* list : frameLists)
		{
			delete list; // NOLINT
		}
	}

	size_t RegisterScheduler::getFrameCount()
	{
		return currentFrameList.load(std::memory_order_acquire)->list.size();
	}

	EtherCATFrameIterator RegisterScheduler::getNextFrames(int frameCount)
	{
		EtherCATFrameList* list = currentFrameList.load(std::memory_order_acquire);
		size_t nextIndex = list->nextIndex;
		list->nextIndex = (list->nextIndex + frameCount) % list->list.size();
		return EtherCATFrameIterator(list, nextIndex, frameCount);
	}

	/*!
	 * \brief Turn a map of which registers to read into a vector of addresses that must be read.
	 *
	 * Multi-byte registers are accounted for - every byte is added to the list individually.
	 * The returned list is sorted.
	 * \param toRead the register read map to turn into a list of addresses
	 * \return the sorted list of addresses
	 */
	std::vector<int> registerReadMapToAddressList(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		std::vector<int> addressesToRead;
		for (std::pair<datatypes::RegisterEnum, bool> selection : toRead)
		{
			if (!selection.second)
			{
				continue;
			}

			// The higher bytes are used to index into a byte in our register numbering scheme
			// - We don't need that here.
			static constexpr int addressMask = 0xFFFF;
			static constexpr float byteLength = 8.0;

			int baseAddress = static_cast<int>(selection.first) & addressMask;

			for (int i = 0; // NOLINTNEXTLINE(bugprone-narrowing-conversions)
			     i < std::ceil(datatypes::registerMap.at(selection.first).bitLength / byteLength);
			     ++i)
			{
				addressesToRead.push_back(baseAddress + i);
			}
		}
		std::sort(addressesToRead.begin(), addressesToRead.end());
		addressesToRead.erase(
		    std::unique(addressesToRead.begin(), addressesToRead.end()), addressesToRead.end());
		return addressesToRead;
	}

	/*!
	 * \brief Calculate the optimal separation of registers into intervals for PDUs.
	 *
	 * Note that the intervals may not be optimal once you pack them into EtherCAT frames
	 * because their size might lead to suboptimal fittings. Usually, the intervals
	 * shouldn't be that large though (an Ethernet frame fits 1500 bytes of payload, after all).
	 *
	 * The interval ends are exclusive (i.e., the interval is [start, end)).
	 * \param toRead the registers to fit into intervals
	 * \return a list of intervals that cover the registers optimally
	 */
	std::vector<std::pair<int, int>> createFrameIntervals(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		if (toRead.empty())
		{
			return {};
		}

		std::vector<int> addressesToRead(registerReadMapToAddressList(toRead));

		std::vector<std::pair<int, int>> pduIntervals;
		int currentIntervalStart = 0;
		int currentIntervalEnd = 0;

		for (int address : addressesToRead)
		{
			// If we've started accumulating an interval && adding the next address
			// adds fewer bytes than just starting a new PDU && our PDU hasn't grown too large
			if (currentIntervalStart > 0 && address - currentIntervalEnd - 1 < pduOverhead
			    && address + 1 - currentIntervalStart + pduOverhead
			        < static_cast<int>(maxTotalPDULength))
			{
				currentIntervalEnd = address + 1;
			}
			else
			{
				if (currentIntervalStart > 0)
				{
					pduIntervals.push_back({ currentIntervalStart, currentIntervalEnd });
				}
				currentIntervalStart = address;
				currentIntervalEnd = address + 1;
			}
		}
		pduIntervals.push_back({ currentIntervalStart, currentIntervalEnd });
		return pduIntervals;
	}

	void RegisterScheduler::changeRegisterSettings(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		auto intervals = createFrameIntervals(toRead);
		EtherCATFrameList* newList = createEtherCATFrameList(intervals);
		frameLists.push_back(newList);
		currentFrameList.store(newList, std::memory_order_release);
	}

	/*!
	 * \brief Create the PDUMetaData for a PDU in an EtherCAT frame.
	 * \param pduInterval the PDU interval (associated with a slave) to generate metadata for
	 * \param pduOffset the offset of the PDU relative to the start of the EtherCATFrame
	 * \return the PDU metadata
	 */
	PDUMetaData createPDUMetaData(std::tuple<uint16_t, int, int> pduInterval, size_t pduOffset)
	{
		PDUMetaData result;
		result.slaveConfiguredAddress = std::get<0>(pduInterval);
		size_t dataLength = std::get<2>(pduInterval) - std::get<1>(pduInterval);
		result.workingCounterOffset = pduOffset + sizeof(EtherCATPDU) - 1 + dataLength;

		// This is kinda ugly and also somewhat inefficient. I just haven't found a nice way
		// to get the required information from the top-level call all the way down here.
		for (int address = std::get<1>(pduInterval); address < std::get<2>(pduInterval); ++address)
		{
			datatypes::RegisterEnum addressAsEnum = static_cast<datatypes::RegisterEnum>(address);
			if (datatypes::registerMap.find(addressAsEnum) != datatypes::registerMap.end())
			{
				result.registerOffsets[addressAsEnum]
				    = address - std::get<1>(pduInterval) + sizeof(EtherCATPDU) - 1 + pduOffset;
			}
		}
		return result;
	}

	/*!
	 * \brief Create an EtherCATFrame that reads all the given address intervals via FPRD
	 * if sent on the EtherCAT bus.
	 * \param slaveAssignedPDUIntervals the PDU intervals to fit in an EtherCATFrame
	 * \return the EtherCATFrame along with its metadata
	 * \exception std::length_error iff the PDUs are too long to fit in an EtherCAT frame
	 */
	std::pair<EtherCATFrame, EtherCATFrameMetaData> createEtherCATFrame(
	    std::vector<std::tuple<uint16_t, int, int>> slaveAssignedPDUIntervals)
	{
		EtherCATFrame frame{};
		EtherCATFrameMetaData metaData{};

		// Remember where we will write the next PDU
		uint8_t* currentLocation = static_cast<uint8_t*>(frame.pduArea);
		for (auto it = slaveAssignedPDUIntervals.begin(); it != slaveAssignedPDUIntervals.end();
		     ++it)
		{
			uint16_t dataLength = std::get<2>(*it) - std::get<1>(*it);
			if (currentLocation + dataLength + pduOverhead // NOLINT
			    >= static_cast<uint8_t*>(frame.pduArea + maxTotalPDULength)) // NOLINT
			{
				throw std::length_error(
				    "Total PDU length exceeded maximum EtherCAT frame capacity.");
			}

			EtherCATPDU* pdu = new (currentLocation) EtherCATPDU; // NOLINT

			pdu->slaveConfiguredAddress = flipBytesIfBigEndianHost(std::get<0>(*it));
			pdu->registerAddress = flipBytesIfBigEndianHost(std::get<1>(*it));

			if (it + 1 != slaveAssignedPDUIntervals.end())
			{
				static constexpr uint16_t hasNextPDU = 1 << 15;
				pdu->dataLengthAndNext = flipBytesIfBigEndianHost(dataLength | hasNextPDU);
			}
			else
			{
				pdu->dataLengthAndNext = flipBytesIfBigEndianHost(dataLength);
			}

			metaData.pdus.push_back(createPDUMetaData(
			    *it, currentLocation - reinterpret_cast<uint8_t*>(&frame))); // NOLINT

			currentLocation += sizeof(EtherCATPDU) - 1 + dataLength + sizeof(uint16_t); // NOLINT
		}
		uint16_t frameLength
		    = currentLocation - static_cast<uint8_t*>(frame.pduArea) + sizeof(uint16_t);
		metaData.lengthOfFrame = frameLength;

		static constexpr uint16_t frameTypePDUs = 0x1000;
		frame.lengthAndType = flipBytesIfBigEndianHost((frameLength - 2) | frameTypePDUs);

		return { frame, metaData };
	}

	/*!
	 * \brief Create an EtherCATFrameList from the given address intervals.
	 *
	 * The frames will be generated to contain every interval for every slave.
	 *
	 * This method will try to fit the PDUs into the frames in order,
	 * starting a new frame if the next PDU doesn't fit. That is not optimal.
	 * If it bugs you, write something smarter.
	 * \param pduIntervals The address intervals to fit into EtherCAT frames
	 */
	EtherCATFrameList* RegisterScheduler::createEtherCATFrameList(
	    const std::vector<std::pair<int, int>>& pduIntervals)
	{
		// We need the slave addresses later when generating the frames,
		// so we add them in here.
		std::vector<std::tuple<uint16_t, int, int>> slaveAssignedPDUIntervals;
		for (uint16_t slaveConfiguredAddress : slaveConfiguredAddresses)
		{
			for (std::pair<int, int> pduInterval : pduIntervals)
			{
				slaveAssignedPDUIntervals.push_back(
				    { slaveConfiguredAddress, pduInterval.first, pduInterval.second });
			}
		}

		EtherCATFrameList* frameList = new EtherCATFrameList(); // NOLINT
		int frameTotalSize = 0;
		std::vector<std::tuple<uint16_t, int, int>> nextFrameIntervals;

		// We add in PDUs until we can't fit the next, then start a new frame.
		for (auto interval : slaveAssignedPDUIntervals)
		{
			int nextIntervalLength = std::get<2>(interval) - std::get<1>(interval);
			if (frameTotalSize + nextIntervalLength + pduOverhead
			    < static_cast<int>(maxTotalPDULength))
			{
				nextFrameIntervals.push_back(interval);
				frameTotalSize += nextIntervalLength + pduOverhead;
			}
			else
			{
				frameList->list.push_back(createEtherCATFrame(nextFrameIntervals));
				nextFrameIntervals.clear();
				frameTotalSize = 0;
			}
		}
		frameList->list.push_back(createEtherCATFrame(nextFrameIntervals));
		return frameList;
	}

} // namespace etherkitten::reader
