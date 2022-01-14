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

#include "RegisterChooser.hpp"
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	RegisterChooser::RegisterChooser(QWidget* parent,
	    std::unordered_map<datatypes::RegisterEnum, bool> registersChosen,
	    std::vector<std::pair<datatypes::RegisterEnum, std::string>> registers)
	    : QDialog(parent)
	    , registersChosen(registersChosen)
	{
		setWindowTitle("Choose registers to read");
		QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* label
		    = new QLabel("Choose registers to read (registers stored in the same byte will "
		                 "be checked and unchecked automatically):");
		layout->addWidget(label);
		QListWidget* list = new QListWidget(this);
		registerItems.reserve(registers.size());
		for (auto& entry : registers)
		{
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(entry.second));
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			registerItems.emplace_back(item, entry.first);
			bool chosen = registersChosen[entry.first];
			if (chosen)
				item->setCheckState(Qt::CheckState::Checked);
			else
				item->setCheckState(Qt::CheckState::Unchecked);
			list->addItem(item);
		}
		connect(list, &QListWidget::itemChanged, [this, list](QListWidgetItem* item) {
			int index = list->row(item);
			if (static_cast<size_t>(index) >= registerItems.size())
				return;
			Qt::CheckState state = item->checkState();
			/* set all registers in the same byte to the same checkstate
			 * yes, I know this is ugly */
			int lower
			    = static_cast<int>(registerItems[static_cast<size_t>(index)].regType) & 0xFFFF;
			for (int i = index; i >= 0
			     && lower
			         == (static_cast<int>(registerItems[static_cast<size_t>(i)].regType) & 0xFFFF);
			     i--)
			{
				registerItems[static_cast<size_t>(i)].item->setCheckState(state);
			}
			for (int i = index + 1; i < static_cast<int>(registerItems.size())
			     && lower
			         == (static_cast<int>(registerItems[static_cast<size_t>(i)].regType) & 0xFFFF);
			     i++)
			{
				registerItems[static_cast<size_t>(i)].item->setCheckState(state);
			}
		});
		layout->addWidget(list);
		QDialogButtonBox* buttonBox
		    = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
		layout->addWidget(buttonBox);
		connect(buttonBox, &QDialogButtonBox::accepted, [this]() {
			for (auto& item : registerItems)
			{
				this->registersChosen[item.regType]
				    = (item.item->checkState() == Qt::CheckState::Checked);
			}
			accept();
		});
		connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	}

	std::unordered_map<datatypes::RegisterEnum, bool> RegisterChooser::getRegisters() const
	{
		return registersChosen;
	}

} // namespace etherkitten::gui
