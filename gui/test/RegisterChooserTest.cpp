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

#include <etherkitten/gui/RegisterChooser.hpp>
#include <QApplication>
#include <iostream>
#include <map>
#include <unordered_map>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	std::unordered_map<etherkitten::datatypes::RegisterEnum, bool> registersChosen;
	std::vector<std::pair<etherkitten::datatypes::RegisterEnum, std::string>> registers;
	registers.emplace_back(std::make_pair(etherkitten::datatypes::RegisterEnum::TYPE, "Type"));
	registersChosen[etherkitten::datatypes::RegisterEnum::TYPE] = false;
	registers.emplace_back(std::make_pair(etherkitten::datatypes::RegisterEnum::BUILD, "Build"));
	registersChosen[etherkitten::datatypes::RegisterEnum::TYPE] = true;

	etherkitten::gui::RegisterChooser chooser(nullptr, registersChosen, registers);
	if (chooser.exec() == QDialog::Accepted)
	{
		for (auto& reg : chooser.getRegisters())
		{
			std::cout << reg.second << " " << etherkitten::datatypes::registerMap.at(reg.first).name
			          << std::endl;
		}
	}
}
