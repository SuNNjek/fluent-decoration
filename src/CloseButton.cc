/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
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
#include "CloseButton.h"
#include "Decoration.h"

// KF
#include <KDecoration2/DecoratedClient>
#include <KColorUtils>

// Qt
#include <QPainter>

namespace Fluent
{
    CloseButton::CloseButton(Decoration *decoration, QObject *parent)
            : FluentDecorationButton(KDecoration2::DecorationButtonType::Close, decoration, parent)
    {
        auto *decoratedClient = decoration->client().toStrongRef().data();
        connect(decoratedClient, &KDecoration2::DecoratedClient::closeableChanged,
                this, &CloseButton::setVisible);

        setVisible(decoratedClient->isCloseable());
    }

    void CloseButton::paint(QPainter *painter, const QRect &repaintRegion)
    {
        Q_UNUSED(repaintRegion)

        const QRectF buttonRect = geometry();
        QRectF crossRect = QRectF(0, 0, 10, 10);
        crossRect.moveCenter(buttonRect.center().toPoint());

        painter->save();

        painter->setRenderHints(QPainter::Antialiasing, false);

        // Background.
        painter->setPen(Qt::NoPen);
        painter->setBrush(backgroundColor());
        painter->drawRect(buttonRect);

        // Foreground.
        painter->setPen(foregroundColor());
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(crossRect.topLeft(), crossRect.bottomRight());
        painter->drawLine(crossRect.topRight(), crossRect.bottomLeft());

        painter->restore();
    }

    QColor CloseButton::backgroundColor() const
    {
        const auto *deco = qobject_cast<Decoration *>(decoration());
        if (!deco) {
            return {};
        }

        if (isPressed()) {
            auto *decoratedClient = deco->client().toStrongRef().data();
            const auto color = decoratedClient->color(
                    KDecoration2::ColorGroup::Warning,
                    KDecoration2::ColorRole::Foreground
            );

            return KColorUtils::mix(color, deco->titleBarBackgroundColor(), 0.3);
        }

        if (isHovered()) {
            auto *decoratedClient = deco->client().toStrongRef().data();
            return decoratedClient->color(
                    KDecoration2::ColorGroup::Warning,
                    KDecoration2::ColorRole::Foreground
            );
        }

        return Qt::transparent;
    }
}
