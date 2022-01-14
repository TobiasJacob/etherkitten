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


#include "BusReader.hpp"

extern "C"
{
#include <ethercat.h>
}

#include "BusReader-impl/busfunctions.hpp"
#include "BusReader-impl/rtfuncs.hpp"
#include "viewtemplates.hpp"

namespace etherkitten::reader
{
	using namespace bReader;

	BusReader::BusReader(BusSlaveInformant& slaveInformant, BusQueues& queues,
	    std::unordered_map<datatypes::RegisterEnum, bool>& registers)
	    : SearchListReader(getSlaveConfiguredAddresses(), slaveInformant.getBusInfo().ioMapUsedSize,
	          datatypes::now())
	    , slaveInformant(slaveInformant)
	    , busInfo(slaveInformant.getBusInfo())
	    , registerScheduler(slaveConfiguredAddresses, registers)
	    , queues(queues)
	    , desiredBusMode(datatypes::BusMode::READ_WRITE_OP)
	    , actualBusMode(busInfo.statusAfterInit == datatypes::BusStatus::OP
	              ? datatypes::BusMode::READ_WRITE_OP
	              : datatypes::BusMode::READ_WRITE_SAFE_OP)
	    , dataStorageThread(new std::thread(&BusReader::initDataStorageThread, this))
	    , realtimeThread(new std::thread(&BusReader::initRealtimeThread, this))
	{
	}

	BusReader::~BusReader()
	{
		ec_close();
		shouldHalt.store(true, std::memory_order_release);
		realtimeThread->join();
		dataStorageThread->join();
	}

	void BusReader::changeRegisterSettings(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		registerScheduler.changeRegisterSettings(toRead);
	}

	void BusReader::toggleBusSafeOp()
	{
		if (actualBusMode.load(std::memory_order_acquire) == datatypes::BusMode::READ_WRITE_OP)
		{
			desiredBusMode.store(datatypes::BusMode::READ_WRITE_SAFE_OP, std::memory_order_release);
		}
		else
		{
			desiredBusMode.store(datatypes::BusMode::READ_WRITE_OP, std::memory_order_release);
		}
	}

	datatypes::BusMode BusReader::getBusMode()
	{
		return actualBusMode.load(std::memory_order_acquire);
	}

	void BusReader::messageHalt() { shouldHalt.store(true, std::memory_order_release); }

	/*!
	 * \brief Initialize and start the data storage loop of this BusReader.
	 */
	void BusReader::initDataStorageThread() { dataStorageLoop(); }

	/*!
	 * \brief Initialize and start the realtime (bus-interacting) loop of this BusReader.
	 */
	void BusReader::initRealtimeThread()
	{
#ifdef ENABLE_RT
		static constexpr int realtimeThreadPriority = 49;
		if (!pinThreadToCPU(0))
		{
			queues.postError(
			    { "Failed to pin realtime thread to CPU.", datatypes::ErrorSeverity::MEDIUM });
		}
		if (!setThreadPriority(realtimeThreadPriority))
		{
			queues.postError({ "Failed to set realtime thread priority to "
			        + std::to_string(realtimeThreadPriority) + ".",
			    datatypes::ErrorSeverity::MEDIUM });
		}
#endif
		readerLoop();
	}

