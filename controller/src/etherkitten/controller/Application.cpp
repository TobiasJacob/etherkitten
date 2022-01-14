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

#include "Application.hpp"
#include "etherkitten/datatypes/errors.hpp"
#include "etherkitten/gui/GUIController.hpp"
#include <QTranslator>
#include <chrono>
#include <ctime>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>

const std::string rawSocketOptionName = "raw-socket";
std::optional<int> popRawSocketArgs(int& argc, char**& argv);

using namespace etherkitten::controller;

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(i18n);

	QApplication qApplication(argc, argv);
	QApplication::setApplicationName("EtherKITten");
	QApplication::setApplicationVersion(QString("0.? built with Qt ") + QT_VERSION_STR);

	QTranslator t;
	if (!t.load(":/i18n/etherkitten_en_secret.qm"))
		t.load(":i18n/etherkitten_en.qm");
	QApplication::installTranslator(&t);

	// Handle command line options
	QCommandLineParser parser;
	parser.setApplicationDescription("Developer and Visualization Tools for Robot Bus Systems");

	QCommandLineOption helpOption = parser.addHelpOption();
	QCommandLineOption versionOption = parser.addVersionOption();
	QCommandLineOption rawSocketOption(QString::fromStdString(rawSocketOptionName),
	    "Support raw_socket. Warning: only specify this option if raw_socket is actually used!");
	parser.addOption(rawSocketOption);
#ifdef ENABLE_MOCKS
	QCommandLineOption busMockOption("bus-mock", "Run with a mocked bus.");
	parser.addOption(busMockOption);
#endif

	parser.process(qApplication);

	InterfaceInfo interfaceInfo;
	if (parser.isSet(rawSocketOption))
	{
#ifdef ENABLE_MOCKS
		if (parser.isSet(busMockOption))
		{
			std::cout << "Error: incompatible options specified!" << std::endl;
			return -1;
		}
#endif
		char* end;
		errno = 0;
		long socket = std::strtol(argv[1], &end, 10);
		if (errno != 0 || *end != '\0' || socket < std::numeric_limits<int>::min()
		    || socket > std::numeric_limits<int>::max())
		{
			std::cout << "Error: raw_socket arguments not parseable!" << std::endl
			          << "This probably means that EtherKITten has not been called by raw_socket!"
			          << std::endl;
			return -1;
		}
		interfaceInfo = InterfaceInfo{ static_cast<int>(socket) };
	}
#ifdef ENABLE_MOCKS
	else if (parser.isSet(busMockOption))
	{
		interfaceInfo = InterfaceInfo{ true };
	}
#endif

	Application application{ interfaceInfo };
	Application::connect(
	    &qApplication, &QApplication::aboutToQuit, &application, &Application::destroyBackend);

	QApplication::exec();

	Q_CLEANUP_RESOURCE(i18n);
}

namespace etherkitten::controller
{

	Application::Application(InterfaceInfo interfaceInfo)
	    : standardConfigPath(config::ConfigIO::getHomeDir() + "/.config/etherkitten")
	    , maximumMemory(0) // unlimited
	    , interfaceInfo(interfaceInfo)
	    , config(standardConfigPath)
	    , etherKitten(new reader::EtherKitten())
	    , dataModelAdapter(*etherKitten)
	    , busInfoSupplier(*etherKitten, config, interfaceInfo)
	    , gui(dataModelAdapter, busInfoSupplier, busLayout)
	{

		// clang-format off
 		std::cout << "================================= EtherKITten ==================================" << std::endl;
 		std::cout << "            Developer and Visualization Tools for Robot Bus Systems             " << std::endl;
 		std::cout << "================================================================================" << std::endl;
		// clang-format on

		connectToGUIController();
		config.registerObserver(*this);
		config.readLogFolderPath();
		config.readMaximumMemory();
		config.readDefaultConfig();
		gui.setDefaultPath(logFolderPath);
		std::unordered_map<etherkitten::datatypes::RegisterEnum, bool> regMap;
		for (uint32_t reg : busConfig.getVisibleRegisters())
			regMap[static_cast<etherkitten::datatypes::RegisterEnum>(reg)] = true;
		gui.setRegisters(regMap);
		gui.setBusLayout(busLayout);
	}

	Application::~Application()
	{
		futureSignalThread.quit();
		futureSignalThread.wait();
	}

