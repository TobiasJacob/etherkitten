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

#include "BusInfoSupplier.hpp"
#include "DataModelAdapter.hpp"
#include "ErrorView.hpp"
#include "MainWindow.hpp"
#include "PlotList.hpp"
#include "SlaveGraph.hpp"
#include "SlaveTree.hpp"
#include "TooltipFormatter.hpp"
#include "WatchList.hpp"
#include "util.hpp"
#include <QErrorMessage>
#include <QFileDialog>
#include <QObject>
#include <QProgressDialog>
#include <QTimer>
#include <chrono>
#include <etherkitten/config/BusLayout.hpp>
#include <etherkitten/datatypes/DataObjectVisitor.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

namespace etherkitten::gui
{

	/*!
	 * \brief Creates the user interface and controls the data exchange within.
	 */
	class GUIController : public QObject
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new GUIController.
		 *
		 * \param dataAdapter The DataModelAdapter used for data exchange with the backend.
		 * \param busInfo The BusInfoSupplier used for obtaining bus information from the backend.
		 * \param busLayout The initial BusLayout to determine the positions of slave in the
		 * SlaveGraph.
		 */
		GUIController(
		    DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo, config::BusLayout& busLayout);

		~GUIController();

		/*!
		 * \brief Set the default path used for the file dialogs.
		 *
		 * \param path The path to use.
		 */
		void setDefaultPath(std::string path);

		/*!
		 * \brief Set the shown registers.
		 *
		 * \param registers A map of the registers to booleans storing whether each register is
		 * shown.
		 */
		void setRegisters(std::unordered_map<datatypes::RegisterEnum, bool> registers);

		/*!
		 * \brief Set the layout shown in the bus graph.
		 *
		 * \param busLayout The new layout of the graph.
		 */
		void setBusLayout(config::BusLayout& busLayout);

		/*!
		 * \brief Clear all old data and setup the new data to be shown.
		 */
		void setupData();

		/*!
		 * \brief Clear all data shown.
		 */
		void clearData();

		/*!
		 * \brief Notify the GUIController that saving a profile was successful.
		 * This has to be called so the GUIController knows if the "Save"-Button
		 * should work.
		 */
		void saveWasSuccessful();

		/*!
		 * \brief Notify the GUIController whether the data are being logged or not.
		 * This is needed so the GUIController knows if the "Start logging"-Button
		 * should work.
		 * \param logging whether the data are being logged or not.
		 */
		void setLogging(bool logging);

		/*!
		 * \brief Show a non-blocking progress bar.
		 */
		void showNonBlockingProgressBar();

		/*!
		 * \brief Set the shown percentage on the non-blocking progress bar.
		 * Note that this only stores the percentage and text at first and updates
		 * the GUI in the next frame because Qt doesn't work well when being
		 * called from another thread. There is probably a better way to do this,
		 * but not when there is no time available to find that way.
		 * \param percent the percentage to show.
		 * \param text text to show with the progress bar.
		 */
		void setNonBlockingProgressBarPercentage(int percent, std::string text);

		/*!
		 * \brief Hide the non-blocking progress bar.
		 */
		void hideNonBlockingProgressBar();

		/*!
		 * \brief Set whether a log is currently being opened. This is used to
		 * set the text of the log opening button and determine what to do when
		 * it is clicked (either open a log or cancel the log opening).
		 * \param openingLog whether a log is currently being opened.
		 */
		void setOpeningLog(bool openingLog);

	public slots:
		/*!
		 * \brief Show an error message.
		 *
		 * \param msg The message to show.
		 */
		void showError(std::string msg);

		/*!
		 * \brief Show an error message and stop updating the shown data.
		 *
		 * \param msg The message to show.
		 */
		void showFatalError(std::string msg);

		/*!
		 * \brief Show a blocking progress dialog. If a dialog is already shown,
		 * its window title is set to the given title.
		 * \param title the title for the progress dialog.
		 */
		void showBlockingProgressBar(std::string title);

		/*!
		 * \brief Set the shown percentage on the blocking progress dialog.
		 * If no dialog is shown, this does nothing.
		 * \param percent the percentage to show.
		 * \param text text to show with the progress dialog.
		 */
		void setBlockingProgressBarPercentage(int percent, std::string text);

		/*!
		 * \brief Hide the blocking progress dialog. If no dialog is currently
		 * shown, this does nothing.
		 */
		void hideBlockingProgressBar();

	private slots:
		/*!
		 * \brief Update all shown data.
		 */
		void updateData();

		/*!
		 * \brief Show a dialog to choose the read registers.
		 */
		void showRegisterChooser();

