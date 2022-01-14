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

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QWidget>

/* Forward-declare this so mainwindow.hpp can be included by the
 * GUIController without having to include ui_mainwindow.h (which
 * includes the whole generated implementation) */
namespace Ui
{
	class MainWindow;
}

namespace etherkitten::gui
{

	/*!
	 * \brief Acts as an interface between the class Qt generates from
	 * mainwindow.ui and the GUIController.
	 */
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new MainWindow and set up all the widgets.
		 * \param parent The parent widget (generally just nullptr).
		 */
		MainWindow(QWidget* parent = nullptr);

		/*!
		 * \brief Destroy the MainWindow.
		 */
		~MainWindow();

		/*!
		 * \brief Return the QAction which is used to choose a network
		 * interface or detach the bus.
		 * \return The QAction used to choose a network interface or
		 * detach the bus.
		 */
		QAction* getInterfaceAction() const;

		/*!
		 * \brief Return the QAction which is used to open a log.
		 * \return The QAction which is used to open a log.
		 */
		QAction* getLogOpenAction() const;

		/*!
		 * \brief Return the QAction which is used to start logging.
		 * \return The QAction which is used to start logging.
		 */
		QAction* getLogStartAction() const;

		/*!
		 * \brief Return the QAction which is used to choose which registers
		 * should be read.
		 * \return The QAction which is used to choose which registers are read.
		 */
		QAction* getRegisterAction() const;

		/*!
		 * \brief Return the QAction which is used to save a profile.
		 * \return The QAction which is used to save a profile.
		 */
		QAction* getProfileSaveAction() const;

		/*!
		 * \brief Return the QAction which is used to save a profile as
		 * a new file.
		 * \return The QAction which is used to save a profile as a new file.
		 */
		QAction* getProfileSaveAsAction() const;

		/*!
		 * \brief Return the QAction which is used to open a profile.
		 * \return The QAction which is used to open a profile.
		 */
		QAction* getProfileOpenAction() const;

		/*!
		 * \brief Return the QAction which is used to toggle the bus mode
		 * between Safe-Op and Op.
		 * \return The QAction which is used to toggle the bus mode between
		 * Safe-Op and Op.
		 */
		QAction* getSafeOpAction() const;

		/*!
		 * \brief Return the area in which the graph should be displayed.
		 * \return The area in which the graph should be displayed.
		 */
		QWidget* getGraphArea() const;

		/*!
		 * \brief Return the area in which the plots should be displayed.
		 * \return The area in which the plots should be displayed.
		 */
		QWidget* getPlotArea() const;

		/*!
		 * \brief Return the area in which the slave tree should be displayed.
		 * \return The area in which the slave tree should be displayed.
		 */
		QWidget* getTreeArea() const;

		/*!
		 * \brief Return the area in which the watchlist should be displayed.
		 * \return The area in which the watchlist should be displayed.
		 */
		QWidget* getWatchlistArea() const;

		/*!
		 * \brief Return the area in which the error view should be displayed.
		 * \return The area in which the error view should be displayed.
		 */
		QWidget* getErrorArea() const;

		/*!
		 * \brief Return the label used to display the framerate.
		 * \return The label used to display the framerate.
		 */
		QLabel* getFramerateLabel() const;

		/*!
		 * \brief Return the progress bar shown in the status bar.
		 * \return the progress bar.
		 */
		QProgressBar* getProgressBar() const;

		/*!
		 * \brief Return the label used for showing information of the
		 * current progress of the progress bar.
		 * \return the label.
		 */
		QLabel* getProgressLabel() const;

		/*!
		 * \brief Focus the area for the slave tree, i.e. jump to the
		 * appropriate tab.
		 */
		void focusTreeArea() const;

		/*!
		 * \brief Focus the area for the plots, i.e. jump to the
		 * appropriate tab.
		 */
		void focusPlotArea() const;

	private:
		Ui::MainWindow* ui;
	};

} // namespace etherkitten::gui
