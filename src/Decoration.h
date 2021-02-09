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

#pragma once

// KDecoration
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButtonGroup>

// Qt
#include <QVariant>

namespace Fluent
{
    class FluentDecorationButton;
    class MenuButton;

    struct ShadowParams
    {
        ShadowParams() = default;

        ShadowParams(const QPoint &offset, int radius, qreal opacity)
                : offset(offset)
                , radius(radius)
                , opacity(opacity) {}

        QPoint offset;
        int radius = 0;
        qreal opacity = 0;
    };

    struct CompositeShadowParams
    {
        CompositeShadowParams() = default;

        CompositeShadowParams(
                const QPoint &offset,
                const ShadowParams &shadow1,
                const ShadowParams &shadow2)
                : offset(offset)
                , shadow1(shadow1)
                , shadow2(shadow2) {}

        QPoint offset;
        ShadowParams shadow1;
        ShadowParams shadow2;
    };

    class Decoration : public KDecoration2::Decoration
    {
    Q_OBJECT

    public:
        Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
        ~Decoration() override;

        void paint(QPainter *painter, const QRect &repaintRegion) override;

        int titleBarHeight() const;

        QColor titleBarBackgroundColor() const;
        QColor titleBarForegroundColor() const;

    public slots:
        void init() override;

    private:
        void updateBorders();
        void updateResizeBorders();
        void updateTitleBar();
        void updateButtonsGeometry();
        void updateButtonsGeometryDelayed();
        void updateShadow();

        void paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const;
        void paintTitleBarBackground(QPainter *painter, const QRect &repaintRegion) const;
        void paintCaption(QPainter *painter, const QRect &repaintRegion) const;
        void paintButtons(QPainter *painter, const QRect &repaintRegion) const;

        static QSharedPointer<KDecoration2::DecorationShadow> createShadow(const CompositeShadowParams shadowParams, const qreal strength);

        KDecoration2::DecorationButtonGroup *m_leftButtons;
        KDecoration2::DecorationButtonGroup *m_rightButtons;
    };
}