		/*!
		 * \brief If a log is currently being read, cancel the remainder of the
		 * reading, otherwise show the log opening dialog.
		 */
		void openOrCancelLog();

		/*!
		 * \brief If no log is currently being written, show a dialog to allow the user
		 * to start logging, otherwise stop logging.
		 */
		void startOrStopLog();

		/*!
		 * \brief If no bus is connected, show a dialog to choose a network interface,
		 * otherwise emit a signal for the controller to detach the bus.
		 */
		void attachOrDetachBus();

		/*!
		 * \brief Emit a signal for the controller that the bus has to be toggled
		 * between Safe-OP and OP.
		 */
		void toggleSafeOp();

		/*!
		 * \brief Show the profile save dialog.
		 */
		void saveProfile();

		/*!
		 * \brief Show the profile save as dialog.
		 */
		void showProfileSaveAsDialog();

		/*!
		 * \brief Show the profile open dialog.
		 */
		void showProfileOpenDialog();

	signals:
		/*!
		 * \brief Request that the profile be saved at a given path.
		 *
		 * \param path The path to save the profile at.
		 */
		void requestSaveProfileAs(std::string path);

		/*!
		 * \brief Request that the profile at the given path be opened.
		 *
		 * \param path The path to open the profile from.
		 */
		void requestOpenProfile(std::string path);

		/*!
		 * \brief Request that the given interface be used for the bus connection.
		 *
		 * \param interface The network interface.
		 * \param startLogging Whether logging should be started immediately.
		 */
		void requestOpenInterface(std::string interface, bool startLogging);

		/*!
		 * \brief Request that the read registers be set.
		 *
		 * \param registers A map of the registers to booleans determining if they should be read.
		 */
		void requestSetRegisters(std::unordered_map<datatypes::RegisterEnum, bool> registers);

		/*!
		 * \brief Request that the logging be started.
		 *
		 * \param offset The number of seconds in the past that the log should begin at.
		 */
		void requestStartLogging(std::chrono::seconds offset);

		/*!
		 * \brief Request that the logging be stopped.
		 */
		void requestStopLogging();

		/*!
		 * \brief Request that a log be opened at the given path.
		 *
		 * \param path The path of the log.
		 */
		void requestOpenLog(std::string path);

		/*!
		 * \brief Request that the log opening is stopped.
		 */
		void requestStopOpeningLog();

		/*!
		 * \brief Request that the Safe-OP mode be toggled.
		 */
		void requestToggleSafeOp();

		/*!
		 * \brief Request that the bus be detached.
		 */
		void requestDetachBus();

	private:
		void genPlotMenu(QMenu* menu, const datatypes::DataObject& obj);
		class DataVisitor : public datatypes::DataObjectVisitor
		{
		public:
			DataVisitor();
			void handlePDO(const datatypes::PDO& pdo) override;
			void handleCoE(const datatypes::CoEObject& obj) override;
			void handleRegister(const datatypes::Register& reg) override;
			void handleErrorStatistic(const datatypes::ErrorStatistic& stat) override;
			bool canAddToWatchlist() const;
			bool canAddToPlots() const;

		private:
			bool watchlist;
			bool plots;
		};
		struct RegCmp
		{
			bool operator()(const std::pair<datatypes::RegisterEnum, std::string> a,
			    const std::pair<datatypes::RegisterEnum, std::string> b)
			{
				return compareRegisters(a.first, b.first);
			}
		};
		DataModelAdapter& adapter;
		BusInfoSupplier& busInfo;
		std::reference_wrapper<config::BusLayout> busLayout;
		QString defaultPath;
		SlaveGraph* slaveGraph;
		SlaveTree* slaveTree;
		WatchList* watchList;
		PlotList* plotList;
		ErrorView* errorView;
		QAction* safeOpAction;
		QAction* interfaceAction;
		QAction* logStartAction;
		QAction* logOpenAction;
		QTimer* timer;
		QProgressDialog* blockingProgress;
		QLabel* fpsLabel;
		QLabel* progressLabel;
		QProgressBar* nonBlockingProgress;
		std::string nonBlockingProgressLabel;
		int nonBlockingProgressPercent;
		std::mutex blockingProgressMutex;
		std::string lastProfile;
		datatypes::BusMode busMode;
		std::unordered_map<datatypes::RegisterEnum, bool> registers;
		std::vector<std::pair<datatypes::RegisterEnum, std::string>> registerNames;
		DataVisitor visitor;
		TooltipFormatter tooltipFormatter;
		MainWindow mainWindow;
		bool hasProfile;
		bool logging;
		bool openingLog;
		bool nonBlockingProgressDirty;
	};

} // namespace etherkitten::gui