	/*!
	 * \brief The main realtime loop of this BusReader.
	 *
	 * It will target a frequency of 1 / desiredPDOTimeStep, and will adjust how
	 * many registers it reads per round dynamically.
	 * It communicates with the data storage loop via triple buffers.
	 * It will stop if messageHalt() has been signaled to the BusQueues.
	 */
	void BusReader::readerLoop()
	{
		using namespace std::chrono_literals;
		// This is from SOEM. No idea why the output working counter is doubled.
		const int expectedWKC = ec_group[0].outputsWKC * 2 + ec_group[0].inputsWKC;
		datatypes::TimeStamp lastLoopStart;
		int registersPerRound = 1;
		size_t currentIOMapBufferIndex = 0;
		size_t currentRegisterBufferIndex = 0;

		while (true)
		{
			lastLoopStart = datatypes::now();

			static const int timeoutus = 100;
			if (ec_send_processdata() <= 0)
			{
				queues.postError(
				    { "Failed to transmit process data frames.", datatypes::ErrorSeverity::LOW });
			}
			int actualWKC = ec_receive_processdata(timeoutus);

			// write IOMap in triple buffer
			if (actualWKC >= expectedWKC)
			{
				auto* buffer = ioMapBuffer.getProducerSlot(currentIOMapBufferIndex);
				std::memcpy(&buffer->value, busInfo.ioMap.data(), busInfo.ioMapUsedSize);
				buffer->time = datatypes::now();
				buffer->valid = true;
				if (currentIOMapBufferIndex == tripleBufferSize - 1)
				{
					ioMapBuffer.swapProducer();
					currentIOMapBufferIndex = 0;
				}
				else
				{
					++currentIOMapBufferIndex;
				}
			}
			else
			{
				queues.postError({ "Failed to receive process data frames."
				                   " Expected working counter "
				        + std::to_string(expectedWKC) + ", actual working counter "
				        + std::to_string(actualWKC),
				    datatypes::ErrorSeverity::LOW });
			}

			handleRequests();

			// Read registers
			for (EtherCATFrameIterator it = registerScheduler.getNextFrames(registersPerRound);
			     !it.atEnd(); ++it)
			{
				readRegisterFrame(
				    (*it).first, (*it).second, currentRegisterBufferIndex, it.hasCompletedLoop());
			}

			if (shouldHalt.load(std::memory_order_acquire))
			{
				desiredBusMode.store(datatypes::BusMode::READ_ONLY, std::memory_order_release);
				actualBusMode.store(datatypes::BusMode::READ_ONLY, std::memory_order_release);
				break;
			}

			handleBusMode();

			// Adjust loop length
			datatypes::TimeStamp currentLoopEnd(datatypes::now());
			auto loopDuration = currentLoopEnd - lastLoopStart;
			if (loopDuration > desiredPDOTimeStep && registersPerRound > 1)
			{
				--registersPerRound;
			}
			else if (loopDuration < registerFramePerRoundIncrementThreshold * desiredPDOTimeStep)
			{
				if (registersPerRound
				    < maxRegisterCyclesPerRound * registerScheduler.getFrameCount())
				{
					++registersPerRound;
				}
				else
				{
					while (datatypes::now() - lastLoopStart < desiredPDOTimeStep - 50us)
					{
						// Busy waiting because we don't want to give up the CPU
					}
				}
			}
		}
	}

	/*!
	 * \brief The loop responsible for storing read data in data structures.
	 *
	 * It communicates with the reader loop via triple buffers.
	 * It will stop if shouldHalt() has been messaged to the BusQueues.
	 */
	void BusReader::dataStorageLoop()
	{
		while (true)
		{
			ioMapBuffer.swapConsumer();

			// write IOMap in SearchList
			for (size_t index = 0; index < tripleBufferSize; ++index)
			{
				auto* buffer = ioMapBuffer.getConsumerSlot(index);
				if (!buffer->valid)
				{
					break;
				}
				std::unique_ptr<IOMap> ptr(
				    new (busInfo.ioMapUsedSize) IOMap{ busInfo.ioMapUsedSize, {} });
				std::memcpy(ptr->ioMap, buffer->value.data(), busInfo.ioMapUsedSize); // NOLINT
				insertIOMap(std::move(ptr), datatypes::TimeStamp(buffer->time));
				buffer->valid = false;
			}

			registerBuffer.swapConsumer();

			// write registers in lists
			for (size_t index = 0; index < tripleBufferSize; ++index)
			{
				auto* buffer = registerBuffer.getConsumerSlot(index);
				if (!buffer->valid)
				{
					break;
				}
				writeRegistersToLists(buffer->value, buffer->time);
				if (buffer->value.completedLoop)
				{
					insertNewRegisterTimeStamp(buffer->time);
				}
				buffer->valid = false;
			}

			freeMemoryIfNecessary();

			if (shouldHalt.load(std::memory_order_acquire))
			{
				break;
			}
		}
	}

