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

#include "ProfileOpenDialog.hpp"
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	ProfileOpenDialog::ProfileOpenDialog(QWidget* parent, std::vector<std::string> profiles)
	    : QDialog(parent)
	{
		setWindowTitle("Open profile");
		QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* label = new QLabel("Choose a profile to load:", this);
		layout->addWidget(label);
		box = new QComboBox(this);
		for (auto& profile : profiles)
		{
			box->addItem(QString::fromStdString(profile));
		}
		layout->addWidget(box);
		QDialogButtonBox* buttons
		    = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		if (profiles.empty())
			buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
		connect(buttons, &QDialogButtonBox::accepted, [this]() {
			profile = box->currentText().toStdString();
			accept();
		});
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
		layout->addWidget(buttons);
	}

	std::string ProfileOpenDialog::getProfile() const { return profile; }

} // namespace etherkitten::gui
