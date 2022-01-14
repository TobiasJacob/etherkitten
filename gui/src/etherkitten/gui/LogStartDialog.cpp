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

#include "LogStartDialog.hpp"
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>

namespace etherkitten::gui
{

	LogStartDialog::LogStartDialog(QWidget* parent)
	    : QDialog(parent)
	    , totalSeconds(std::chrono::seconds::max())
	{
		setWindowTitle("Start logging");
		QVBoxLayout* layout = new QVBoxLayout(this);
		QLabel* label = new QLabel("Choose offset for log:");
		layout->addWidget(label);
		checkbox = new QCheckBox("Save all data");
		checkbox->setCheckState(Qt::CheckState::Checked);
		connect(checkbox, &QCheckBox::stateChanged, [this](int state) {
			if (state == Qt::CheckState::Checked)
			{
				totalSeconds = std::chrono::seconds::max();
				seconds->setDisabled(true);
				minutes->setDisabled(true);
				hours->setDisabled(true);
			}
			else
			{
				seconds->setEnabled(true);
				minutes->setEnabled(true);
				hours->setEnabled(true);
			}
		});
		layout->addWidget(checkbox);
		QLabel* timeLabel
		    = new QLabel("Save data starting at the following duration in the past (hh:mm:ss):");
		layout->addWidget(timeLabel);
		QFrame* frame = new QFrame(this);
		QHBoxLayout* timeLayout = new QHBoxLayout(frame);
		seconds = new QSpinBox(frame);
		minutes = new QSpinBox(frame);
		hours = new QSpinBox(frame);
		seconds->setRange(0, 59);
		minutes->setRange(0, 59);
		hours->setMinimum(0);
		seconds->setValue(0);
		minutes->setValue(0);
		hours->setValue(0);
		seconds->setDisabled(true);
		minutes->setDisabled(true);
		hours->setDisabled(true);
		timeLayout->addWidget(hours);
		timeLayout->addWidget(minutes);
		timeLayout->addWidget(seconds);
		timeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		layout->addWidget(frame);
		QDialogButtonBox* box
		    = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(box, &QDialogButtonBox::accepted, [this]() {
			if (checkbox->checkState() == Qt::CheckState::Unchecked)
			{
				totalSeconds = std::chrono::seconds(
				    hours->value() * 3600 + minutes->value() * 60 + seconds->value());
			}
			accept();
		});
		connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
		layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
		layout->addWidget(box);
	}

	std::chrono::seconds LogStartDialog::getOffset() const { return totalSeconds; }

} // namespace etherkitten::gui
