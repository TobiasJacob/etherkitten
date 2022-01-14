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

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include <etherkitten/datatypes/errors.hpp>

namespace etherkitten::gui
{

	/* Forward-declare this to avoid circular includes */
	class Node;

	/*!
	 * \brief Represents a connection between two slaves in the SlaveGraph.
	 */
	class Edge : public QGraphicsItem
	{
	public:
		/*!
		 * \brief Create a new Edge.
		 * \param scene The GraphicsScene the Edge is displayed in.
		 * \param sourceNode The source Node.
		 * \param destNode The destination Node.
		 */
		Edge(QGraphicsScene* scene, Node* sourceNode, Node* destNode);

		/*!
		 * \brief Return the source node.
		 * \return the source node.
		 */
		Node* sourceNode() const;

		/*!
		 * \brief Return the destination node.
		 * \return the destination node.
		 */
		Node* destNode() const;

		/*!
		 * \brief Adjust the position after one of the Nodes has changed its position.
		 */
		void adjustPosition();

		/*!
		 * \brief Clear the error shown on this Edge.
		 */
		void clearError();

		/*!
		 * \brief Show an error on this Edge.
		 * \param msg The message to show as a tooltip.
		 * \param severity The severity of the error.
		 */
		void showError(std::string msg, datatypes::ErrorSeverity severity);
		enum
		{
			Type = UserType + 2
		};
		int type() const override { return Type; }

	protected:
		QRectF boundingRect() const override;
		void paint(
		    QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	private:
		void setErrorPos();
		Node* source;
		Node* dest;
		QPointF sourcePoint;
		QPointF destPoint;
		QGraphicsScene* scene;
		QGraphicsPixmapItem* lowImg;
		QGraphicsPixmapItem* mediumImg;
	};

} // namespace etherkitten::gui
