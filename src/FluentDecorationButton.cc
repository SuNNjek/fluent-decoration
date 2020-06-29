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
#include "FluentDecorationButton.h"
#include "Decoration.h"

// KDecoration
#include <KDecoration2/DecoratedClient>

// KF
#include <KColorUtils>

// Qt
#include <QPainter>

namespace Fluent
{
    FluentDecorationButton::FluentDecorationButton(KDecoration2::DecorationButtonType type, Decoration *decoration, QObject *parent)
            : DecorationButton(type, decoration, parent)
    {
        connect(this, &FluentDecorationButton::hoveredChanged, this,
                [this] {
                    update();
                });

        const int titleBarHeight = decoration->titleBarHeight();
        const QSize size(qRound(titleBarHeight * 1.33), titleBarHeight);
        setGeometry(QRect(QPoint(0, 0), size));
    }

    FluentDecorationButton::~FluentDecorationButton() { }

    QColor FluentDecorationButton::backgroundColor() const
    {
        const auto *deco = qobject_cast<Decoration *>(decoration());
        if (!deco) {
            return {};
        }

        if (isPressed()) {
            return KColorUtils::mix(
                    deco->titleBarBackgroundColor(),
                    deco->titleBarForegroundColor(),
                    0.3);
        }

        if (isHovered()) {
            return KColorUtils::mix(
                    deco->titleBarBackgroundColor(),
                    deco->titleBarForegroundColor(),
                    0.2);
        }

        return Qt::transparent;
    }

    QColor FluentDecorationButton::foregroundColor() const
    {
        const auto *deco = qobject_cast<Decoration *>(decoration());
        if (!deco) {
            return {};
        }

        return deco->titleBarForegroundColor();
    }
}