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

#include <QDialog>
#include <QListWidget>
#include <QWidget>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Asks the user which registers should be read.
	 */
	class RegisterChooser : public QDialog
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new RegisterChooser.
		 * \param parent The parent widget/window.
		 * \param registersChosen A map storing which registers are currently being read.
		 * \param registers A mapping storing the display names for the registers.
		 */
		RegisterChooser(QWidget* parent,
		    std::unordered_map<datatypes::RegisterEnum, bool> registersChosen,
		    std::vector<std::pair<datatypes::RegisterEnum, std::string>> registers);
		/*!
		 * \brief Return a map storing which registers are currently being read.
		 * \return a map storing which registers are currently being read.
		 */
		std::unordered_map<datatypes::RegisterEnum, bool> getRegisters() const;

	private:
		struct RegisterItem
		{
			RegisterItem(QListWidgetItem* item, datatypes::RegisterEnum regType)
			    : item(item)
			    , regType(regType)
			{
			}
			QListWidgetItem* item;
			datatypes::RegisterEnum regType;
		};
		std::vector<RegisterItem> registerItems;
		std::unordered_map<datatypes::RegisterEnum, bool> registersChosen;
	};

} // namespace etherkitten::gui
