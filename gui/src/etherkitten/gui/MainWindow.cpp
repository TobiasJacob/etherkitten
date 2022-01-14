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

#include "MainWindow.hpp"
#include "ui_mainwindow.h"

/* workaround because these macros cannot be called from within
 * a namespace (see https://doc.qt.io/qt-5/qdir.html#Q_INIT_RESOURCE) */
void initImageResources() { Q_INIT_RESOURCE(images); }
void cleanupImageResources() { Q_CLEANUP_RESOURCE(images); }

namespace etherkitten::gui
{

	MainWindow::MainWindow(QWidget* parent)
	    : QMainWindow(parent)
	    , ui(new Ui::MainWindow)
	{
		ui->setupUi(this);
		initImageResources();
		setWindowIcon(QIcon(":images/icon.png"));
	}

	MainWindow::~MainWindow()
	{
		delete ui;
		cleanupImageResources();
	}

	QAction* MainWindow::getInterfaceAction() const { return ui->actionConnection; }

	QAction* MainWindow::getLogOpenAction() const { return ui->actionOpenLog; }

	QAction* MainWindow::getLogStartAction() const { return ui->actionStartLog; }

	QAction* MainWindow::getRegisterAction() const { return ui->actionChooseRegisters; }

	QAction* MainWindow::getProfileSaveAction() const { return ui->actionSaveProfile; }

	QAction* MainWindow::getProfileSaveAsAction() const { return ui->actionSaveProfileAs; }

	QAction* MainWindow::getProfileOpenAction() const { return ui->actionOpenProfile; }

	QAction* MainWindow::getSafeOpAction() const { return ui->actionToggleSafeOP; }

	QWidget* MainWindow::getGraphArea() const { return ui->graphTab; }

	QWidget* MainWindow::getPlotArea() const { return ui->plotsTab; }

	QWidget* MainWindow::getTreeArea() const { return ui->slavesTab; }

	QWidget* MainWindow::getWatchlistArea() const { return ui->watchlistTab; }

	QWidget* MainWindow::getErrorArea() const { return ui->errorsTab; }

	QLabel* MainWindow::getFramerateLabel() const { return ui->framerateLabel; }

	void MainWindow::focusTreeArea() const { ui->slavesErrorsTabs->setCurrentIndex(0); }

	QProgressBar* MainWindow::getProgressBar() const { return ui->progressBar; }

	QLabel* MainWindow::getProgressLabel() const { return ui->progressLabel; }

	void MainWindow::focusPlotArea() const { ui->graphPlotTabs->setCurrentIndex(1); }

} // namespace etherkitten::gui
