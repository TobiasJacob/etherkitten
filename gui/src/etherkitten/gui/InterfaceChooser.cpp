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

#include "InterfaceChooser.hpp"
#include <QDialogButtonBox>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	InterfaceChooser::InterfaceChooser(QWidget* parent, std::vector<std::string> interfaces)
	    : QDialog(parent)
	{
		setWindowTitle("Choose network interface");
		QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* label = new QLabel("Choose network interface:");
		box = new QComboBox(this);
		for (auto& interface : interfaces)
		{
			box->addItem(QString::fromStdString(interface));
		}
		QDialogButtonBox* buttons
		    = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		checkbox = new QCheckBox("Start logging immediately", this);
		checkbox->setCheckState(Qt::CheckState::Checked);
		layout->addWidget(label);
		layout->addWidget(box);
		layout->addWidget(checkbox);
		layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
		layout->addWidget(buttons);
		connect(buttons, &QDialogButtonBox::accepted, [this]() {
			startLoggingImmediately = (checkbox->checkState() == Qt::CheckState::Checked);
			interface = box->currentText().toStdString();
			accept();
		});
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	}

	std::string InterfaceChooser::getInterface() const { return interface; }

	bool InterfaceChooser::getStartLoggingImmediately() const { return startLoggingImmediately; }

} // namespace etherkitten::gui
