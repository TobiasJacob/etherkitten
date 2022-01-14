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

#include "SearchListReader.hpp"

#include "endianness.hpp"

namespace etherkitten::reader
{
	SearchListReader::SearchListReader(std::vector<uint16_t>&& slaveConfiguredAddresses,
	    size_t ioMapUsedSize, datatypes::TimeStamp&& startTime)
	    : slaveConfiguredAddresses(slaveConfiguredAddresses)
	    , maximumMemory(0)
	    , currentMemoryUsage(0)
	    , ioMapUsedSize(ioMapUsedSize)
	    , startTime(startTime)

	{
		for (uint16_t slave : slaveConfiguredAddresses)
		{
			registerLists.emplace(
			    std::piecewise_construct, std::forward_as_tuple(slave), std::forward_as_tuple());
			for (const auto& reg : datatypes::registerMap)
			{
				static constexpr int twoByteMask = 0xFFFF;
				int registerAddress = static_cast<int>(reg.first) & twoByteMask;
				datatypes::RegisterEnum addressAsRegister
				    = static_cast<datatypes::RegisterEnum>(registerAddress);
				if (registerLists[slave].find(addressAsRegister) != registerLists[slave].end())
				{
					continue;
				}
				switch (datatypes::getRegisterByteLength(addressAsRegister))
				{
				case 1:
					registerLists[slave].emplace(addressAsRegister,
					    std::in_place_type<
					        SearchList<datatypes::EtherCATDataType::UNSIGNED8, nodeSize>>);
					break;
				case 2:
					registerLists[slave].emplace(addressAsRegister,
					    std::in_place_type<
					        SearchList<datatypes::EtherCATDataType::UNSIGNED16, nodeSize>>);
					break;
				case 4:
					registerLists[slave].emplace(addressAsRegister,
					    std::in_place_type<
					        SearchList<datatypes::EtherCATDataType::UNSIGNED32, nodeSize>>);
					break;
				case 8: // NOLINT
					registerLists[slave].emplace(addressAsRegister,
					    std::in_place_type<
					        SearchList<datatypes::EtherCATDataType::UNSIGNED64, nodeSize>>);
					break;
				}
			}
		}
	}

	SearchListReader::~SearchListReader(){};

