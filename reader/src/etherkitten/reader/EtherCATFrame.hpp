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
 * \brief Defines types and constants related to EtherCAT-type Ethernet frames.
 */

#include <array>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief The maximum length of an Ethernet frame as defined by the standard.
	 */
	static constexpr size_t ethernetFrameMaxLength = 1518;

	/*!
	 * \brief The size of an Ethernet frame header as defined by the standard.
	 */
	static constexpr size_t ethernetHeaderSize = 14;

	/*!
	 * \brief The maximum space that a series of PDUs is allowed to have in an EtherCAT frame.
	 *
	 * (Max. length of eth.) - (eth. header size) - (ecat header size) - (CRC checksum size)
	 */
	static constexpr size_t maxTotalPDULength
	    = ethernetFrameMaxLength - ethernetHeaderSize - sizeof(uint16_t) - sizeof(uint32_t);

	/*!
	 * \brief The EtherCATFrame struct represents an EtherCAT frame according to the EtherCAT spec.
	 *
	 * See the EtherCAT standard, part 4, page 30
	 */
	struct EtherCATFrame
	{
		uint16_t lengthAndType{};
		uint8_t pduArea[maxTotalPDULength]{}; // NOLINT
	};

	static constexpr uint8_t fprdCommandType = 0x04; /*!< The type specifier for an FPRD PDU */

	/*!
	 * \brief This is a placeholder index - SOEM uses the index field of the first PDU to sort
	 * frames, so we have to set this in the SOEM-interacting code where we know which index we
	 * need.
	 */
	static constexpr uint8_t invalidIndex = 0xff;

	/*!
	 * \brief The EtherCATPDU struct represents an EtherCAT PDU according to the EtherCAT spec.
	 *
	 * See the EtherCAT standard, part 4, page 32 (we only use FPRD PDUs here)
	 */
	struct EtherCATPDU
	{
		uint8_t commandType = fprdCommandType;
		uint8_t index = invalidIndex;
		uint16_t slaveConfiguredAddress = 0;
		uint16_t registerAddress = 0;
		uint16_t dataLengthAndNext = 0;
		uint16_t externalEvent = 0;

		/* This is a workaround since C++ doesn't have flexible array members like C.
		 * EtherCATPDUs are only ever placement-constructed inside an EtherCATFrame,
		 * so this doesn't create problems with memory allocation. */
		uint8_t data[1] = { 0 }; // NOLINT
	} __attribute__((packed));

	/*!
	 * \brief The overhead that a PDU has = its size without the data section.
	 *
	 * We use this to decide where to set PDU boundaries to save space in an EtherCAT frame.
	 *
	 * (Size of the PDU struct) - (That annoying workaround byte) + (The working counter at the end)
	 */
	static constexpr int pduOverhead = sizeof(EtherCATPDU) - sizeof(uint8_t) + sizeof(uint16_t);

	/*!
	 * \brief The PDUMetaData struct holds information about the structure of a PDU.
	 *
	 * We need this if we don't want to reparse the frames once we receive to get the data out.
	 */
	struct PDUMetaData
	{
		uint16_t slaveConfiguredAddress;

		/*!
		 * \brief The offsets of the registers that are written into this PDU by the slave
		 * relative to the start of the EtherCAT frame.
		 */
		std::unordered_map<datatypes::RegisterEnum, size_t> registerOffsets;

		/*!
		 * \brief The offset of the working counter for this PDU relative to the start of
		 * the EtherCAT frame.
		 */
		size_t workingCounterOffset;
	};

	/*!
	 * \brief The EtherCATFrameMetaData struct holds information about the structure of an
	 * EtherCAT frame.
	 */
	struct EtherCATFrameMetaData
	{
		size_t lengthOfFrame;
		std::vector<PDUMetaData> pdus;
	};

	/*!
	 * \brief The EtherCATFrameList struct holds a list of EtherCAT frames that can be
	 * scheduled in round-robin-style.
	 */
	struct EtherCATFrameList
	{
		size_t nextIndex = 0; /*!< The next frame in the RR scheduler */
		std::vector<std::pair<EtherCATFrame, EtherCATFrameMetaData>> list;
	};

	/*!
	 * \brief The EtherCATFrameIterator class iterates over a set range of EtherCAT frames
	 * once.
	 */
	class EtherCATFrameIterator
	{
	public:
		/*!
		 * \brief Construct a new EtherCATFrameIterator that iterates over the given
		 * range of EtherCAT frames.
		 *
		 * If `startIndex + count > list->list.size()`, the iterator will wrap around
		 * to the start fo the list.
		 * \param list the list to iterate over
		 * \param startIndex the index to start iterating on
		 * \param count the number of frames to iterate over (including the first)
		 */
		EtherCATFrameIterator(EtherCATFrameList* list, size_t startIndex, size_t count);

		/*!
		 * \brief Get the EtherCAT frame this iterator is on with metadata.
		 * \return an EtherCATFrame along with its metadata
		 */
		std::pair<EtherCATFrame*, EtherCATFrameMetaData*> operator*() const;

		/*!
		 * \brief Check if this iterator points to the first available register frame.
		 *
		 * This indicates that all registers have been read by sequential iterators.
		 * \retval true iff this iterator points to the first register frame
		 * \retval false iff this iterator does not point to the first register frame
		 * \exception
		 */
		bool hasCompletedLoop() const;

		/*!
		 * \brief Move this iterator to the next EtherCAT frame.
		 *
		 * Does nothing if this iterator is at the end of its range.
		 * \return a reference to this iterator
		 */
		EtherCATFrameIterator& operator++();

		/*!
		 * \brief Check if this iterator has stepped over its range.
		 * \retval true iff this iterator can no longer be dereferenced or advanced
		 * \retval false iff this iterator can still be dereferenced and advanced
		 */
		bool atEnd() const;

	private:
		EtherCATFrameList* list;
		size_t nextIndex;
		size_t remaining;
	};

} // namespace etherkitten::reader
