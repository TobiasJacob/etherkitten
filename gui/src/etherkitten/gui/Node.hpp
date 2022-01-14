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
#include <QGraphicsSceneMouseEvent>
#include <QList>
#include <QPainter>
#include <QRectF>
#include <QStyleOptionGraphicsItem>
#include <QVariant>
#include <QWidget>
#include <etherkitten/datatypes/errors.hpp>
#include <functional>

namespace etherkitten::gui
{

	/* Forward-declare this to avoid circular includes */
	class Edge;

	/*!
	 * \brief Represents a slave in the SlaveGraph.
	 */
	class Node : public QGraphicsItem
	{
	public:
		enum NodeStatus
		{
			SAFE_OP,
			OP,
			UNKNOWN
		};
		/*!
		 * \brief Create a new Node.
		 * \param scene The GraphicsScene this Node is displayed in.
		 * \param id The slave ID.
		 * \param clickFunction A function that is called when the Node is double-clicked.
		 */
		Node(QGraphicsScene* scene, std::string name, unsigned int id,
		    std::function<void()> clickFunction);

		/*!
		 * \brief Clear all errors shown on this slave or on connected edges.
		 */
		void clearErrors();

		/*!
		 * \brief Show an error on the connection between this slave and another one.
		 * \param otherSlave The ID of the other slave.
		 * \param msg The error message to show as a tooltip.
		 * \param severity The severity of the error.
		 */
		void showConnectionError(
		    unsigned int otherSlave, std::string msg, datatypes::ErrorSeverity severity);

		/*!
		 * \brief Show an error on this slave.
		 * \param msg The error message to show as a tooltip.
		 * \param severity The severity of the error.
		 */
		void showSlaveError(std::string msg, datatypes::ErrorSeverity severity);

		/*!
		 * \brief Add an edge that is connected to this slave.
		 * \param edge The edge to add.
		 */
		void addEdge(Edge* edge);

		/*!
		 * \brief Return the slave ID of this Node.
		 * \return the slave ID of this Node.
		 */
		unsigned int getSlaveID() const;

		/*!
		 * \brief Set the current status of the node.
		 * \param status The new status
		 */
		void setStatus(NodeStatus status);

		/*!
		 * \brief Get the current status of the node.
		 * \return the current status.
		 */
		NodeStatus getStatus() const;

		QPainterPath shape() const override;
		void paint(
		    QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		enum
		{
			Type = UserType + 1
		};
		int type() const override { return Type; }
		QRectF boundingRect() const override;

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
		void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

	private:
		/* maximum shown length of slave before "..." is inserted */
		static const size_t MAX_NAME_LENGTH = 10;
		static const int TEXT_PADDING = 20;

		unsigned int id;
		NodeStatus status;
		QList<Edge*> edges;
		QGraphicsPixmapItem* lowImg;
		QGraphicsPixmapItem* mediumImg;
		QGraphicsTextItem* text;
		QGraphicsScene* scene;
		QString label;
		double textWidth, textHeight;
		int width, height;
		std::function<void()> clickFunction; /* because QGraphicsItems can't use signals */
	};

} // namespace etherkitten::gui