	void Application::connectToGUIController()
	{
		connect(&gui, &gui::GUIController::requestSaveProfileAs, this, &Application::saveProfile);
		connect(&gui, &gui::GUIController::requestOpenProfile, this, &Application::openProfile);
		connect(&gui, &gui::GUIController::requestOpenInterface, this, &Application::openInterface);
		connect(&gui, &gui::GUIController::requestDetachBus, this, &Application::detachBus);
		connect(&gui, &gui::GUIController::requestSetRegisters, this, &Application::setRegisters);
		connect(&gui, &gui::GUIController::requestStartLogging, this, &Application::startLogging);
		connect(&gui, &gui::GUIController::requestStopLogging, this, &Application::stopLogging);
		connect(&gui, &gui::GUIController::requestOpenLog, this, &Application::openLog);
		connect(&gui, &gui::GUIController::requestToggleSafeOp, this, &Application::toggleSafeOp);

		connect(
		    &gui, &gui::GUIController::requestStopOpeningLog, this, &Application::stopOpeningLog);

		/*
		 * This is a signal-slot connection for threading reasons.
		 * Qt::BlockingQueuedConnection is used as connection type to ensure that Qt cannot
		 * call setBlockingProgressBarPercentage recursively, when the signal is more than once
		 * in the dispatcher queue.
		 */
		connect(this, &Application::changeBlockingProgressBar, &gui,
		    &gui::GUIController::setBlockingProgressBarPercentage, Qt::BlockingQueuedConnection);
	}

	void Application::onBusLayoutChanged(config::BusLayout busLayout, std::string busId)
	{
		if (this->busId.has_value() && this->busId == busId)
		{
			this->busLayout = busLayout;
		}
	}

	void Application::onBusConfigChanged(
	    config::BusConfig busConfig, std::optional<std::string> busId)
	{
		// If busId is nullopt, the given busConfig is the default.
		// Only adopt the default if no busId is set.
		if ((!busId.has_value() && !this->busId.has_value())
		    || (busId.has_value() && this->busId == busId))
		{
			this->busConfig = busConfig;
		}
	}

	void Application::onLogPathChanged(std::filesystem::path newLogFolderPath)
	{
		this->logFolderPath = newLogFolderPath;
	}

	void Application::onMaximumMemoryChanged(size_t newMaximumMemory)
	{
		etherKitten->setMaximumMemory(newMaximumMemory);
	}

	void Application::saveProfile(std::string busId)
	{
		std::optional<std::string> oldBusId = this->busId;
		try
		{
			this->busId = busId;

			config.writeBusLayout(busLayout, busId);
			config.writeBusConfig(busConfig, busId);
			gui.saveWasSuccessful();
		}
		catch (const std::exception& e)
		{
			this->busId = oldBusId;
			gui.showError("An error occurred while saving profile:\n" + std::string(e.what()));
		}
	}

	void Application::openProfile(std::string busId)
	{
		std::optional<std::string> oldBusId = this->busId;
		try
		{
			this->busId = busId;

			config.readBusConfig(busId);
			config.readBusLayout(busId);

			std::unordered_map<datatypes::RegisterEnum, bool> guiRegs;
			std::vector<uint32_t> configRegs = busConfig.getVisibleRegisters();
			for (uint32_t configReg : configRegs)
				guiRegs[static_cast<datatypes::RegisterEnum>(configReg)] = true;
			gui.setRegisters(guiRegs);
			gui.setBusLayout(busLayout);
			gui.saveWasSuccessful();
			auto mode = etherKitten->getBusMode();
			if (mode == datatypes::BusMode::READ_WRITE_OP
			    || mode == datatypes::BusMode::READ_WRITE_SAFE_OP)
				etherKitten->changeRegisterSettings(guiRegs);
		}
		catch (const std::exception& e)
		{
			this->busId = oldBusId;
			gui.showError("An error occurred while loading profile:\n" + std::string(e.what()));
		}
	}

