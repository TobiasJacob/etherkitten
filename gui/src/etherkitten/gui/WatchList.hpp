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

#include "DataEntry.hpp"
#include "DataModelAdapter.hpp"
#include "TooltipFormatter.hpp"
#include "WatchlistVisitor.hpp"
#include <QMenu>
#include <QPoint>
#include <QString>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Displays the watchlist, i.e. a list of data objects the user
	 * has chosen to display in order to watch them more easily.
	 */
	class WatchList : public QFrame
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new WatchList.
		 * \param parent The parent widget.
		 * \param dataAdapter The DataModelAdapter used to communicate with the backend.
		 * \param tooltipFormatter The TooltipFormatter used to generate tooltips.
		 */
		WatchList(
		    QWidget* parent, DataModelAdapter& dataAdapter, TooltipFormatter& tooltipFormatter);

		/*!
		 * \brief Update the displayed data from the backend.
		 */
		void updateData();

		/*!
		 * \brief Remove all items in the watchlist.
		 */
		void clear();

		/*!
		 * \brief Set the bus mode. This is required because some data objects can only
		 * be read or written in one of the modes.
		 * \param safeOP Whether the bus is in Safe-Op or Op mode.
		 */
		void setSafeOP(bool safeOP);

		/*!
		 * \brief Set whether the bus is live (actually reading from an interface) or
		 * not (reading a log or currently not connected to an interface).
		 * \param busLive whether the bus is live or not.
		 */
		void setBusLive(bool busLive);

		/*!
		 * \brief Register a lambda that adds QActions or QMenus to the given context menu
		 * for the given DataObject.
		 * \param generator The menu generator.
		 */
		void registerMenuGenerator(
		    std::function<void(QMenu* menu, const datatypes::DataObject& obj)> generator);

	public slots:
		/*!
		 * \brief Add a DataObject to be displayed in the WatchList.
		 * \param data The DataObject to add.
		 */
		void addData(const datatypes::DataObject& data);

		/*!
		 * \brief Write the data in the currently selected entry.
		 * \param obj The DataObject to write.
		 * \param entry The DataEntry containing the text.
		 */
		void writeData(const datatypes::DataObject& obj, DataEntry* entry);

		/*!
		 * \brief Remove the currently selected entry.
		 */
		void removeData();

		/*!
		 * \brief Set the displayed format of the current entry to decimal.
		 */
		void setFormatDec();

		/*!
		 * \brief Set the displayed format of the current entry to hexadecimal.
		 */
		void setFormatHex();

		/*!
		 * \brief Set the displayed format of the current entry to binary.
		 */
		void setFormatBin();

	protected:
		void showEvent(QShowEvent* event) override;

	signals:
		/*!
		 * \brief Signal that an error has occurred.
		 * \param msg The error message.
		 */
		void errorMessage(std::string msg);

	private slots:
		/*!
		 * \brief Show a context menu for the item under the cursor.
		 * \param pos The cursor position.
		 */
		void showContextMenu(const QPoint& pos);

	private:
		/*!
		 * \brief Set the displayed format of the current entry to the given base.
		 * \param base The base.
		 */
		void setFormat(datatypes::NumberFormat base);

		/*!
		 * \brief Set the slave ID for an item in the tree widget. This checks if
		 * the slave ID is valid and sets it to "N/A" instead (e.g. for overall error
		 * statistics).
		 * \param item The item
		 * \param slaveID The slave ID
		 */
		void setSlaveID(QTreeWidgetItem* item, unsigned int slaveID);

		struct DataObjectMetadata
		{
			DataObjectMetadata(datatypes::NumberFormat base, datatypes::TimeStamp lastTime,
			    bool dirty, const datatypes::DataObject& data,
			    std::unique_ptr<datatypes::AbstractNewestValueView> view, DataEntry* entry,
			    std::function<void(QMenu* menu, const datatypes::DataObject& obj, bool busLive)>
			        generator)
			    : base(base)
			    , lastTime(lastTime)
			    , dirty(dirty)
			    , data(data)
			    , view(std::move(view))
			    , entry(entry)
			    , generator(generator)
			{
			}
			DataObjectMetadata(DataObjectMetadata&& other)
			    : base(other.base)
			    , lastTime(other.lastTime)
			    , dirty(other.dirty)
			    , data(other.data)
			    , view(std::move(other.view))
			    , entry(other.entry)
			    , generator(other.generator)
			{
			}
			datatypes::NumberFormat base;
			datatypes::TimeStamp lastTime;
			bool dirty;
			std::reference_wrapper<const datatypes::DataObject> data;
			std::unique_ptr<datatypes::AbstractNewestValueView> view;
			DataEntry* entry;
			std::function<void(QMenu* menu, const datatypes::DataObject& obj, bool busLive)>
			    generator;
			DataObjectMetadata& operator=(DataObjectMetadata&& other)
			{
				base = other.base;
				lastTime = other.lastTime;
				dirty = other.dirty;
				data = other.data;
				view = std::move(other.view);
				entry = other.entry;
				generator = other.generator;
				return *this;
			}
		};

		DataModelAdapter& dataAdapter;
		TooltipFormatter& tooltipFormatter;
		WatchlistVisitor widgetFormatter;
		QTreeWidget* treeWidget;
		QVBoxLayout* layout;
		std::vector<DataObjectMetadata> entries;
		std::vector<std::function<void(QMenu* menu, const datatypes::DataObject& obj)>>
		    menuGenerators;
		bool safeOP;
		bool busLive;
	};

} // namespace etherkitten::gui