	std::unique_ptr<datatypes::AbstractNewestValueView> SearchListReader::getNewest(
	    const datatypes::PDO& pdo)
	{
		datatypes::PDOInfo info = getAbsolutePDOInfo(pdo);
		return datatypes::dataTypeMap<bReader::NewestValueViewRetriever, std::unique_ptr<IOMap>,
			bReader::SizeT2Type<nodeSize>>.at(
		    pdo.getType())(ioMapList, info.bitOffset, info.bitLength);
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> SearchListReader::getNewest(
	    const datatypes::Register& reg)
	{
		return bReader::makeRegisterView<std::unique_ptr<datatypes::AbstractNewestValueView>,
		    bReader::SizeT2Type<nodeSize>>(registerLists, reg, { datatypes::TimeStamp(), 0s },
		    slaveConfiguredAddresses[reg.getSlaveID() - 1]);
	}

	std::shared_ptr<datatypes::AbstractDataView> SearchListReader::getView(
	    const datatypes::PDO& pdo, datatypes::TimeSeries time)
	{
		datatypes::PDOInfo info = getAbsolutePDOInfo(pdo);
		return datatypes::dataTypeMap<bReader::DataViewRetriever, std::unique_ptr<IOMap>,
			bReader::SizeT2Type<nodeSize>>.at(
		    pdo.getType())(ioMapList, time, info.bitOffset, info.bitLength);
	}

	std::shared_ptr<datatypes::AbstractDataView> SearchListReader::getView(
	    const datatypes::Register& reg, datatypes::TimeSeries time)
	{
		return bReader::makeRegisterView<std::shared_ptr<datatypes::AbstractDataView>,
		    bReader::SizeT2Type<nodeSize>>(
		    registerLists, reg, time, slaveConfiguredAddresses[reg.getSlaveID() - 1]);
	}

	std::shared_ptr<DataView<std::unique_ptr<IOMap>, Reader::nodeSize, IOMap*>>
	SearchListReader::getIOMapView(datatypes::TimeStamp startTime)
	{
		datatypes::TimeSeries timeSeries{ startTime, datatypes::TimeStep(0) };
		return ioMapList.getView<IOMap*>(timeSeries, false);
	}

	std::shared_ptr<datatypes::AbstractDataView> SearchListReader::getRegisterRawDataView(
	    const uint16_t slaveId, const uint16_t regId, datatypes::TimeSeries time)
	{
		datatypes::RegisterEnum addressAsRegister = static_cast<datatypes::RegisterEnum>(regId);
		auto& searchListVariant
		    = registerLists.at(slaveConfiguredAddresses.at(slaveId - 1)).at(addressAsRegister);
		switch (datatypes::getRegisterByteLength(addressAsRegister))
		{
		case 1:
			return std::get<0>(searchListVariant).getView(time, false);
		case 2:
			return std::get<1>(searchListVariant).getView(time, false);
		case 4:
			return std::get<2>(searchListVariant).getView(time, false);
		case 8: // NOLINT
			return std::get<3>(searchListVariant).getView(time, false);
		default:
			throw std::runtime_error("bytelength is not a valid register bytelength");
		}
	}

	void SearchListReader::insertIOMap(std::unique_ptr<IOMap> ioMap, datatypes::TimeStamp&& time)
	{
		currentMemoryUsage += sizeof(LLNode<std::unique_ptr<IOMap>, nodeSize>) / nodeSize
		    + sizeof(IOMap) + ioMap->ioMapSize - 1;
		ioMapList.append(std::move(ioMap), time);
		pdoTimeStamps.add(time);
	}

	void SearchListReader::insertRegister(datatypes::RegisterEnum registerType, uint8_t* dataPtr,
	    uint16_t slaveConfiguredAddress, datatypes::TimeStamp& time)
	{
		auto& list = registerLists.at(slaveConfiguredAddress).at(registerType);
		uint64_t result = 0;
		switch (getRegisterByteLength(registerType))
		{
		case 1:
			std::get<0>(list).append(flipBytesIfBigEndianHost(*dataPtr), time);
			currentMemoryUsage
			    += sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED8, nodeSize>) / nodeSize;
			break;
		case 2:
			std::memcpy(&result, dataPtr, sizeof(datatypes::EtherCATDataType::UNSIGNED16));
			std::get<1>(list).append(flipBytesIfBigEndianHost(result), time);
			currentMemoryUsage
			    += sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED16, nodeSize>) / nodeSize;
			break;
		case 4:
			std::memcpy(&result, dataPtr, sizeof(datatypes::EtherCATDataType::UNSIGNED32));
			std::get<2>(list).append(flipBytesIfBigEndianHost(result), time);
			currentMemoryUsage
			    += sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED32, nodeSize>) / nodeSize;
			break;
		case 8: // NOLINT
			std::memcpy(&result, dataPtr, sizeof(datatypes::EtherCATDataType::UNSIGNED64));
			std::get<3>(list).append(flipBytesIfBigEndianHost(result), time);
			currentMemoryUsage
			    += sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED64, nodeSize>) / nodeSize;
			break;
		}
	}

	double SearchListReader::getFrequency(
	    RingBuffer<datatypes::TimeStamp, frequencyAveragerCount>& buffer)
	{
		if (buffer.isEmpty())
		{
			return 0;
		}
		std::chrono::nanoseconds step;
		do
		{
			step = buffer.getNewestElement() - buffer.getOldestElement();
			// This loop exists due to a race condition in RingBuffer. See its docs.
		} while (step.count() < 0);
		static constexpr uint64_t nanosPerSecond = 1000000000;
		return static_cast<double>(buffer.getSize()) / step.count() * nanosPerSecond;
	}

	double SearchListReader::getPDOFrequency() { return getFrequency(pdoTimeStamps); }

	double SearchListReader::getRegisterFrequency() { return getFrequency(registerTimeStamps); }

	void SearchListReader::setMaximumMemory(size_t size)
	{
		maximumMemory.store(size, std::memory_order_release);
	}

	void SearchListReader::freeMemoryIfNecessary()
	{
		size_t maxMemory = maximumMemory.load(std::memory_order_acquire);
		if (maxMemory > 0)
		{
			float memoryUsageQuota = currentMemoryUsage / static_cast<float>(maxMemory);
			if (memoryUsageQuota > quotaUsageBeforeDeletion)
			{
				size_t toFree = (1 - quotaUsageBeforeDeletion) * maxMemory * 2;
				currentMemoryUsage -= freeMemory(toFree);
			}
		}
	}

	size_t SearchListReader::freeMemory(size_t toFree)
	{
		// Value chosen after profiling with Massif
		static constexpr float pdoQuota = 0.05;
		size_t freeFromIOMap = pdoQuota * toFree;
		size_t freeFromRegisters = (1 - pdoQuota) * toFree;

		// Space for the node + pointer + space for IOMap + space for IOMap metadata (-1 for first
		// array element)
		size_t singleIOMapNodeSpace = sizeof(LLNode<std::unique_ptr<IOMap>, nodeSize>)
		    + nodeSize * (ioMapUsedSize + sizeof(IOMap) - 1);
		size_t ioMapsToFree = freeFromIOMap / singleIOMapNodeSpace;

		int freedIOMaps = ioMapList.removeOldest(ioMapsToFree);
		// Try to reclaim remaining space from registers
		freeFromRegisters += (ioMapsToFree - freedIOMaps) * singleIOMapNodeSpace;

		// This approach does not always perform optimally and does not free registers equally
		// across slaves or among one slave. It should do well enough as long as there are
		// not too many view though.
		size_t remainingToFreeFromRegisters = freeFromRegisters;
		size_t remainingSlaves = slaveConfiguredAddresses.size();
		size_t freeFromRegistersPerSlave = remainingToFreeFromRegisters / remainingSlaves;
		for (auto& slaveRegisterLists : registerLists)
		{
			size_t remainingToFreeFromSlave = freeFromRegistersPerSlave;
			size_t remainingRegisters = slaveRegisterLists.second.size();
			size_t freePerRegister = remainingToFreeFromSlave / remainingRegisters;
			for (auto& registerList : slaveRegisterLists.second)
			{
				size_t sizeOfRegister = 0;
				switch (getRegisterByteLength(registerList.first))
				{
				case 1:
					sizeOfRegister
					    = sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED8, nodeSize>);
					break;
				case 2:
					sizeOfRegister
					    = sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED16, nodeSize>);
					break;
				case 4:
					sizeOfRegister
					    = sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED32, nodeSize>);
					break;
				case 8: // NOLINT
					sizeOfRegister
					    = sizeof(LLNode<datatypes::EtherCATDataType::UNSIGNED64, nodeSize>);
					break;
				}
				size_t registersToFree = freePerRegister / sizeOfRegister;
				int freedRegisters = std::visit(
				    [registersToFree](auto& list) { return list.removeOldest(registersToFree); },
				    registerList.second);
				size_t freedFromRegister = freedRegisters * sizeOfRegister;
				remainingToFreeFromSlave -= freedFromRegister;
				if (--remainingRegisters > 0)
				{
					freePerRegister = remainingToFreeFromSlave / remainingRegisters;
				}
			}
			remainingToFreeFromRegisters -= (freeFromRegistersPerSlave - remainingToFreeFromSlave);
			if (--remainingSlaves > 0)
			{
				freeFromRegistersPerSlave = remainingToFreeFromRegisters / remainingSlaves;
			}
		}
		return toFree - remainingToFreeFromRegisters;
	}

	void SearchListReader::insertNewRegisterTimeStamp(datatypes::TimeStamp& time)
	{
		registerTimeStamps.add(time);
	}

	datatypes::TimeStamp SearchListReader::getStartTime() const { return startTime; }
} // namespace etherkitten::reader
