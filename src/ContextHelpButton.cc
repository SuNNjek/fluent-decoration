/*
 * Copyright (C) 2020 SuNNjek <sunnerlp@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// own
#include "ContextHelpButton.h"
#include "Decoration.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// Qt
#include <QPainter>
#include <QPainterPath>

namespace Fluent
{
    ContextHelpButton::ContextHelpButton(Decoration *decoration, QObject *parent)
        : FluentDecorationButton(KDecoration2::DecorationButtonType::ContextHelp, decoration, parent)
    {
        auto *decoratedClient = decoration->client().toStrongRef().data();
        connect(decoratedClient, &KDecoration2::DecoratedClient::providesContextHelpChanged,
                this, &ContextHelpButton::setVisible);

        setVisible(decoratedClient->providesContextHelp());
    }

    void ContextHelpButton::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        const QRectF buttonRect = geometry();

        painter->save();

        painter->setRenderHints(QPainter::Antialiasing, true);

        // Background
        painter->setPen(Qt::NoPen);
        painter->setBrush(backgroundColor());
        painter->drawRect(buttonRect);

        // Foreground
        painter->setPen(foregroundColor());
        painter->setBrush(Qt::NoBrush);

        QFont font = painter->font();
        font.setPointSize(11);
        painter->setFont(font);

        painter->drawText(buttonRect, Qt::AlignCenter, "?");

        painter->restore();
    }
}
