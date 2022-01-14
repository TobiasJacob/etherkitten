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
 * \brief Defines structs to hold ESI information with.
 */

#include "ethercatdatatypes.hpp"
#include <optional>
#include <string>
#include <vector>

namespace etherkitten::datatypes
{

	struct ESIHeader
	{
		EtherCATDataType::UNSIGNED16 pdiControl;
		EtherCATDataType::UNSIGNED16 pdiConfiguration;
		EtherCATDataType::UNSIGNED16 syncImpulseLen;
		EtherCATDataType::UNSIGNED16 pdiConfiguration2;
		EtherCATDataType::UNSIGNED16 stationAlias;
		EtherCATDataType::UNSIGNED16 checkSum;
		EtherCATDataType::UNSIGNED32 vendorID;
		EtherCATDataType::UNSIGNED32 productCode;
		EtherCATDataType::UNSIGNED32 revisionNumber;
		EtherCATDataType::UNSIGNED32 serialNumber;
		EtherCATDataType::UNSIGNED16 bootstrapReceiveMailboxOffset;
		EtherCATDataType::UNSIGNED16 bootstrapReceiveMailboxSize;
		EtherCATDataType::UNSIGNED16 bootstrapSendMailboxOffset;
		EtherCATDataType::UNSIGNED16 bootstrapSendMailboxSize;
		EtherCATDataType::UNSIGNED16 standardReceiveMailboxOffset;
		EtherCATDataType::UNSIGNED16 standardReceiveMailboxSize;
		EtherCATDataType::UNSIGNED16 standardSendMailboxOffset;
		EtherCATDataType::UNSIGNED16 standardSendMailboxSize;
		EtherCATDataType::UNSIGNED16 mailboxProtocol;
		EtherCATDataType::UNSIGNED16 eepromSize;
		EtherCATDataType::UNSIGNED16 version;
	};

	enum class MailboxProtocols
	{
		AOE = 0x1,
		EOE = 0x2,
		COE = 0x4,
		FOE = 0x8,
		SOE = 0x10,
		VOE = 0x20,
	};

	using ESIStrings = std::vector<std::string>;

	struct ESIGeneral
	{
		EtherCATDataType::UNSIGNED8 groupIdx;
		EtherCATDataType::UNSIGNED8 imgIdx;
		EtherCATDataType::UNSIGNED8 orderIdx;
		EtherCATDataType::UNSIGNED8 nameIdx;
		EtherCATDataType::UNSIGNED8 coEDetails;
		EtherCATDataType::UNSIGNED8 foEDetails;
		EtherCATDataType::UNSIGNED8 eoEDetails;
		EtherCATDataType::UNSIGNED8 soEChannels;
		EtherCATDataType::UNSIGNED8 dS402Channels;
		EtherCATDataType::UNSIGNED8 sysmanClass;
		EtherCATDataType::UNSIGNED8 flags;
		EtherCATDataType::INTEGER16 currentOnEBus;
		EtherCATDataType::UNSIGNED16 physicalPort;
		EtherCATDataType::UNSIGNED16 physicalMemoryAddress;
		EtherCATDataType::BIT3 identALStatus;
		EtherCATDataType::BIT4 identPhysicalMemoryAddress;
	};

	using ESIFMMU = std::vector<EtherCATDataType::UNSIGNED8>;

	struct ESISyncMElement
	{
		EtherCATDataType::UNSIGNED16 physicalStartAddress;
		EtherCATDataType::UNSIGNED16 length;
		EtherCATDataType::UNSIGNED8 controlRegister;
		EtherCATDataType::UNSIGNED8 statusRegister;
		EtherCATDataType::UNSIGNED8 enableSynchManager;
		EtherCATDataType::UNSIGNED8 syncManagerType;
	};

	using ESISyncM = std::vector<ESISyncMElement>;

	struct ESIPDOEntry
	{
		EtherCATDataType::UNSIGNED16 index;
		EtherCATDataType::UNSIGNED8 subIndex;
		EtherCATDataType::UNSIGNED8 nameIdx;
		EtherCATDataType::UNSIGNED8 dataType;
		EtherCATDataType::UNSIGNED8 bitLength;
		EtherCATDataType::UNSIGNED16 flags;
	};

	struct ESIPDOObject
	{
		EtherCATDataType::UNSIGNED16 pdoIndex;
		EtherCATDataType::UNSIGNED8 entryCount;
		EtherCATDataType::UNSIGNED8 syncManager;
		EtherCATDataType::UNSIGNED8 synchronization;
		EtherCATDataType::UNSIGNED8 nameIdx;
		EtherCATDataType::UNSIGNED16 flags;
		std::vector<ESIPDOEntry> entries;
	};

	using ESITxPDO = std::vector<ESIPDOObject>;
	using ESIRxPDO = std::vector<ESIPDOObject>;

	/*!
	 * \brief Holds ESI data that can be read from slaves via SII.
	 *
	 * For details regarding contents and structure, see ETG1000.6.
	 */
	struct ESIData
	{
		ESIHeader header{};
		ESIStrings strings;
		std::optional<ESIGeneral> general;
		ESIFMMU fmmu;
		ESISyncM syncM;
		ESITxPDO txPDO;
		ESIRxPDO rxPDO;
	};

} // namespace etherkitten::datatypes
