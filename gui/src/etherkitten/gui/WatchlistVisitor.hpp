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
#include <QMenu>
#include <QWidget>
#include <etherkitten/datatypes/DataObjectVisitor.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <functional>
#include <string>

namespace etherkitten::gui
{

	/*!
	 * \brief Creates widgets for the WatchList.
	 */
	class WatchlistVisitor : public datatypes::DataObjectVisitor
	{
	public:
		/*!
		 * \brief Create a new WatchlistVisitor with the given DataModelAdapter.
		 * \param adapter The DataModelAdapter used for performing actions in the
		 * context menu (e.g. reading a CoE object).
		 */
		WatchlistVisitor(DataModelAdapter& adapter);

		/*!
		 * \brief Destroy the WatchlistVisitor.
		 */
		~WatchlistVisitor() override;

		/*!
		 * \brief Set the bus mode used for generating widgets. This is required
		 * since some objects can only be read or written in a certain mode.
		 * \param safeOP Whether the bus is in Safe-Op or Op mode.
		 */
		void setSafeOP(bool safeOP);

		/*!
		 * \brief Return the last widget that was generated.
		 * \return The generated widget.
		 */
		DataEntry* getWidget() const;

		/*!
		 * \brief Return a lambda used to generate a special context menu for the
		 * last generated widget.
		 * \return The menu generator.
		 */
		std::function<void(QMenu* menu, const datatypes::DataObject& obj, bool busLive)>
		getMenuGenerator() const;

		void handlePDO(const datatypes::PDO& object) override;
		void handleCoE(const datatypes::CoEObject& object) override;
		void handleErrorStatistic(const datatypes::ErrorStatistic& statistic) override;
		void handleRegister(const datatypes::Register& reg) override;

	private:
		bool safeOP;
		DataModelAdapter& adapter;
		DataEntry* entry;
		std::function<void(QMenu* menu, const datatypes::DataObject& obj, bool busLive)> generator;
	};

} // namespace etherkitten::gui