	void Application::openInterface(std::string interface, bool startLogging)
	{
		gui.clearData();
		// Reset the bus ID when a bus is connected
		busId = std::nullopt;
		busConnectionTime = datatypes::now();
		std::unordered_map<etherkitten::datatypes::RegisterEnum, bool> regMap;
		for (uint32_t reg : busConfig.getVisibleRegisters())
		{
			regMap[static_cast<etherkitten::datatypes::RegisterEnum>(reg)] = true;
		}

		std::shared_future<void> busInitFuture;
		qRegisterMetaType<std::string>();
		qRegisterMetaType<std::shared_future<void>>();

		switch (interfaceInfo.getType())
		{
		case InterfaceType::GENERIC:
		{
			gui.showBlockingProgressBar("Initializing bus...");
			busInitFuture = std::async([interface, regMap, this]() mutable {
				etherKitten->connectBus(interface, regMap, blockingProgressFunc);
			});
			break;
		}
		case InterfaceType::RAW_SOCKET:
		{
			gui.showBlockingProgressBar("Initializing bus...");
			busInitFuture = std::async([regMap, this]() mutable {
				etherKitten->connectBus(
				    this->interfaceInfo.getSocket(), regMap, blockingProgressFunc);
			});
			break;
		}
		default:
#ifdef ENABLE_MOCKS
			etherKitten->startBusMock(regMap);
#else
			gui.showError(
			    "An error occurred while attaching the bus:\nInvalid interface type specified.");
#endif
			return;
		}

		// Create notification that activates when initialization is done
		FutureSignal* futureSignal = new FutureSignal;
		futureSignal->moveToThread(&futureSignalThread);
		connect(&futureSignalThread, &QThread::finished, futureSignal, &QObject::deleteLater);
		connect(this, &Application::createFutureSignal, futureSignal, &FutureSignal::waitFor);
		connect(futureSignal, &FutureSignal::futureReady, this, &Application::finishOpenInterface);
		futureSignalThread.start();

		emit createFutureSignal(std::move(busInitFuture), startLogging);
	}

	void Application::finishOpenInterface(
	    std::shared_future<void> result, bool startLogging, FutureSignal* sender)
	{
		// Do not get called again by the same sender
		disconnect(sender, &FutureSignal::futureReady, this, &Application::finishOpenInterface);
		try
		{
			result.get();
		}
		catch (const reader::SlaveInformantError& e)
		{
			std::ostringstream combinedErrorMessage;
			combinedErrorMessage << "An error occurred while attaching the bus:\n";
			combinedErrorMessage << std::string(e.what()) << std::endl;
			for (const datatypes::ErrorMessage& msg : e.getErrorList())
			{
				combinedErrorMessage << msg << std::endl;
			}

			gui.hideBlockingProgressBar();
			gui.showError(combinedErrorMessage.str());
			return;
		}
		catch (const std::exception& e)
		{
			gui.hideBlockingProgressBar();
			gui.showError("An error occurred while attaching the bus:\n" + std::string(e.what()));
			return;
		}
		gui.hideBlockingProgressBar();
		if (startLogging)
			this->startLogging(std::chrono::seconds::zero());
		gui.setupData();
		if (startLogging)
			gui.setLogging(true);
	}

	void Application::detachBus()
	{
		try
		{
			etherKitten->detachBus();
		}
		catch (const std::exception& e)
		{
			gui.showError("An error occurred while detaching the bus:\n" + std::string(e.what()));
		}
	}

	void Application::setRegisters(std::unordered_map<datatypes::RegisterEnum, bool> registers)
	{
		try
		{
			// Reset all registers
			for (const auto& setRegister : busConfig.getVisibleRegisters())
			{
				busConfig.setRegisterVisible(setRegister, false);
			}
			// Override them with the settings of the GUI
			for (const auto& mapEntry : registers)
				busConfig.setRegisterVisible(
				    static_cast<uint16_t>(mapEntry.first), mapEntry.second);
			gui.setRegisters(registers);
			auto mode = etherKitten->getBusMode();
			if (mode == datatypes::BusMode::READ_WRITE_OP
			    || mode == datatypes::BusMode::READ_WRITE_SAFE_OP)
				etherKitten->changeRegisterSettings(registers);
		}
		catch (const std::exception& e)
		{
			gui.showError(
			    "An error occurred while saving the register settings:\n" + std::string(e.what()));
		}
	}

	void Application::startLogging(std::chrono::seconds offset)
	{
		std::string prefix;
		if (busId.has_value())
			prefix = busId.value();
		else
			prefix = "unnamed";
		std::ostringstream fileName;
		datatypes::TimeStamp time;
		if (datatypes::now().time_since_epoch() < offset)
			time = busConnectionTime;
		else
			time = datatypes::now() - offset;

		// This is system time, since TimeStamp is no absolute time point
		// but relative to e.g. system boot
		time_t fileNameTime
		    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - offset);
		std::string formattedTime(30, '\0');
		std::strftime(&formattedTime[0], formattedTime.size(), "%Y-%m-%d_%H:%M:%S",
		    std::localtime(&fileNameTime));
		fileName << prefix << "_" << formattedTime << ".log";

