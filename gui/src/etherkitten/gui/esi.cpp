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

#include "esi.hpp"
#include <QApplication>

/* Nasty stuff - cover your eyes */

namespace etherkitten::gui
{

	using datatypes::EtherCATDataType::BIT3;
	using datatypes::EtherCATDataType::BIT4;
	using datatypes::EtherCATDataType::INTEGER16;
	using datatypes::EtherCATDataType::UNICODE_STRING;
	using datatypes::EtherCATDataType::UNSIGNED16;
	using datatypes::EtherCATDataType::UNSIGNED32;
	using datatypes::EtherCATDataType::UNSIGNED8;

	std::string ESIAdapter::getString(size_t index, const datatypes::ESIStrings& strings)
	{
		if (index < strings.size())
			return strings.at(index);
		else
			return "INVALID STRING INDEX";
	}

	std::vector<std::unique_ptr<AbstractESIDataWrapper>> ESIAdapter::convertESIData(
	    const datatypes::ESIData& data)
	{
		std::vector<std::unique_ptr<AbstractESIDataWrapper>> conv;
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("PDI Control",
		    data.header.pdiControl, QApplication::translate("ESIAdapter", "ESI:PDI Control")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("PDI Configuration",
		    data.header.pdiConfiguration,
		    QApplication::translate("ESIAdapter", "ESI:PDI Configuration")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("SyncImpulseLen",
		    data.header.syncImpulseLen,
		    QApplication::translate("ESIAdapter", "ESI:SyncImpulseLen")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("PDI Configuration2",
		    data.header.pdiConfiguration2,
		    QApplication::translate("ESIAdapter", "ESI:PDI Configuration2")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("Configured Station Alias",
		    data.header.stationAlias, QApplication::translate("ESIAdapter", "ESI:Alias Address")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("Checksum",
		    data.header.checkSum, QApplication::translate("ESIAdapter", "ESI:Checksum")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED32>>("Vendor ID",
		    data.header.vendorID, QApplication::translate("ESIAdapter", "ESI:Vendor ID")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED32>>("Product Code",
		    data.header.productCode, QApplication::translate("ESIAdapter", "ESI:Product Code")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED32>>("Revision Number",
		    data.header.revisionNumber,
		    QApplication::translate("ESIAdapter", "ESI:Revision Number")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED32>>("Serial Number",
		    data.header.serialNumber, QApplication::translate("ESIAdapter", "ESI:Serial Number")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Bootstrap Receive Mailbox Offset", data.header.bootstrapReceiveMailboxOffset,
		    QApplication::translate("ESIAdapter", "ESI:Bootstrap Receive Mailbox Offset")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Bootstrap Receive Mailbox Size", data.header.bootstrapReceiveMailboxSize,
		    QApplication::translate("ESIAdapter", "ESI:Bootstrap Receive Mailbox Size")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Bootstrap Send Mailbox Offset", data.header.bootstrapSendMailboxOffset,
		    QApplication::translate("ESIAdapter", "ESI:Bootstrap Send Mailbox Offset")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Bootstrap Send Mailbox Size", data.header.bootstrapSendMailboxSize,
		    QApplication::translate("ESIAdapter", "ESI:Bootstrap Send Mailbox Size")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Standard Receive Mailbox Offset", data.header.standardReceiveMailboxOffset,
		    QApplication::translate("ESIAdapter", "ESI:Standard Receive Mailbox Offset")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Standard Receive Mailbox Size", data.header.standardReceiveMailboxSize,
		    QApplication::translate("ESIAdapter", "ESI:Standard Receive Mailbox Size")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
		    "Standard Send Mailbox Offset", data.header.standardSendMailboxOffset,
		    QApplication::translate("ESIAdapter", "ESI:Standard Send Mailbox Offset")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("Standard Send Mailbox Size",
		    data.header.standardSendMailboxSize,
		    QApplication::translate("ESIAdapter", "ESI:Standard Send Mailbox Size")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("Mailbox Protocol",
		    data.header.mailboxProtocol,
		    QApplication::translate("ESIAdapter", "ESI:Mailbox Protocol")));
		conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("EEPROM Size",
		    data.header.eepromSize, QApplication::translate("ESIAdapter", "ESI:EEPROM Size")));
		conv.emplace_back(
		    std::make_unique<ESIDataWrapper<UNSIGNED16>>("Version", data.header.version, ""));
		if (data.general)
		{
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>("Group",
			    getString(data.general.value().groupIdx, data.strings),
			    QApplication::translate("ESIAdapter", "ESI:General:Group")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>("Img",
			    getString(data.general.value().imgIdx, data.strings),
			    QApplication::translate("ESIAdapter", "ESI:General:Img")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>("Order",
			    getString(data.general.value().orderIdx, data.strings),
			    QApplication::translate("ESIAdapter", "ESI:General:Order")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>("Name",
			    getString(data.general.value().nameIdx, data.strings),
			    QApplication::translate("ESIAdapter", "ESI:General:Name")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>("CoE Details",
			    data.general.value().coEDetails,
			    QApplication::translate("ESIAdapter", "ESI:General:CoE Details")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>("FoE Details",
			    data.general.value().foEDetails,
			    QApplication::translate("ESIAdapter", "ESI:General:FoE Details")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>("EoE Details",
			    data.general.value().eoEDetails,
			    QApplication::translate("ESIAdapter", "ESI:General:EoE Details")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    "SoEChannels", data.general.value().soEChannels, ""));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    "DS402Channels", data.general.value().dS402Channels, ""));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    "SysmanClass", data.general.value().sysmanClass, ""));
			conv.emplace_back(
			    std::make_unique<ESIDataWrapper<UNSIGNED8>>("Flags", data.general.value().flags,
			        QApplication::translate("ESIAdapter", "ESI:General:Flags")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<INTEGER16>>("CurrentOnEBus",
			    data.general.value().currentOnEBus,
			    QApplication::translate("ESIAdapter", "ESI:General:CurrentOnEBus")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>("Physical Port",
			    data.general.value().physicalPort,
			    QApplication::translate("ESIAdapter", "ESI:General:Physical Port")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
			    "Physical Memory Address", data.general.value().physicalMemoryAddress,
			    QApplication::translate("ESIAdapter", "ESI:General:Physical Memory Address")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<BIT3>>("ESI:General:IdentALSts",
			    data.general.value().identALStatus,
			    QApplication::translate("ESIAdapter", "ESI:General:IdentALSts")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<BIT4>>("ESI:General:IdentPhyM",
			    data.general.value().identPhysicalMemoryAddress,
			    QApplication::translate("ESIAdapter", "ESI:General:IdentPhyM")));
		}
		for (size_t i = 0; i < data.fmmu.size(); i++)
		{
			conv.emplace_back(
			    std::make_unique<ESIDataWrapper<UNSIGNED8>>("FMMU" + std::to_string(i),
			        data.fmmu[i], QApplication::translate("ESIAdapter", "ESI:FMMU")));
		}
		for (size_t i = 0; i < data.syncM.size(); i++)
		{
			std::string name = "SyncM" + std::to_string(i);
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
			    name + " Physical Start Address", data.syncM[i].physicalStartAddress,
			    QApplication::translate("ESIAdapter", "ESI:SyncM:Physical Start Address")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
			    name + " Length", data.syncM[i].length, ""));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    name + " Control Register", data.syncM[i].controlRegister,
			    QApplication::translate("ESIAdapter", "ESI:SyncM:Control Register")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    name + " Status Register", data.syncM[i].statusRegister, ""));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    name + " Enable Synch Manager", data.syncM[i].enableSynchManager,
			    QApplication::translate("ESIAdapter", "ESI:SyncM:Enable Synch Manager")));
			conv.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    name + " Sync Manager Type", data.syncM[i].syncManagerType,
			    QApplication::translate("ESIAdapter", "ESI:SyncM:Sync Manager Type")));
		}
		addPDO(conv, data.txPDO, data, "Tx");
		addPDO(conv, data.rxPDO, data, "Rx");
		return conv;
	}

	/* NOTE: This is not nicely hierarchical as it could technically be, mainly because
	 * that would make it much more complicated, especially when the SlaveTree has to
	 * determine which actual entry is under the mouse pointer when it is clicked */
	void ESIAdapter::addPDO(std::vector<std::unique_ptr<AbstractESIDataWrapper>>& entries,
	    const std::vector<datatypes::ESIPDOObject>& pdos, const datatypes::ESIData& data,
	    std::string prefix)
	{
		for (size_t i = 0; i < pdos.size(); i++)
		{
			std::string name = prefix + "PDO" + std::to_string(i);
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
			    name + " PDO Index", pdos[i].pdoIndex, ""));
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(name + " nEntry",
			    pdos[i].entryCount, QApplication::translate("ESIAdapter", "ESI:PDO:nEntry")));
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(name + " SyncM",
			    pdos[i].syncManager, QApplication::translate("ESIAdapter", "ESI:PDO:SyncM")));
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
			    name + " Synchronization", pdos[i].synchronization,
			    QApplication::translate("ESIAdapter", "ESI:PDO:Sync")));
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>(name + " Name",
			    getString(pdos[i].nameIdx, data.strings),
			    QApplication::translate("ESIAdapter", "ESI:PDO:Name")));
			entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(name + " Flags",
			    pdos[i].flags, QApplication::translate("ESIAdapter", "ESI:PDO:Flags")));
			for (size_t j = 0; j < pdos[i].entries.size(); j++)
			{
				std::string entryName = name + " Entry" + std::to_string(j);
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
				    entryName + " Index", pdos[i].entries[j].index,
				    QApplication::translate("ESIAdapter", "ESI:PDO:Entry:Index")));
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
				    entryName + " Subindex", pdos[i].entries[j].subIndex, ""));
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNICODE_STRING>>(
				    entryName + " Name", getString(pdos[i].entries[j].nameIdx, data.strings),
				    QApplication::translate("ESIAdapter", "ESI:PDO:Entry:Name")));
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
				    entryName + " Data Type", pdos[i].entries[j].dataType,
				    QApplication::translate("ESIAdapter", "ESI:PDO:Entry:Data Type")));
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED8>>(
				    entryName + " Bit Length", pdos[i].entries[j].bitLength,
				    QApplication::translate("ESIAdapter", "ESI:PDO:Entry:Bit Length")));
				entries.emplace_back(std::make_unique<ESIDataWrapper<UNSIGNED16>>(
				    entryName + " Flags", pdos[i].entries[j].flags,
				    QApplication::translate("ESIAdapter", "ESI:PDO:Entry:Flags")));
			}
		}
	}

} // namespace etherkitten::gui