	/*!
	 * \brief Send an EtherCATFrame of registers over the bus and place it in the triple buffer.
	 * \param frame the frame to send
	 * \param metaData the metadata to place in the triple buffer
	 * \param registerBufferIndex the index in the triple buffer to place the frame in
	 * \param saveTimeStamp whether to set the saving of the TimeStamp in the triple buffer
	 */
	void BusReader::readRegisterFrame(EtherCATFrame* frame, EtherCATFrameMetaData* metaData,
	    size_t& registerBufferIndex, bool saveTimeStamp)
	{
		auto [workingCounter, bufferIndex]
		    = sendAndReceiveEtherCATFrame(frame, metaData->lengthOfFrame, 0, {});
		if (workingCounter != EC_NOFRAME)
		{
			auto* tripleBuffer = registerBuffer.getProducerSlot(registerBufferIndex);
			// SOEM strips the Ethernet header for us in rxbuf, so we don't need
			// to account for it.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess" // NOLINT
			std::memcpy(&(tripleBuffer->value.frame),
			    &(ecx_context.port->rxbuf[bufferIndex]), // NOLINT
			    metaData->lengthOfFrame);
#pragma GCC diagnostic pop
			tripleBuffer->value.metaData = metaData;
			tripleBuffer->value.completedLoop = saveTimeStamp;
			tripleBuffer->time = datatypes::now();
			tripleBuffer->valid = true;

			if (registerBufferIndex == tripleBufferSize - 1)
			{
				registerBuffer.swapProducer();
				registerBufferIndex = 0;
			}
			else
			{
				++registerBufferIndex;
			}
		}
		else
		{
			queues.postError({ "Failed to send register frame.", datatypes::ErrorSeverity::LOW });
		}
		ecx_setbufstat(ecx_context.port, bufferIndex, EC_BUF_EMPTY);
	}

	/*!
	 * \brief Handle the requests submitted to the BusReader via the BusQueues.
	 */
	void BusReader::handleRequests()
	{
		auto coeRequest = queues.getCoERequest();
		if (coeRequest)
		{
			handleCoERequest(std::move(coeRequest));
		}

		auto pdoRequest = queues.getPDORequest();
		if (pdoRequest)
		{
			handlePDOWriteRequest(std::move(pdoRequest));
		}

		auto registerResetRequest = queues.getRegisterResetRequest();
		if (registerResetRequest.has_value())
		{
			handleRegisterResetRequest(registerResetRequest.value());
		}
	}

	void BusReader::handleCoERequest(std::shared_ptr<CoEUpdateRequest>&& request)
	{
		size_t bitLength = busInfo.coeInfos.at(*request->getObject()).bitLength;
		bool errorOccured = false;
		std::string action;
		if (request->isReadRequest())
		{
			errorOccured = !datatypes::dataTypeMapWithStrings<CoEReadRequestHandler>.at(
			    request->getObject()->getType())(
			    *request->getObject(), *request->getValue(), bitLength);
			action = "read";
		}
		else
		{
			errorOccured = !datatypes::dataTypeMapWithStrings<CoEWriteRequestHandler>.at(
			    request->getObject()->getType())(
			    *request->getObject(), *request->getValue(), bitLength);
			action = "write";
		}
		if (errorOccured)
		{
			std::stringstream sstream;
			sstream << "Failed to " << action << " CoE object at index "
			        << request->getObject()->getIndex() << " and subindex "
			        << request->getObject()->getSubIndex();
			queues.postError({ sstream.str(), request->getObject()->getSlaveID(),
			    datatypes::ErrorSeverity::MEDIUM });
			std::optional<datatypes::ErrorMessage> soemError;
			while ((soemError = convertECErrorToMessage()).has_value())
			{
				queues.postError(std::move(soemError.value()));
			}
			request->setFailed();
		}
		queues.postCoERequestReply(std::move(request));
	}

	void BusReader::handlePDOWriteRequest(std::shared_ptr<PDOWriteRequest>&& request)
	{
		static constexpr size_t byteSize = 8;
		datatypes::PDOInfo info = getAbsolutePDOInfo(request->getPDO());
		size_t byteOffset = info.bitOffset / byteSize;
		datatypes::dataTypeMap<bReader::AbstractDataPointWriter>.at(request->getPDO().getType())(
		    *request->getValue(), busInfo.ioMap.data() + byteOffset, info.bitOffset % byteSize,
		    info.bitLength);
	}

	void BusReader::handleRegisterResetRequest(unsigned int slave)
	{
		static constexpr size_t lengthOfPreMadeFrame = 44;
		static const std::vector<size_t> slaveAddressOffsets{ 2, 28 };

		// clang-format off
		static constexpr EtherCATFrame preMadeResetFrame{ (lengthOfPreMadeFrame - 2)
			    | 0x1000 /* Length of frame - 2 | PDU type */,
			{
			    0x05 /* FPWR */, 0xff /* Index */, 0xff, 0xff /* Slave address */,
				0x00, 0x03 /* Error counters' address */, 0x0e, 0x00 /* Error counters' length */,
				0x00, 0x00 /* External event */, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* Data area */, 0x00, 0x00 /* Working counter */,

				// There's a gap in the registers here that can't be written to,
				// so we have to split the frame into two PDUs.

			    0x05 /* FPWR */, 0xff /* Index */, 0xff, 0xff /* Slave address */,
				0x10, 0x03 /* Error counters' address */, 0x04, 0x00 /* Error counters' length */,
				0x00, 0x00 /* External event */, 0x00, 0x00, 0x00, 0x00 /* Data area */,
                0x00, 0x00 /* Working counter */
			} };
		// clang-format on

		auto [workingCounter, bufferIndex] = sendAndReceiveEtherCATFrame(&preMadeResetFrame,
		    lengthOfPreMadeFrame, slaveConfiguredAddresses[slave - 1], slaveAddressOffsets);

		if (workingCounter == EC_NOFRAME)
		{
			queues.postError({ "Failed to reset register error counters.", slave,
			    datatypes::ErrorSeverity::MEDIUM });
		}

		ecx_setbufstat(ecx_context.port, bufferIndex, EC_BUF_EMPTY);
	}

