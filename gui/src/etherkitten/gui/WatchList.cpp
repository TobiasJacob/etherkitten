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

#include "WatchList.hpp"
#include "util.hpp"
#include <QMessageBox>

namespace etherkitten::gui
{

	WatchList::WatchList(
	    QWidget* parent, DataModelAdapter& dataAdapter, TooltipFormatter& tooltipFormatter)
	    : QFrame(parent)
	    , safeOP(false)
	    , busLive(false)
	    , dataAdapter(dataAdapter)
	    , tooltipFormatter(tooltipFormatter)
	    , widgetFormatter(dataAdapter)
	{
		treeWidget = new QTreeWidget(this);
		layout = new QVBoxLayout(this);
		layout->addWidget(treeWidget);
		treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(treeWidget, &QTreeWidget::customContextMenuRequested, this,
		    &WatchList::showContextMenu);
		QTreeWidgetItem* header = treeWidget->headerItem();
		header->setText(0, "Slave");
		header->setText(1, "Name");
		header->setText(2, "Value");
		treeWidget->setColumnCount(3);
		registerMenuGenerator([this](QMenu* menu, const datatypes::DataObject& obj) {
			QAction* remove = new QAction("Remove from watchlist");
			menu->addAction(remove);
			connect(remove, &QAction::triggered, this, &WatchList::removeData);
			if (!dataObjectIsNumeric(obj))
				return;
			QMenu* formatMenu = new QMenu("Number format");
			QAction* dec = new QAction("Decimal");
			QAction* hex = new QAction("Hexadecimal");
			QAction* bin = new QAction("Binary");
			formatMenu->addAction(dec);
			formatMenu->addAction(hex);
			formatMenu->addAction(bin);
			menu->addMenu(formatMenu);
			connect(dec, &QAction::triggered, this, &WatchList::setFormatDec);
			connect(hex, &QAction::triggered, this, &WatchList::setFormatHex);
			connect(bin, &QAction::triggered, this, &WatchList::setFormatBin);
		});
	}

	void WatchList::showEvent(QShowEvent* event)
	{
		/* force update so there is no delay when it becomes visible */
		updateData();
		QFrame::showEvent(event);
	}

	void WatchList::updateData()
	{
		if (!this->isVisible())
			return;
		for (size_t i = 0; i < entries.size(); i++)
		{
			auto& entry = entries[i];
			if (entry.view->isEmpty())
				continue;
			QTreeWidgetItem* item = treeWidget->topLevelItem(static_cast<int>(i));
			if (entry.dirty || (entry.lastTime != (**entry.view).getTime()))
			{
				std::string newValue = (**entry.view).asString(entry.base);
				if (!entry.entry)
					item->setText(2, QString::fromStdString(newValue));
				else if (!entry.entry->editing())
					entry.entry->setValue(newValue);
				entry.dirty = false;
			}
		}
	}

	void WatchList::clear()
	{
		treeWidget->clear();
		entries.clear();
	}

	void WatchList::setSlaveID(QTreeWidgetItem* item, unsigned int slaveID)
	{
		if (slaveID < std::numeric_limits<unsigned int>::max())
			item->setText(0, QString::number(slaveID));
		else
			item->setText(0, "N/A");
	}

	void WatchList::setSafeOP(bool safeOP)
	{
		this->safeOP = safeOP;
		widgetFormatter.setSafeOP(safeOP);
		treeWidget->clear();
		for (size_t i = 0; i < entries.size(); ++i)
		{
			const datatypes::DataObject& obj = entries[i].data.get();
			obj.acceptVisitor(widgetFormatter);
			DataEntry* entry = widgetFormatter.getWidget();
			entries[i].entry = entry;
			entries[i].generator = widgetFormatter.getMenuGenerator();
			QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
			setSlaveID(item, obj.getSlaveID());
			item->setText(1, QString::fromStdString(obj.getName()));
			if (entry)
			{
				entry->setActive(busLive);
				treeWidget->setItemWidget(item, 2, entries[i].entry);
				connect(entry, &DataEntry::requestWrite,
				    [this, entry, i]() { writeData(this->entries[i].data.get(), entry); });
			}
			entries[i].dirty = true;
		}
	}

