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
#include "TooltipFormatter.hpp"
#include <QFrame>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>
#include <etherkitten/datatypes/errors.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Shows error statistics and a log of all errors.
	 */
	class ErrorView : public QFrame
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new ErrorView.
		 * \param parent The parent widget.
		 * \param dataAdapter The DataModelAdapter with which to obtain NewestValueViews.
		 * \param busInfo The BusInfoSupplier with which to obtain the data.
		 * \param tooltipFormatter The TooltipFormatter to create tooltips for the statistics.
		 */
		ErrorView(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
		    TooltipFormatter& tooltipFormatter);
		/*!
		 * \brief Update all data shown, if the ErrorView is active.
		 */
		void updateData();
		/*!
		 * \brief Obtain all needed data from the BusInfoSupplier and DataModelAdapter,
		 * set up the GUI for them, and set the ErrorView to active.
		 */
		void setupData();
		/*!
		 * \brief Clear all displayed data.
		 */
		void clear();
		/*!
		 * \brief Set whether the bus is live (connected to an interface) or not.
		 * \param busLive whether the bus is live or not.
		 */
		void setBusLive(bool busLive);
		/*!
		 * \brief Register a lambda that is called when an error statistic is
		 * right-clicked to generate a context menu for it.
		 * \param generator The lambda function to call.
		 */
		void registerMenuGenerator(
		    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator);

	protected:
		void showEvent(QShowEvent* event) override;

	private slots:
		/*!
		 * \brief Show a context menu for the object at the given position.
		 * \param pos The position for show the menu at.
		 */
		void showContextMenu(const QPoint& pos);

	signals:
		/*!
		 * \brief Signal that a fatal error has occurred which requires the
		 * bus to be shut down.
		 * \param msg The error message.
		 */
		void fatalError(std::string msg);

	private:
		/*!
		 * \brief Update the shown error statistics.
		 */
		void updateStatistics();
		/*!
		 * \brief Handle an error message.
		 * \param data The error message to handle.
		 */
		void handleErrorMessage(const datatypes::DataPoint<datatypes::ErrorMessage>&& data);

		struct Statistic
		{
			Statistic(const datatypes::ErrorStatistic& statistic,
			    std::unique_ptr<datatypes::AbstractNewestValueView> view, int column,
			    QTreeWidgetItem* item, datatypes::TimeStamp lastTime)
			    : statistic(statistic)
			    , view(std::move(view))
			    , column(column)
			    , item(item)
			    , lastTime(lastTime)
			{
			}
			std::reference_wrapper<const datatypes::ErrorStatistic> statistic;
			std::unique_ptr<datatypes::AbstractNewestValueView> view;
			int column;
			QTreeWidgetItem* item;
			datatypes::TimeStamp lastTime;
		};
		DataModelAdapter& dataAdapter;
		BusInfoSupplier& busInfo;
		TooltipFormatter& tooltipFormatter;
		std::vector<std::function<void(QMenu* menu, const datatypes::DataObject& obj)>>
		    menuGenerators;
		QTreeWidget* errorLog;
		QTreeWidget* statisticList;
		QPushButton* resetBtn;
		std::shared_ptr<datatypes::ErrorIterator> errorIterator;
		std::vector<Statistic> statistics;
		datatypes::TimeStamp
		    startTime; /* beginning of time, used to calculate the time shown in the log */
		int curColumn; /* column under mouse */
		bool busLive;
	};

} // namespace etherkitten::gui
