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

#include "DataEntry.hpp"

namespace etherkitten::gui
{

	EditableDataEntry::EditableDataEntry()
	    : oldText("")
	    , isEditing(false)
	{
		layout = new QHBoxLayout(this);
		writeButton = new QPushButton("Write");
		writeButton->setToolTip(
		    "Write the current value to the slave. The value is interpreted like the displayed "
		    "values, i.e. the the prefixes 0x and 0b cause the value to be interpreted as "
		    "hexadecimal and binary, respectively.");
		resetButton = new QPushButton("Reset");
		resetButton->setToolTip(
		    "Reset the value to what it was before editing was started and start updating it "
		    "again. This is only a local action, it does not write any information to the slave.");
		resetButton->setEnabled(false);
		editLabel = new QLabel("*");
		textEdit = new QLineEdit();
		spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding);
		layout->addWidget(textEdit);
		layout->addWidget(editLabel);
		editLabel->setHidden(true);
		layout->addItem(spacer);
		layout->addWidget(writeButton);
		layout->addWidget(resetButton);
		connect(writeButton, &QPushButton::clicked, [this]() {
			oldText = textEdit->text();
			emit requestWrite();
		});
		connect(resetButton, &QPushButton::clicked, [this]() {
			editLabel->setHidden(true);
			textEdit->setText(oldText);
			resetButton->setEnabled(false);
			isEditing = false;
		});
		connect(textEdit, &QLineEdit::textEdited, [this](const QString& text) {
			(void)text;
			if (!isEditing)
			{
				editLabel->setHidden(false);
				resetButton->setEnabled(true);
			}
			isEditing = true;
		});
	}

	void EditableDataEntry::setEditing(bool edit)
	{
		editLabel->setHidden(!edit);
		isEditing = edit;
		resetButton->setEnabled(edit);
	}

	bool EditableDataEntry::editing() const { return isEditing; }

	std::string EditableDataEntry::getValue() const { return textEdit->text().toStdString(); }

	void EditableDataEntry::setValue(std::string value)
	{
		oldText = QString::fromStdString(value);
		textEdit->setText(oldText);
	}

	void EditableDataEntry::setActive(bool active) { writeButton->setEnabled(active); }

} // namespace etherkitten::gui
