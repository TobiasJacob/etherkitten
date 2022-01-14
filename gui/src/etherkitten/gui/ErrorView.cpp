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

#include "ErrorView.hpp"
#include <QMessageBox>
#include <QVBoxLayout>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace etherkitten::gui
{

	ErrorView::ErrorView(QWidget* parent, DataModelAdapter& dataAdapter, BusInfoSupplier& busInfo,
	    TooltipFormatter& tooltipFormatter)
	    : QFrame(parent)
	    , dataAdapter(dataAdapter)
	    , busInfo(busInfo)
	    , tooltipFormatter(tooltipFormatter)
	    , errorIterator(nullptr)
	    , curColumn(0)
	    , busLive(false)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		errorLog = new QTreeWidget(this);
		errorLog->setColumnCount(4);
		QTreeWidgetItem* logHeader = errorLog->headerItem();
		logHeader->setText(0, "Time");
		logHeader->setText(1, "Slave(s)");
		logHeader->setText(2, "Severity");
		logHeader->setText(3, "Error");
		statisticList = new QTreeWidget(this);
		statisticList->setColumnCount(3);
		QTreeWidgetItem* statsHeader = statisticList->headerItem();
		statsHeader->setText(0, "Name");
		statsHeader->setText(1, "Total");
		statsHeader->setText(2, "Frequency");
		statisticList->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(statisticList, &QTreeWidget::customContextMenuRequested, this,
		    &ErrorView::showContextMenu);

		resetBtn = new QPushButton("Reset all error registers");
		resetBtn->setToolTip("Reset the error registers of all slaves.");
		connect(resetBtn, &QPushButton::clicked, [this]() {
			try
			{
				this->dataAdapter.resetAllErrorRegisters();
			}
			catch (std::exception& e)
			{
				QMessageBox::warning(this, "Error",
				    "Error resetting registers:\n" + QString::fromStdString(e.what()));
			}
		});
		resetBtn->setDisabled(true);
		QPushButton* clearBtn = new QPushButton("Clear error log");
		clearBtn->setToolTip("Clear the log of error messages. This only clears the messages "
		                     "displayed in the GUI, it does not write any data to the slaves.");
		connect(clearBtn, &QPushButton::clicked, [this]() { errorLog->clear(); });
		QFrame* btnFrame = new QFrame(this);
		QHBoxLayout* btnLayout = new QHBoxLayout(btnFrame);
		btnLayout->addWidget(resetBtn);
		btnLayout->addWidget(clearBtn);

		layout->addWidget(errorLog);
		layout->addWidget(statisticList);
		layout->addWidget(btnFrame);

		/* enable the itemEntered signals - I couldn't find any way to
		 * get the column under the mouse when the context menu is
		 * shown, so now the current column of the mouse pointer is
		 * constantly tracked */
		statisticList->setMouseTracking(true);
		connect(
		    statisticList, &QTreeWidget::itemEntered, [this](QTreeWidgetItem* item, int column) {
			    (void)item;
			    curColumn = column;
		    });
	}

	void ErrorView::showEvent(QShowEvent* event)
	{
		/* force update when tab is shown so there is no short delay */
		updateStatistics();
		QFrame::showEvent(event);
	}

	void ErrorView::updateStatistics()
	{
		if (!this->isVisible())
			return;
		for (auto& stat : statistics)
		{
			if (!stat.view->isEmpty() && (**stat.view).getTime() != stat.lastTime)
			{
				stat.lastTime = (**stat.view).getTime();
				stat.item->setText(stat.column,
				    QString::fromStdString(
				        (**stat.view).asString(datatypes::NumberFormat::DECIMAL)));
			}
		}
	}

	void ErrorView::handleErrorMessage(const datatypes::DataPoint<datatypes::ErrorMessage>&& data)
	{
		datatypes::ErrorMessage msg = data.getValue();
		QTreeWidgetItem* item = new QTreeWidgetItem(errorLog);

		long total
		    = std::chrono::duration_cast<std::chrono::milliseconds>(data.getTime() - startTime)
		          .count();
		int millis = total % 1000;
		total /= 1000;
		int secs = total % 60;
		total /= 60;
		int mins = total % 60;
		total /= 60;
		int hours = static_cast<int>(total);
		std::ostringstream ss;
		ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << mins << ":"
		   << std::setw(2) << secs << "." << std::setw(3) << millis;
		item->setText(0, QString::fromStdString(ss.str()));

		item->setText(3, QString::fromStdString(msg.getMessage()));
		switch (msg.getSeverity())
		{
		case datatypes::ErrorSeverity::LOW:
			item->setText(2, "Low");
			break;
		case datatypes::ErrorSeverity::MEDIUM:
			item->setText(2, "Medium");
			break;
		case datatypes::ErrorSeverity::FATAL:
			item->setText(2, "Fatal");
			emit fatalError(msg.getMessage());
			return;
		default:
			item->setText(2, "Unknown");
			break;
		}
		auto slaves = msg.getAssociatedSlaves();
		unsigned int max = std::numeric_limits<unsigned int>::max();
		QString slaveStr;
		if (slaves.first < max && slaves.second < max)
			slaveStr = QString::number(slaves.first) + ", " + QString::number(slaves.second);
		else if (slaves.first < max)
			slaveStr = QString::number(slaves.first);
		else if (slaves.second < max)
			slaveStr = QString::number(slaves.second);
		else
			slaveStr = "N/A";
		item->setText(1, slaveStr);
	}

	void ErrorView::updateData()
	{
		updateStatistics();
		if (!errorIterator)
			return;
		while (errorIterator->hasNext())
		{
			++(*errorIterator);
			handleErrorMessage(**errorIterator);
		}
	}

	void ErrorView::setupData()
	{
		clear();
		resetBtn->setEnabled(busLive);
		std::unordered_map<datatypes::ErrorStatisticType,
		    std::reference_wrapper<const datatypes::ErrorStatistic>>
		    m;
		for (auto& obj : busInfo.getErrorStatistics())
		{
			m.emplace(obj.get().getStatisticType(), obj.get());
		}
		for (auto& stat : datatypes::errorStatisticInfos)
		{
			QTreeWidgetItem* item = new QTreeWidgetItem(statisticList);
			item->setText(0, QString::fromStdString(stat.getName()) + " errors");
			auto total = stat.getType(datatypes::ErrorStatisticCategory::TOTAL_GLOBAL);
			auto freq = stat.getType(datatypes::ErrorStatisticCategory::FREQ_GLOBAL);
			/* these should always be found during normal operation */
			auto iter = m.find(total);
			if (iter != m.end())
			{
				auto& obj = iter->second.get();
				statistics.emplace_back(
				    obj, dataAdapter.getNewestValueView(obj), 1, item, datatypes::TimeStamp());
				obj.acceptVisitor(tooltipFormatter);
				item->setToolTip(1, tooltipFormatter.getTooltip());
			}
			iter = m.find(freq);
			if (iter != m.end())
			{
				auto& obj = iter->second.get();
				statistics.emplace_back(
				    obj, dataAdapter.getNewestValueView(obj), 2, item, datatypes::TimeStamp());
				obj.acceptVisitor(tooltipFormatter);
				item->setToolTip(2, tooltipFormatter.getTooltip());
			}
			statisticList->addTopLevelItem(item);
		}
		startTime = busInfo.getStartTime();
		errorIterator = busInfo.getErrorLog();
		/* get the first message in case it exists */
		if (!errorIterator->isEmpty())
			handleErrorMessage(**errorIterator);
		updateStatistics();
	}

	void ErrorView::clear()
	{
		errorLog->clear();
		errorIterator.reset();
		statisticList->clear();
		statistics.clear();
	}

	void ErrorView::setBusLive(bool busLive)
	{
		this->busLive = busLive;
		resetBtn->setEnabled(busLive);
	}

	void ErrorView::registerMenuGenerator(
	    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator)
	{
		menuGenerators.emplace_back(generator);
	}

	void ErrorView::showContextMenu(const QPoint& pos)
	{
		QTreeWidgetItem* item = statisticList->itemAt(pos);
		/* Note: This is, of course, a very inefficient way to find the correct item.
		 * It is, however, the easiest way considering how the tree is currently
		 * structured, and it shouldn't really be an issue because there aren't very
		 * many items in the list at the moment. */
		for (auto& stat : statistics)
		{
			if (stat.item == item && stat.column == curColumn)
			{
				QMenu menu(this);
				for (auto fn : menuGenerators)
				{
					fn(&menu, stat.statistic);
				}
				if (menu.isEmpty())
					return;
				menu.exec(statisticList->mapToGlobal(pos));
				menu.clear();
				break;
			}
		}
	}

} // namespace etherkitten::gui
