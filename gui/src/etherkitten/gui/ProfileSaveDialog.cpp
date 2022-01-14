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

#include "ProfileSaveDialog.hpp"
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	ProfileSaveDialog::ProfileSaveDialog(QWidget* parent, std::vector<std::string> profiles)
	    : QDialog(parent)
	    , profiles(profiles)
	{
		setWindowTitle("Save profile as...");
		QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* label = new QLabel("Choose a name for the profile:", this);
		layout->addWidget(label);
		edit = new QLineEdit(this);
		layout->addWidget(edit);
		QLabel* profileLabel = new QLabel("Currently available profiles:");
		layout->addWidget(profileLabel);
		list = new QListWidget(this);
		for (auto& profile : profiles)
		{
			list->addItem(QString::fromStdString(profile));
		}
		connect(list, &QListWidget::itemClicked,
		    [this](QListWidgetItem* item) { edit->setText(item->text()); });
		layout->addWidget(list);
		QDialogButtonBox* buttons
		    = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttons, &QDialogButtonBox::accepted, [this]() {
			std::string chosen = edit->text().toStdString();
			if (chosen.find('/') != std::string::npos)
			{
				QMessageBox box(this);
				box.setWindowTitle("Invalid character in profile name");
				box.setText("Invalid character in profile name.");
				box.setStandardButtons(QMessageBox::Ok);
				box.exec();
				return;
			}
			for (auto& profile : this->profiles)
			{
				if (profile.compare(chosen) == 0)
				{
					QMessageBox box(this);
					box.setWindowTitle("Profile name exists already");
					box.setText("A name with this profile exists already.");
					box.setInformativeText("Do you want to overwrite it?");
					box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
					if (box.exec() == QMessageBox::Yes)
						break;
					else
						return;
				}
			}
			profile = chosen;
			accept();
		});
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		layout->addWidget(buttons);
	}

	std::string ProfileSaveDialog::getProfile() const { return profile; }

} // namespace etherkitten::gui
