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

#include "esiparser.hpp"
#include <vector>

namespace etherkitten::datatypes::esiparser
{
	EtherCATDataType::UNSIGNED16 getU16W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress);
	EtherCATDataType::UNSIGNED32 getU32W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress);
	ESIHeader parseHeader(const std::vector<std::byte>& esiBinary);
	std::vector<std::string> parseStrings(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset);
	ESIGeneral parseGeneral(const std::vector<std::byte>& esiBinary, size_t wordOffset);
	ESIFMMU parseFMMU(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);
	ESISyncM parseSyncM(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);
	std::vector<ESIPDOObject> parsePDOs(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len);

	ESIData parseESI(const std::vector<std::byte>& esiBinary)
	{
		ESIData esiData{};
		esiData.header = parseHeader(esiBinary);
		size_t categoryOffset = 0x0040;
		auto categoryType = getU16W(esiBinary, categoryOffset);
		// Attention - the MSB is undefined! -> setting to zero.
		categoryType &= (1 << 15) - 1;
		auto categorySize = getU16W(esiBinary, categoryOffset + 1);
		while (categoryType != 0x7fff)
		{
			switch (categoryType)
			{
			case 0:
				// NOP
				break;
			// 1 - 9 are device specific categories -> ignored
			case 10:
				// STRINGS
				esiData.strings = parseStrings(esiBinary, categoryOffset + 2);
				break;
			case 20:
				// DataTypes
				// "for future use" -> ignored
				break;
			case 30:
				// General
				esiData.general = parseGeneral(esiBinary, categoryOffset + 2);
				break;
			case 40:
				// FMMU
				esiData.fmmu = parseFMMU(esiBinary, categoryOffset + 2, categorySize);
				break;
			case 41:
				// SyncM
				esiData.syncM = parseSyncM(esiBinary, categoryOffset + 2, categorySize);
				break;
			case 50:
				// TxPDO
				esiData.txPDO = parsePDOs(esiBinary, categoryOffset + 2, categorySize);
				break;
			case 51:
				// RxPDO
				esiData.rxPDO = parsePDOs(esiBinary, categoryOffset + 2, categorySize);
				break;
				// 60+ are reserved or vendor specific -> ignored
			}
			categoryOffset += 2 + categorySize;
			categoryType = getU16W(esiBinary, categoryOffset);
			// Attention - the MSB is undefined! -> setting to zero.
			categoryType &= (1 << 15) - 1;
			categorySize = getU16W(esiBinary, categoryOffset + 1);
		}
		return esiData;
	}

	EtherCATDataType::UNSIGNED8 getU8(const std::vector<std::byte>& esiBinary, size_t address)
	{
		if (esiBinary.size() < address + 1)
			throw ParseException("The ESI binary ended unexpectedly!");
		return static_cast<EtherCATDataType::UNSIGNED8>(esiBinary.at(address));
	}

	EtherCATDataType::UNSIGNED16 getU16(const std::vector<std::byte>& esiBinary, size_t address)
	{
		if (esiBinary.size() < address + 2)
			throw ParseException("The ESI binary ended unexpectedly!");
		const auto byteSize = 8;
		return static_cast<EtherCATDataType::UNSIGNED16>(esiBinary.at(address))
		    + (static_cast<EtherCATDataType::UNSIGNED16>(esiBinary.at(address + 1)) << byteSize);
	}

	EtherCATDataType::INTEGER16 get16(const std::vector<std::byte>& esiBinary, size_t address)
	{
		auto unsignedValue = getU16(esiBinary, address);
		return *reinterpret_cast<EtherCATDataType::INTEGER16*>(&unsignedValue);
	}

	EtherCATDataType::UNSIGNED16 getU16W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress)
	{
		return getU16(esiBinary, wordAddress * 2);
	}

	EtherCATDataType::UNSIGNED32 getU32(const std::vector<std::byte>& esiBinary, size_t address)
	{
		if (esiBinary.size() < address + 4)
			throw ParseException("The ESI binary ended unexpectedly!");
		const auto byteSize = 8;
		return static_cast<EtherCATDataType::UNSIGNED32>(esiBinary.at(address))
		    + (static_cast<EtherCATDataType::UNSIGNED32>(esiBinary.at(address + 1)) << byteSize)
		    + (static_cast<EtherCATDataType::UNSIGNED32>(esiBinary.at(address + 2)) << 2 * byteSize)
		    + (static_cast<EtherCATDataType::UNSIGNED32>(esiBinary.at(address + 3))
		          << 3 * byteSize);
	}

	EtherCATDataType::UNSIGNED32 getU32W(
	    const std::vector<std::byte>& esiBinary, size_t wordAddress)
	{
		return getU32(esiBinary, wordAddress * 2);
	}

	ESIHeader parseHeader(const std::vector<std::byte>& esiBinary)
	{
		ESIHeader esiHeader;
		esiHeader.pdiControl = getU16W(esiBinary, 0x0000);
		esiHeader.pdiConfiguration = getU16W(esiBinary, 0x0001);
		esiHeader.syncImpulseLen = getU16W(esiBinary, 0x0002);
		esiHeader.pdiConfiguration2 = getU16W(esiBinary, 0x0003);
		esiHeader.stationAlias = getU16W(esiBinary, 0x0004);
		esiHeader.checkSum = getU16W(esiBinary, 0x0007);
		esiHeader.vendorID = getU32W(esiBinary, 0x0008);
		esiHeader.productCode = getU32W(esiBinary, 0x000A);
		esiHeader.revisionNumber = getU32W(esiBinary, 0x000C);
		esiHeader.serialNumber = getU32W(esiBinary, 0x000E);
		esiHeader.bootstrapReceiveMailboxOffset = getU16W(esiBinary, 0x0014);
		esiHeader.bootstrapReceiveMailboxSize = getU16W(esiBinary, 0x0015);
		esiHeader.bootstrapSendMailboxOffset = getU16W(esiBinary, 0x0016);
		esiHeader.bootstrapSendMailboxSize = getU16W(esiBinary, 0x0017);
		esiHeader.standardReceiveMailboxOffset = getU16W(esiBinary, 0x0018);
		esiHeader.standardReceiveMailboxSize = getU16W(esiBinary, 0x0019);
		esiHeader.standardSendMailboxOffset = getU16W(esiBinary, 0x001A);
		esiHeader.standardSendMailboxSize = getU16W(esiBinary, 0x001B);
		esiHeader.mailboxProtocol = getU16W(esiBinary, 0x001C);
		esiHeader.eepromSize = getU16W(esiBinary, 0x003E);
		esiHeader.version = getU16W(esiBinary, 0x003F);
		return esiHeader;
	}

	std::vector<std::string> parseStrings(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset)
	{
		std::vector<std::string> strings;
		auto nStrings = getU8(esiBinary, 2 * wordOffset);
		strings.reserve(nStrings);
		size_t currentOffset = 2 * wordOffset + 1;
		for (auto i = 0; i < nStrings; ++i)
		{
			EtherCATDataType::UNSIGNED8 len;
			std::string tmp;
			len = static_cast<EtherCATDataType::UNSIGNED8>(esiBinary.at(currentOffset));
			++currentOffset;
			if (currentOffset + len > esiBinary.size())
				throw ParseException("The ESI binary ended unexpectedly while reading a string!");
			tmp.assign((char*)&esiBinary[currentOffset], len);
			strings.emplace_back(tmp);
			currentOffset += len;
		}
		return strings;
	}

	ESIGeneral parseGeneral(const std::vector<std::byte>& esiBinary, size_t wordOffset)
	{
		ESIGeneral esiGeneral;
		size_t offset = 2 * wordOffset;
		esiGeneral.groupIdx = getU8(esiBinary, offset + 0x0000);
		esiGeneral.imgIdx = getU8(esiBinary, offset + 0x0001);
		esiGeneral.orderIdx = getU8(esiBinary, offset + 0x0002);
		esiGeneral.nameIdx = getU8(esiBinary, offset + 0x0003);
		esiGeneral.coEDetails = getU8(esiBinary, offset + 0x0005);
		esiGeneral.foEDetails = getU8(esiBinary, offset + 0x0006);
		esiGeneral.eoEDetails = getU8(esiBinary, offset + 0x0007);
		esiGeneral.soEChannels = getU8(esiBinary, offset + 0x0008);
		esiGeneral.dS402Channels = getU8(esiBinary, offset + 0x0009);
		esiGeneral.sysmanClass = getU8(esiBinary, offset + 0x000a);
		esiGeneral.flags = getU8(esiBinary, offset + 0x000b);
		esiGeneral.currentOnEBus = get16(esiBinary, offset + 0x000c);
		esiGeneral.physicalPort = getU16(esiBinary, offset + 0x0010);
		esiGeneral.physicalMemoryAddress = getU16(esiBinary, offset + 0x0012);
		return esiGeneral;
	}

	ESIFMMU parseFMMU(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len)
	{
		size_t count = 2 * len;
		ESIFMMU esiFmmu;
		esiFmmu.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			esiFmmu.emplace_back(getU8(esiBinary, 2 * wordOffset + i));
		}
		return esiFmmu;
	}

	ESISyncM parseSyncM(const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len)
	{
		const size_t syncMElementLen = 8;
		const size_t count = 2 * len / syncMElementLen;
		ESISyncM esiSyncM;
		esiSyncM.reserve(count);
		size_t currentOffset = 2 * wordOffset;
		for (size_t i = 0; i < count; ++i)
		{
			ESISyncMElement elem;
			elem.physicalStartAddress = getU16(esiBinary, currentOffset + 0x0000);
			elem.length = getU16(esiBinary, currentOffset + 0x0002);
			elem.controlRegister = getU8(esiBinary, currentOffset + 0x0004);
			elem.statusRegister = getU8(esiBinary, currentOffset + 0x0005);
			elem.enableSynchManager = getU8(esiBinary, currentOffset + 0x0006);
			elem.syncManagerType = getU8(esiBinary, currentOffset + 0x0007);
			esiSyncM.emplace_back(elem);
			currentOffset += syncMElementLen;
		}
		return esiSyncM;
	}

	std::vector<ESIPDOObject> parsePDOs(
	    const std::vector<std::byte>& esiBinary, size_t wordOffset, size_t len)
	{
		std::vector<ESIPDOObject> esiPDOs;
		size_t currentOffset = 2 * wordOffset;
		while (currentOffset < 2 * (wordOffset + len))
		{
			ESIPDOObject pdoObject;
			pdoObject.pdoIndex = getU16(esiBinary, currentOffset + 0x0000);
			pdoObject.entryCount = getU8(esiBinary, currentOffset + 0x0002);
			pdoObject.syncManager = getU8(esiBinary, currentOffset + 0x0003);
			pdoObject.synchronization = getU8(esiBinary, currentOffset + 0x0004);
			pdoObject.nameIdx = getU8(esiBinary, currentOffset + 0x0005);
			pdoObject.flags = getU16(esiBinary, currentOffset + 0x0006);
			pdoObject.entries = std::vector<ESIPDOEntry>{ pdoObject.entryCount };
			currentOffset += 8;
			for (int i = 0; i < pdoObject.entryCount; ++i)
			{
				ESIPDOEntry pdoEntry;
				pdoEntry.index = getU16(esiBinary, currentOffset + 0x0000);
				pdoEntry.subIndex = getU8(esiBinary, currentOffset + 0x0002);
				pdoEntry.nameIdx = getU8(esiBinary, currentOffset + 0x0003);
				pdoEntry.dataType = getU8(esiBinary, currentOffset + 0x0004);
				pdoEntry.bitLength = getU8(esiBinary, currentOffset + 0x0005);
				pdoEntry.flags = getU16(esiBinary, currentOffset + 0x0006);
				pdoObject.entries.emplace_back(pdoEntry);
				currentOffset += 8;
			}
			esiPDOs.emplace_back(pdoObject);
		}
		return esiPDOs;
	}
} // namespace etherkitten::datatypes::esiparser