	void WatchList::setBusLive(bool busLive)
	{
		this->busLive = busLive;
		for (auto& m : entries)
		{
			if (m.entry)
				m.entry->setActive(busLive);
		}
	}

	void WatchList::registerMenuGenerator(
	    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator)
	{
		menuGenerators.emplace_back(generator);
	}

	void WatchList::addData(const datatypes::DataObject& data)
	{
		data.acceptVisitor(widgetFormatter);
		data.acceptVisitor(tooltipFormatter);
		DataEntry* entry = widgetFormatter.getWidget();
		entries.emplace_back(datatypes::NumberFormat::DECIMAL, datatypes::TimeStamp(), true, data,
		    dataAdapter.getNewestValueView(data), entry, widgetFormatter.getMenuGenerator());
		QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget);
		setSlaveID(item, data.getSlaveID());
		item->setText(1, QString::fromStdString(data.getName()));
		item->setToolTip(2, tooltipFormatter.getTooltip());
		if (entry)
		{
			entry->setActive(busLive);
			treeWidget->setItemWidget(item, 2, entry);
			connect(entry, &DataEntry::requestWrite,
			    [this, entry, &data]() { writeData(data, entry); });
		}
	}

	void WatchList::writeData(const datatypes::DataObject& obj, DataEntry* entry)
	{
		if (!busLive)
			return;
		try
		{
			dataAdapter.writeData(obj, entry->getValue());
			entry->setEditing(false);
		}
		catch (std::exception& e)
		{
			QMessageBox::warning(
			    this, "Error", "Error writing data:\n" + QString::fromStdString(e.what()));
		}
	}

	void WatchList::removeData()
	{
		QTreeWidgetItem* item = treeWidget->currentItem();
		if (!item)
			return;
		int row = treeWidget->indexOfTopLevelItem(item);
		if (row < 0)
			return;
		DataObjectMetadata& m = entries.at(static_cast<std::size_t>(row));
		treeWidget->takeTopLevelItem(row);
		delete item;
		if (m.entry)
			m.entry->deleteLater();
		entries.erase(entries.begin() + row);
	}

	void WatchList::setFormat(datatypes::NumberFormat base)
	{
		QTreeWidgetItem* item = treeWidget->currentItem();
		if (!item)
			return;
		int row = treeWidget->indexOfTopLevelItem(item);
		if (row < 0)
			return;
		DataObjectMetadata& m = entries.at(static_cast<std::size_t>(row));
		/* set editing to false so data is definitely re-displayed with
		 * the new base on the next update cycle */
		if (m.entry && m.entry->editing())
			m.entry->setEditing(false);
		m.base = base;
		m.dirty = true;
	}

	void WatchList::setFormatDec() { setFormat(datatypes::NumberFormat::DECIMAL); }

	void WatchList::setFormatHex() { setFormat(datatypes::NumberFormat::HEXADECIMAL); }

	void WatchList::setFormatBin() { setFormat(datatypes::NumberFormat::BINARY); }

	void WatchList::showContextMenu(const QPoint& pos)
	{
		QTreeWidgetItem* item = treeWidget->itemAt(pos);
		if (!item)
			return;
		/* I don't know if this is necessary since right click
		 * usually sets the selected item as well, but I set it here
		 * just to be safe because most of the slots require it */
		treeWidget->setCurrentItem(item);
		int row = treeWidget->indexOfTopLevelItem(item);
		DataObjectMetadata& m = entries.at(static_cast<std::size_t>(row));
		QMenu menu(this);
		for (auto fn : menuGenerators)
		{
			fn(&menu, m.data);
		}
		m.generator(&menu, m.data, busLive);
		menu.exec(treeWidget->mapToGlobal(pos));
		menu.clear();
	}

} // namespace etherkitten::gui
