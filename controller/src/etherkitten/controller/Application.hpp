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

#include "EtherKittenBusInfoSupplier.hpp"
#include "EtherKittenDataModelAdapter.hpp"
#include "InterfaceInfo.hpp"
#include <QtCore/QObject>
#include <etherkitten/config/BusConfig.hpp>
#include <etherkitten/config/BusLayout.hpp>
#include <etherkitten/config/ConfigIO.hpp>
#include <etherkitten/config/ConfigObserver.hpp>
#include <etherkitten/gui/GUIController.hpp>
#include <etherkitten/reader/EtherKitten.hpp>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <sys/types.h>

Q_DECLARE_METATYPE(std::shared_future<void>) // NOLINT
Q_DECLARE_METATYPE(std::string) // NOLINT

namespace etherkitten::controller
{
	/*!
	 * \brief Takes a std::future and emits a signal if its result is available.
	 */
	class FutureSignal : public QObject
	{
		Q_OBJECT

	public slots:
		/**
		 * \brief Wait untile the future's result is available and then emit futureReady().
		 * \param[in]  future the future to wait for
		 * \param[in]  arg a forwarded argument
		 */
		void waitFor(std::shared_future<void> future, bool arg)
		{
			future.wait();

			emit futureReady(std::move(future), arg, this);
		}

	signals:
		/**
		 * \brief Notify the connected slot that the future is ready.
		 * \param[in]  future the future that is ready
		 * \param[in]  arg an argument that is forwarded to the slots
		 * \param[in]  sender the FutureSignal object that emitted this signal
		 */
		void futureReady(std::shared_future<void> future, bool arg, FutureSignal* sender);
	};

	/*!
	 * \brief The top-most Class in EtherKITten. Creates and owns front- and backend
	 * as well as the config.
	 */
	class Application : public QObject, public config::ConfigObserver
	{
		Q_OBJECT

	public:
		/**
		 * \brief Constructs the application with the given interface for bus access.
		 * \param[in]  interfaceInfo information on how to access the bus
		 */
		Application(InterfaceInfo interfaceInfo);
		~Application();

		//! \copydoc config::ConfigObserver::onBusLayoutChanged()
		void onBusLayoutChanged(config::BusLayout busLayout, std::string busId) override;
		//! \copydoc config::ConfigObserver::onBusConfigChanged()
		void onBusConfigChanged(
		    config::BusConfig busConfig, std::optional<std::string> busId) override;
		//! \copydoc config::ConfigObserver::onLogPathChanged()
		void onLogPathChanged(std::filesystem::path newLogFolderPath) override;
		//! \copydoc config::ConfigObserver::onMaximumMemoryChanged()
		void onMaximumMemoryChanged(size_t newMaximumMemory) override;

		// public slots:
		/**
		 * \brief Destroy the held EtherKitten instance.
		 */
		void destroyBackend();

	private slots:
		/*!
		 * \brief Save the currently held BusLayout and BusConfig to the specified \p busId. Also
		 * set \p busId as the current busId. It will then be used to identify the current bus, e.g.
		 * for log file names.
		 * \param busId the name of the configuration file to which the
		 * BusLayout and BusConfig will be saved
		 */
		void saveProfile(std::string busId);
		/*!
		 * \brief Replace the currently held BusLayout and BusConfig with the ones associated with
		 * the specified \p busId. Also set \p busId as the current busId. It will then be used to
		 * identify the current bus, e.g. for log file names.
		 * \param busId the name of the
		 * configuration file from which the BusLayout and BusConfig will be loaded
		 */
		void openProfile(std::string path);
		/*!
		 * \brief Start EtherKitten using the specified network interface.
		 * \param interface the network interface to start EtherKitten with
		 * \param startLogging whether to start logging immediately
		 */
		void openInterface(std::string interface, bool startLogging);
		/*!
		 * \brief Tell EtherKitten to shut down any activity with its currently
		 * attached bus.
		 */
		void detachBus();
		/*!
		 * \brief Define which Registers are read and forwarded.
		 * \param registers a map of the Register addresses to whether they are read
		 */
		void setRegisters(std::unordered_map<datatypes::RegisterEnum, bool> registers);
		/*!
		 * \brief Start logging with the specified offset.
		 * \param offset the offset
		 */
		void startLogging(std::chrono::seconds offset);
		/*!
		 * \brief Stop logging.
		 */
		void stopLogging();
		/*!
		 * \brief Open a log and emulate the last recorded state.
		 * \param path the path of the log file
		 */
		void openLog(std::string path);
		/*!
		 * \brief Stop opening log
		 */
		void stopOpeningLog();
		/*!
		 * \brief Toggle the bus state from Safeop to OP and vice versa.
		 */
		void toggleSafeOp();
		/*!
		 * \brief React to the end of the initialization of an opened interface.
		 * \details For internal use only.
		 * \param result any exception that occurred during initialization
		 * \param startLogging whether to start logging
		 * \param sender the FutureSignal object that emitted this signal
		 */
		void finishOpenInterface(
		    std::shared_future<void> result, bool startLogging, FutureSignal* sender);
		/*!
		 * \brief React to the end of the initialization of an opened log.
		 * \details For internal use only.
		 * \param result any exception that occurred during initialization
		 * \param arg ignored
		 * \param sender the FutureSignal object that emitted this signal
		 */
		void finishOpenLog(std::shared_future<void> result, bool arg, FutureSignal* sender);

	signals:
		/**
		 * \brief Create a FutureSignal for the given future
		 * \param[in]  future the future to create a FutureSignal for
		 * \param[in]  args additional argument that will be forwarded to the slot
		 */
		void createFutureSignal(std::shared_future<void> future, bool arg);

		/**
		 * \brief Change the contents of the blocking progress bar.
		 * \param[in]  percentage the percentage to show
		 * \param[in]  statusMessage an additional message that is displayed next to the progress
		 * bar
		 */
		void changeBlockingProgressBar(int percentage, std::string statusMessage);

	private:
		const std::filesystem::path standardConfigPath;
		std::filesystem::path logFolderPath;
		size_t maximumMemory;
		InterfaceInfo interfaceInfo;
		config::ConfigIO config;
		config::BusConfig busConfig;
		config::BusLayout busLayout;
		std::optional<std::string> busId;
		std::unique_ptr<reader::EtherKitten> etherKitten;
		EtherKittenDataModelAdapter dataModelAdapter;
		EtherKittenBusInfoSupplier busInfoSupplier;
		gui::GUIController gui;
		datatypes::TimeStamp busConnectionTime = datatypes::now();

		std::function<void(int, std::string)> blockingProgressFunc
		    = [this](int percentage, std::string statusMessage) {
			      emit changeBlockingProgressBar(percentage, statusMessage);
		      };

		QThread futureSignalThread;

		void connectToGUIController();
	};
} // namespace etherkitten::controller
