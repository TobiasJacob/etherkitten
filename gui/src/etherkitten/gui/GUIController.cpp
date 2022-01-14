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

#include "GUIController.hpp"
#include "InterfaceChooser.hpp"
#include "LogStartDialog.hpp"
#include "ProfileOpenDialog.hpp"
#include "ProfileSaveDialog.hpp"
#include "RegisterChooser.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	GUIController::DataVisitor::DataVisitor()
	    : watchlist(false)
	    , plots(false)
	{
	}

	void GUIController::DataVisitor::handlePDO(const datatypes::PDO& pdo)
	{
		(void)pdo;
		watchlist = true;
		plots = true;
	}

	void GUIController::DataVisitor::handleCoE(const datatypes::CoEObject& obj)
	{
		(void)obj;
		watchlist = true;
		plots = false;
	}

	void GUIController::DataVisitor::handleRegister(const datatypes::Register& reg)
	{
		(void)reg;
		watchlist = true;
		plots = true;
	}

	void GUIController::DataVisitor::handleErrorStatistic(const datatypes::ErrorStatistic& stat)
	{
		(void)stat;
		watchlist = true;
		plots = true;
	}

	bool GUIController::DataVisitor::canAddToPlots() const { return plots; }

	bool GUIController::DataVisitor::canAddToWatchlist() const { return watchlist; }

	GUIController::GUIController(
	    DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo, config::BusLayout& busLayout)
	    : adapter(dataAdapter)
	    , busInfo(busInfo)
	    , busLayout(busLayout)
	    , blockingProgress(nullptr)
	    , hasProfile(false)
	    , busMode(datatypes::BusMode::NOT_AVAILABLE)
	    , openingLog(false)
	{
		/* statusbar */
		fpsLabel = mainWindow.getFramerateLabel();
		progressLabel = mainWindow.getProgressLabel();
		nonBlockingProgress = mainWindow.getProgressBar();
		progressLabel->hide();
		nonBlockingProgress->hide();

		QWidget* plotArea = mainWindow.getPlotArea();
		QVBoxLayout* layout = new QVBoxLayout(plotArea);
		plotList = new PlotList(plotArea, busInfo, dataAdapter);
		layout->addWidget(plotList);
		QWidget* watchlistArea = mainWindow.getWatchlistArea();
		layout = new QVBoxLayout(watchlistArea);
		watchList = new WatchList(watchlistArea, dataAdapter, tooltipFormatter);
		layout->addWidget(watchList);
		QWidget* errorArea = mainWindow.getErrorArea();
		layout = new QVBoxLayout(errorArea);
		errorView = new ErrorView(errorArea, dataAdapter, busInfo, tooltipFormatter);
		layout->addWidget(errorView);
		QWidget* graphArea = mainWindow.getGraphArea();
		layout = new QVBoxLayout(graphArea);
		slaveGraph = new SlaveGraph(graphArea, dataAdapter, busInfo, busLayout);
		layout->addWidget(slaveGraph);
		QWidget* treeArea = mainWindow.getTreeArea();
		layout = new QVBoxLayout(treeArea);
		slaveTree = new SlaveTree(treeArea, dataAdapter, busInfo, tooltipFormatter);
		layout->addWidget(slaveTree);

		connect(slaveGraph, &SlaveGraph::slaveClicked, [this](unsigned int slave) {
			mainWindow.focusTreeArea();
			slaveTree->jumpToSlave(slave);
		});
		watchList->registerMenuGenerator([this](QMenu* menu, const datatypes::DataObject& obj) {
			obj.acceptVisitor(visitor);
			if (visitor.canAddToPlots())
				genPlotMenu(menu, obj);
		});
		auto generator = [this](QMenu* menu, const datatypes::DataObject& obj) {
			obj.acceptVisitor(visitor);
			if (visitor.canAddToPlots())
				genPlotMenu(menu, obj);
			if (visitor.canAddToWatchlist())
			{
				QAction* add = new QAction("Add to watchlist");
				connect(add, &QAction::triggered, [this, &obj]() { watchList->addData(obj); });
				menu->addAction(add);
			}
		};
		slaveTree->registerMenuGenerator(generator);
		errorView->registerMenuGenerator(generator);
		safeOpAction = mainWindow.getSafeOpAction();
		safeOpAction->setDisabled(true);
		interfaceAction = mainWindow.getInterfaceAction();
		logStartAction = mainWindow.getLogStartAction();
		logOpenAction = mainWindow.getLogOpenAction();
		connect(mainWindow.getRegisterAction(), &QAction::triggered, this,
		    &GUIController::showRegisterChooser);
		connect(logOpenAction, &QAction::triggered, this, &GUIController::openOrCancelLog);
		connect(safeOpAction, &QAction::triggered, this, &GUIController::toggleSafeOp);
		connect(interfaceAction, &QAction::triggered, this, &GUIController::attachOrDetachBus);
		connect(logStartAction, &QAction::triggered, this, &GUIController::startOrStopLog);
		connect(mainWindow.getProfileOpenAction(), &QAction::triggered, this,
		    &GUIController::showProfileOpenDialog);
		connect(mainWindow.getProfileSaveAction(), &QAction::triggered, this,
		    &GUIController::saveProfile);
		mainWindow.getProfileSaveAction()->setDisabled(true);
		connect(mainWindow.getProfileSaveAsAction(), &QAction::triggered, this,
		    &GUIController::showProfileSaveAsDialog);
		for (auto& reg : datatypes::registerMap)
		{
			registerNames.emplace_back(std::make_pair(reg.first, reg.second.name));
		}
		std::sort(registerNames.begin(), registerNames.end(), RegCmp());
		logStartAction->setDisabled(true);
		timer = new QTimer();
		connect(timer, &QTimer::timeout, this, &GUIController::updateData);
		timer->start(200);
		mainWindow.show();
	}

	void GUIController::genPlotMenu(QMenu* menu, const datatypes::DataObject& obj)
	{
		QMenu* plotMenu = new QMenu("Add to plot");
		for (int i = 0; i < plotList->getNumPlots(); i++)
		{
			QAction* act = new QAction(QStringLiteral("Plot ") + QString::number(i + 1));
			connect(act, &QAction::triggered, [this, i, &obj]() {
				plotList->addToPlot(i, obj);
				mainWindow.focusPlotArea();
			});
			plotMenu->addAction(act);
		}
		QAction* newPlot = new QAction("New plot");
		connect(newPlot, &QAction::triggered, [this, &obj]() {
			plotList->addPlot(obj);
			mainWindow.focusPlotArea();
		});
		plotMenu->addAction(newPlot);
		menu->addMenu(plotMenu);
	}

	void GUIController::showNonBlockingProgressBar()
	{
		nonBlockingProgress->setValue(0);
		nonBlockingProgress->show();
		progressLabel->setText("");
		progressLabel->show();
		nonBlockingProgressDirty = false;
	}

	void GUIController::setNonBlockingProgressBarPercentage(int percent, std::string text)
	{
		nonBlockingProgressPercent = percent;
		nonBlockingProgressLabel = text;
		nonBlockingProgressDirty = true;
	}

	void GUIController::hideNonBlockingProgressBar()
	{
		nonBlockingProgress->hide();
		progressLabel->hide();
	}

	void GUIController::showBlockingProgressBar(std::string title)
	{
		{
			std::scoped_lock lock(blockingProgressMutex);
			if (blockingProgress != nullptr)
			{
				blockingProgress->setWindowTitle(QString::fromStdString(title));
				return;
			}
		}
		mainWindow.setDisabled(true);
		blockingProgress = new QProgressDialog(&mainWindow);
		blockingProgress->setModal(true);
		blockingProgress->setCancelButton(nullptr);
		blockingProgress->setMinimum(0);
		blockingProgress->setMaximum(100);
		blockingProgress->setWindowTitle(QString::fromStdString(title));
		blockingProgress->setWindowFlags(
		    Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
		blockingProgress->show();
	}

	void GUIController::setBlockingProgressBarPercentage(int percent, std::string text)
	{
		std::scoped_lock lock(blockingProgressMutex);
		if (blockingProgress == nullptr)
			return;
		blockingProgress->setValue(percent);
		blockingProgress->setLabelText(QString::fromStdString(text));
	}

	void GUIController::hideBlockingProgressBar()
	{
		std::scoped_lock lock(blockingProgressMutex);
		if (blockingProgress == nullptr)
			return;
		blockingProgress->deleteLater();
		blockingProgress = nullptr;
		mainWindow.setDisabled(false);
	}

	void GUIController::setDefaultPath(std::string path)
	{
		defaultPath = QString::fromStdString(path);
	}

	void GUIController::setRegisters(std::unordered_map<datatypes::RegisterEnum, bool> newRegisters)
	{
		/* The registers set in the config are only the actual byte addresses, so every known
		 * register has to be checked here so that registers at the same byte address are all set to
		 * the same state */
		for (auto& reg : datatypes::registerMap)
		{
			int lower = static_cast<int>(reg.first) & 0xFFFF;
			if (newRegisters[static_cast<datatypes::RegisterEnum>(lower)])
				registers[reg.first] = true;
			else
				registers[reg.first] = false;
		}
		slaveTree->rebuildRegisters(registers);
	}

	void GUIController::setBusLayout(config::BusLayout& busLayout)
	{
		this->busLayout = busLayout;
		slaveGraph->setBusLayout(busLayout);
		slaveGraph->applyBusLayout();
	}

	void GUIController::setupData()
	{
		slaveGraph->setupData();
		errorView->setupData();
		slaveTree->setupData(this->registers);
		plotList->clear();
		watchList->clear();
		logging = false;
		logStartAction->setText("Start log");
		logStartAction->setDisabled(true);
		safeOpAction->setText("Change mode to SafeOp");
		safeOpAction->setDisabled(true);
		updateData();
	}

	void GUIController::clearData()
	{
		errorView->clear();
		watchList->clear();
		plotList->clear();
		slaveGraph->clear();
		slaveTree->clear();
	}

	void GUIController::showError(std::string msg)
	{
		QMessageBox::warning(&mainWindow, "Error", QString::fromStdString(msg));
	}

	void GUIController::showFatalError(std::string msg)
	{
		QMessageBox::critical(&mainWindow, "Fatal error", QString::fromStdString(msg));
	}

	void GUIController::updateData()
	{
		if (busInfo.getBusMode() != busMode)
		{
			busMode = busInfo.getBusMode();
			switch (busMode)
			{
			case datatypes::BusMode::READ_WRITE_OP:
				watchList->setSafeOP(false);
				slaveTree->setSafeOP(false);
				safeOpAction->setText("Change mode to SafeOp");
				break;
			case datatypes::BusMode::READ_WRITE_SAFE_OP:
				watchList->setSafeOP(true);
				slaveTree->setSafeOP(true);
				safeOpAction->setText("Change mode to Op");
				break;
			default:
				fpsLabel->setText(QStringLiteral("Not connected to bus"));
				watchList->setBusLive(false);
				slaveTree->setBusLive(false);
				errorView->setBusLive(false);
				interfaceAction->setText("Connect to interface");
				safeOpAction->setDisabled(true);
				logStartAction->setEnabled(false);
			}
			if (busMode == datatypes::BusMode::READ_WRITE_OP
			    || busMode == datatypes::BusMode::READ_WRITE_SAFE_OP)
			{
				watchList->setBusLive(true);
				slaveTree->setBusLive(true);
				errorView->setBusLive(true);
				interfaceAction->setText("Detach bus");
				safeOpAction->setDisabled(false);
				logStartAction->setEnabled(true);
			}
		}

		/* this is to make setNonBlockingProgressPercentage work better with threads */
		if (nonBlockingProgressDirty)
		{
			nonBlockingProgress->setValue(nonBlockingProgressPercent);
			progressLabel->setText(QString::fromStdString(nonBlockingProgressLabel));
			nonBlockingProgressDirty = false;
		}

		errorView->updateData();
		slaveTree->updateData();
		slaveGraph->updateData();
		watchList->updateData();
		plotList->updateData();
		if (busMode == datatypes::BusMode::READ_WRITE_OP
		    || busMode == datatypes::BusMode::READ_WRITE_SAFE_OP)
		{
			fpsLabel->setText(QStringLiteral("PDO framerate: ")
			    + QString::number(busInfo.getPDOFramerate())
			    + QStringLiteral(", Register framerate: ")
			    + QString::number(busInfo.getRegisterFramerate()));
		}
	}

	void GUIController::showRegisterChooser()
	{
		RegisterChooser chooser(&mainWindow, registers, registerNames);
		/* It doesn't matter that these registers sometimes have the high bits set to signal
		 * a bit offset because Application trims them to 16 bits anyways when giving them
		 * to the config. */
		if (chooser.exec() == QDialog::Accepted)
			emit requestSetRegisters(chooser.getRegisters());
	}

	void GUIController::setOpeningLog(bool openingLog)
	{
		this->openingLog = openingLog;
		if (openingLog)
		{
			logOpenAction->setText("Cancel opening log");
		}
		else
		{
			logOpenAction->setText("Open log");
		}
	}

	void GUIController::openOrCancelLog()
	{
		if (openingLog)
		{
			emit requestStopOpeningLog();
		}
		else
		{
			QString filename = QFileDialog::getOpenFileName(&mainWindow, "Open log", defaultPath);
			if (!filename.isEmpty())
			{
				defaultPath = QFileInfo(filename).path();
				emit requestOpenLog(filename.toStdString());
			}
		}
	}

	void GUIController::setLogging(bool logging)
	{
		this->logging = logging;
		if (logging)
			logStartAction->setText("Stop logging");
		else
			logStartAction->setText("Start logging");
	}

	void GUIController::startOrStopLog()
	{
		if (logging)
		{
			emit requestStopLogging();
		}
		else
		{
			LogStartDialog d(&mainWindow);
			if (d.exec() == QDialog::Accepted)
			{
				emit requestStartLogging(d.getOffset());
			}
		}
	}

	void GUIController::attachOrDetachBus()
	{
		if (busMode == datatypes::BusMode::READ_WRITE_OP
		    || busMode == datatypes::BusMode::READ_WRITE_SAFE_OP)
		{
			emit requestDetachBus();
		}
		else
		{
			InterfaceChooser chooser(&mainWindow, busInfo.getInterfaceNames());
			if (chooser.exec() == QDialog::Accepted)
			{
				emit requestOpenInterface(
				    chooser.getInterface(), chooser.getStartLoggingImmediately());
			}
		}
	}

	void GUIController::toggleSafeOp() { emit requestToggleSafeOp(); }

	void GUIController::saveWasSuccessful()
	{
		hasProfile = true;
		mainWindow.getProfileSaveAction()->setEnabled(true);
	}

	void GUIController::saveProfile()
	{
		slaveGraph->writeBusLayout();
		if (!hasProfile)
			showProfileSaveAsDialog();
		else
			emit requestSaveProfileAs(lastProfile);
	}

	void GUIController::showProfileSaveAsDialog()
	{
		slaveGraph->writeBusLayout();
		ProfileSaveDialog dialog(&mainWindow, busInfo.getProfileNames());
		if (dialog.exec() == QDialog::Accepted)
		{
			lastProfile = dialog.getProfile();
			emit requestSaveProfileAs(lastProfile);
		}
	}

	void GUIController::showProfileOpenDialog()
	{
		ProfileOpenDialog dialog(&mainWindow, busInfo.getProfileNames());
		if (dialog.exec() == QDialog::Accepted)
		{
			lastProfile = dialog.getProfile();
			emit requestOpenProfile(lastProfile);
		}
	}

	GUIController::~GUIController() { delete timer; }

} // namespace etherkitten::gui