		try
		{
			std::string prefix;
			if (busId.has_value())
				prefix = busId.value();
			else
				prefix = "unnamed";
			std::ostringstream fileName;
			datatypes::TimeStamp time;
			// If the offset would let the log begin before the start of the epoch,
			// things would go south.
			// Also, if the offset would let the log begin before the bus connection time,
			// floor it to that.
			if (offset > datatypes::now().time_since_epoch()
			    || busConnectionTime > datatypes::now() - offset)
				time = busConnectionTime;
			else
				time = datatypes::now() - offset;

			// This is system time, since TimeStamp is no absolute time point
			// but relative to e.g. system boot
			time_t fileNameTime
			    = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - offset);
			std::string formattedTime(30, '\0');
			std::strftime(&formattedTime[0], formattedTime.size(), "%Y-%m-%d_%H:%M:%S",
			    std::localtime(&fileNameTime));
			fileName << prefix << "_" << formattedTime << ".log";
			std::filesystem::create_directories(logFolderPath);
			gui.showNonBlockingProgressBar();
			etherKitten->startLogging(logFolderPath / fileName.str(), time,
			    [this](int percentage, std::string statusMessage) {
				    this->gui.setNonBlockingProgressBarPercentage(percentage, statusMessage);
				    if (percentage >= 100)
				    {
					    this->gui.hideNonBlockingProgressBar();
				    }
			    });
			gui.setLogging(true);
		}
		catch (const std::exception& e)
		{
			gui.hideNonBlockingProgressBar();
			gui.showError("An error occurred while starting the log:\n" + std::string(e.what()));
			gui.setLogging(false);
		}
	}

	void Application::stopLogging()
	{
		etherKitten->stopLogging();
		gui.setLogging(false);
	}

	void Application::openLog(std::string path)
	{
		// Reset the bus ID when a log is being loaded
		busId = std::nullopt;
		gui.clearData();
		this->gui.setOpeningLog(true);
		gui.showBlockingProgressBar("Loading log...");
		qRegisterMetaType<std::string>();
		std::shared_future<void> logInitFuture = std::async([path, this]() {
			etherKitten->loadLog(path, this->blockingProgressFunc,
			    [this](int percentage, std::string statusMessage) {
				    this->gui.setNonBlockingProgressBarPercentage(percentage, statusMessage);
				    if (percentage >= 100)
				    {
					    this->gui.hideNonBlockingProgressBar();
					    this->gui.setOpeningLog(false);
				    }
			    });
		});

		// Create notification that activates when initialization is done
		FutureSignal* futureSignal = new FutureSignal;
		futureSignal->moveToThread(&futureSignalThread);
		connect(&futureSignalThread, &QThread::finished, futureSignal, &QObject::deleteLater);
		qRegisterMetaType<std::shared_future<void>>();
		connect(this, &Application::createFutureSignal, futureSignal, &FutureSignal::waitFor);
		connect(futureSignal, &FutureSignal::futureReady, this, &Application::finishOpenLog);
		futureSignalThread.start();

		emit createFutureSignal(std::move(logInitFuture), false /*ignored*/);
	}

	void Application::stopOpeningLog()
	{
		this->gui.setOpeningLog(false);
		etherKitten->stopReadingLog();
	}

	void Application::finishOpenLog(
	    std::shared_future<void> result, bool ignored, FutureSignal* sender)
	{
		// Do not get called again by the same sender
		disconnect(sender, &FutureSignal::futureReady, this, &Application::finishOpenLog);
		try
		{
			result.get();
		}
		catch (const reader::SlaveInformantError& e)
		{
			this->gui.setOpeningLog(false);
			std::ostringstream combinedErrorMessage;
			combinedErrorMessage << "An error occurred while attaching the bus:\n";
			combinedErrorMessage << std::string(e.what()) << std::endl;
			for (const datatypes::ErrorMessage& msg : e.getErrorList())
			{
				combinedErrorMessage << msg << std::endl;
			}

			gui.hideBlockingProgressBar();
			gui.showError(combinedErrorMessage.str());
			return;
		}
		catch (const std::exception& e)
		{
			this->gui.setOpeningLog(false);
			gui.hideBlockingProgressBar();
			gui.showError("An error occurred while opening a log:\n" + std::string(e.what()));
			return;
		}
		gui.hideBlockingProgressBar();
		gui.showNonBlockingProgressBar();
		gui.setupData();
	}

	void Application::toggleSafeOp()
	{
		try
		{
			etherKitten->toggleBusSafeOp();
		}
		catch (const std::exception& e)
		{
			gui.showError(
			    "An error occurred while changing the bus status:\n" + std::string(e.what()));
		}
	}

	void Application::destroyBackend() { etherKitten.reset(); }
} // namespace etherkitten::controller
