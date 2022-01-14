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
 * \brief Defines the BusReader, which reads data from an EtherCAT bus.
 */

#include <atomic>
#include <memory>
#include <thread>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>

#include "BusQueues.hpp"
#include "BusSlaveInformant.hpp"
#include "EtherCATFrame.hpp"
#include "IOMap.hpp"
#include "RegisterScheduler.hpp"
#include "RingBuffer.hpp"
#include "SearchList.hpp"
#include "SearchListReader.hpp"
#include "TripleBuffer.hpp"
#include "etherkitten/datatypes/time.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The BusReader class is responsible for reading data from and writing data to an
	 * EtherCAT bus.
	 *
	 * The BusReader uses its own threads to read data from the bus. It will run until its
	 * BusQueues have been sent the `messageHalt()` signal, after which it will stop the reading
	 * process. From that point on, the data read by the BusReader will still be available,
	 * but no new data will be read.
	 *
	 * The BusReader will accept reading and writing requests for CoEObjects, PDOs, and registers
	 * via its BusQueues and will handle them on a best-effort basis. See the BusQueues
	 * documentation for details.
	 *
	 * The BusReader will log any errors it encounters via the BusQueues.
	 * The BusReader expects to have exclusive access to the EtherCAT bus. Sending frames across
	 * the bus while the BusReader is running (after instantiation and before `messageHalt()`)
	 * will result in undefined behavior.
	 *
	 * The BusReader reads a user-specified set of registers from the EtherCAT bus while ignoring
	 * the others. The user can change which registers are read while the BusReader is running.
	 * Reading fewer registers will be faster and lead to slower accumulation of memory usage.
	 */
	class BusReader : public SearchListReader
	{
	public:
		/*!
		 * \brief Construct a new BusReader that interacts with the EtherCAT bus the
		 * BusSlaveInformant was initialized with.
		 *
		 * The realtime loop will be started immediately. Initially, the given registers will be
		 * read from the bus.
		 *
		 * The BusReader will use the BusQueues-specific end of the queues to communicate with
		 * the user. After this method has been called, those methods may no longer be used.
		 *
		 * This constructor is non-blocking.
		 * \param slaveInformant the BusSlaveInformant to get slave and bus information from
		 * \param queues the BusQueues to communicate over
		 * \param registers the registers to read from the start
		 */
		BusReader(BusSlaveInformant& slaveInformant, BusQueues& queues,
		    std::unordered_map<datatypes::RegisterEnum, bool>& registers);

		// These constructors are deleted because implementing them properly would be
		// a lot of work (and would be kind of pointless).
		BusReader(const BusReader&) = delete;

		BusReader(BusReader&&) = delete;

		BusReader& operator=(const BusReader&) = delete;

		BusReader& operator=(BusReader&&) = delete;

		~BusReader() override;

		datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo) override;

		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead) override;

		void toggleBusSafeOp() override;

		datatypes::BusMode getBusMode() override;

		void messageHalt() override;

	private:
		BusSlaveInformant& slaveInformant;
		BusInfo& busInfo;

		RegisterScheduler registerScheduler;
		BusQueues& queues;

		struct EtherCATFrameWithMetaData
		{
			EtherCATFrameMetaData* metaData = nullptr;
			bool completedLoop = false;
			EtherCATFrame frame;
		};

		static constexpr size_t tripleBufferSize = 20;
		TripleBuffer<std::array<uint8_t, ioMapSize>, tripleBufferSize> ioMapBuffer;
		TripleBuffer<EtherCATFrameWithMetaData, tripleBufferSize> registerBuffer;

		std::atomic<datatypes::BusMode> desiredBusMode;
		std::atomic<datatypes::BusMode> actualBusMode;

		static constexpr int maxBusModeChangeAttemptsBeforeError = 2;
		unsigned int busModeChangeAttemptNumber = 0;

		std::unique_ptr<std::thread> dataStorageThread;
		std::unique_ptr<std::thread> realtimeThread;

		std::atomic<bool> shouldHalt = false;

		static constexpr datatypes::TimeStep desiredPDOTimeStep = 1ms;

		// If the time for a round is smaller than this * desiredPDOTimeStep,
		// read more registers unless you are already reading the maximum number.
		static constexpr double registerFramePerRoundIncrementThreshold = 0.9;
		static constexpr uint64_t maxRegisterCyclesPerRound = 2;

		void initRealtimeThread();
		void initDataStorageThread();
		void readerLoop();
		void dataStorageLoop();

		void readRegisterFrame(EtherCATFrame* frame, EtherCATFrameMetaData* metaData,
		    size_t& registerBufferIndex, bool saveTimeStamp);

		void writeRegistersToLists(
		    EtherCATFrameWithMetaData frameWMetaData, datatypes::TimeStamp time);

		void handleRequests();
		void handleCoERequest(std::shared_ptr<CoEUpdateRequest>&& request);
		void handlePDOWriteRequest(std::shared_ptr<PDOWriteRequest>&& request);
		void handleRegisterResetRequest(unsigned int slave);

		void handleBusMode();
	};
} // namespace etherkitten::reader