	void BusReader::handleBusMode()
	{
		static constexpr int stateCheckTimeoutus = 2000;
		datatypes::BusMode desired = desiredBusMode.load(std::memory_order_acquire);
		if (desired != actualBusMode.load(std::memory_order_acquire))
		{
			int soemState = 0;
			switch (desiredBusMode)
			{
			case datatypes::BusMode::READ_WRITE_OP:
				soemState = EC_STATE_OPERATIONAL;
				break;
			case datatypes::BusMode::READ_WRITE_SAFE_OP:
				soemState = EC_STATE_SAFE_OP;
				break;
			default:
				queues.postError({ "Unknown bus state required. No action will be taken",
				    datatypes::ErrorSeverity::LOW });
				break;
			}

			ec_slave[0].state = soemState;
			ec_writestate(0);

			if (ec_statecheck(0, soemState, stateCheckTimeoutus) != soemState)
			{
				if (busModeChangeAttemptNumber == maxBusModeChangeAttemptsBeforeError - 1)
				{
					queues.postError(
					    { "Failed to set slaves into state " + std::to_string(soemState) + ".",
					        datatypes::ErrorSeverity::MEDIUM });
					busModeChangeAttemptNumber = 0;
				}
				else
				{
					++busModeChangeAttemptNumber;
				}
			}
			else
			{
				actualBusMode.store(desiredBusMode, std::memory_order_release);
				busModeChangeAttemptNumber = 0;
			}
		}
	}

	/*!
	 * \brief Write the register data contained in the parameters to the SearchLists.
	 * \param frameWMetaData the frame to get the registers from, and the metadata to get them with
	 * \param time the TimeStamp to store in the lists.
	 */
	void BusReader::writeRegistersToLists(
	    EtherCATFrameWithMetaData frameWMetaData, datatypes::TimeStamp time)
	{
		uint8_t* frameData = reinterpret_cast<uint8_t*>(&frameWMetaData.frame); // NOLINT
		for (const PDUMetaData& meta : frameWMetaData.metaData->pdus)
		{
			uint16_t workingCounter = 0;
			std::memcpy(
			    &workingCounter, frameData + meta.workingCounterOffset, sizeof(uint16_t)); // NOLINT
			workingCounter = flipBytesIfBigEndianHost(workingCounter);
			if (workingCounter == 0)
			{
				continue;
			}
			for (const auto& reg : meta.registerOffsets)
			{
				uint8_t* registerPointer = frameData + reg.second; // NOLINT
				insertRegister(reg.first, registerPointer, meta.slaveConfiguredAddress, time);
			}
		}
	}

	datatypes::PDOInfo BusReader::getAbsolutePDOInfo(const datatypes::PDO& pdo)
	{
		static constexpr size_t byteSize = 8;
		size_t slaveStart = 0;
		size_t slaveStartBit = 0;
		// SOEM assigns input and output in exactly the opposite way to us
		switch (pdo.getDirection())
		{
		case datatypes::PDODirection::INPUT:
			slaveStart = ec_slave[pdo.getSlaveID()].outputs - busInfo.ioMap.data(); // NOLINT
			slaveStartBit = ec_slave[pdo.getSlaveID()].Ostartbit; // NOLINT
			break;
		case datatypes::PDODirection::OUTPUT:
			slaveStart = ec_slave[pdo.getSlaveID()].inputs - busInfo.ioMap.data(); // NOLINT
			slaveStartBit = ec_slave[pdo.getSlaveID()].Istartbit; // NOLINT
			break;
		}
		datatypes::PDOInfo info = busInfo.pdoOffsets.at(pdo);
		return { .bitOffset = info.bitOffset + slaveStart * byteSize + slaveStartBit,
			.bitLength = info.bitLength };
	}
} // namespace etherkitten::reader
