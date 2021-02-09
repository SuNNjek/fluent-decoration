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
#include "Decoration.h"
#include "BoxShadowHelper.h"
#include "CloseButton.h"
#include "MaximizeButton.h"
#include "MinimizeButton.h"
#include "ContextHelpButton.h"
#include "MenuButton.h"

// KDecoration
#include <KDecoration2/DecoratedClient>
#include <KDecoration2/DecorationSettings>
#include <KDecoration2/DecorationShadow>

// Qt
#include <QPainter>
#include <QSharedPointer>
#include <QTimer>

namespace Fluent
{
    namespace
    {
        const CompositeShadowParams s_shadowParams = CompositeShadowParams(
                QPoint(0, 12),
                ShadowParams(QPoint(0, 0), 48, 0.8),
                ShadowParams(QPoint(0, -6), 24, 0.2));
    }

    static int s_decoCount = 0;
    static QColor s_shadowColor(0, 0, 0);
    static QSharedPointer<KDecoration2::DecorationShadow> s_cachedShadow;
    static QSharedPointer<KDecoration2::DecorationShadow> s_cachedShadowInactive;

    static qreal s_titleBarOpacityActive = 0.8;
    static qreal s_titleBarOpacityInactive = 0.8;

    Decoration::Decoration(QObject *parent, const QVariantList &args)
            : KDecoration2::Decoration(parent, args)
    {
        ++s_decoCount;
    }

    Decoration::~Decoration()
    {
        if (--s_decoCount == 0) {
            s_cachedShadow.clear();
        }
    }

    void Decoration::paint(QPainter *painter, const QRect &repaintRegion)
    {
        auto *decoratedClient = client().toStrongRef().data();

        if (!decoratedClient->isShaded()) {
            paintFrameBackground(painter, repaintRegion);
        }

        paintTitleBarBackground(painter, repaintRegion);
        paintCaption(painter, repaintRegion);
        paintButtons(painter, repaintRegion);
    }

    void Decoration::init()
    {
        auto *decoratedClient = client().toStrongRef().data();

        connect(decoratedClient, &KDecoration2::DecoratedClient::widthChanged,
                this, &Decoration::updateTitleBar);
        connect(decoratedClient, &KDecoration2::DecoratedClient::widthChanged,
                this, &Decoration::updateButtonsGeometry);
        connect(decoratedClient, &KDecoration2::DecoratedClient::maximizedChanged,
                this, &Decoration::updateButtonsGeometry);

        auto repaintTitleBar = [this] {
            update(titleBar());
        };

        auto onActiveChanged = [this] {
            update(titleBar());
            updateShadow();
        };

        connect(decoratedClient, &KDecoration2::DecoratedClient::captionChanged,
                this, repaintTitleBar);
        connect(decoratedClient, &KDecoration2::DecoratedClient::activeChanged,
                this, onActiveChanged);

        updateBorders();
        updateResizeBorders();
        updateTitleBar();

        auto s = settings();
        connect(s.data(), &KDecoration2::DecorationSettings::borderSizeChanged, this, &Decoration::updateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::fontChanged, this, &Decoration::updateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::updateBorders);
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::updateBorders);

        connect(s.data(), &KDecoration2::DecorationSettings::spacingChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(s.data(), &KDecoration2::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(s.data(), &KDecoration2::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::updateButtonsGeometryDelayed);
        connect(s.data(), &KDecoration2::DecorationSettings::reconfigured, this, &Decoration::updateButtonsGeometryDelayed);

        auto buttonCreator = [this] (KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
                -> KDecoration2::DecorationButton* {
            Q_UNUSED(decoration)

            switch (type) {
                case KDecoration2::DecorationButtonType::Close:
                    return new CloseButton(this, parent);

                case KDecoration2::DecorationButtonType::Maximize:
                    return new MaximizeButton(this, parent);

                case KDecoration2::DecorationButtonType::Minimize:
                    return new MinimizeButton(this, parent);

                case KDecoration2::DecorationButtonType::ContextHelp:
                    return new ContextHelpButton(this, parent);

                case KDecoration2::DecorationButtonType::Menu:
                    return new MenuButton(this, parent);

                default:
                    return nullptr;
            }
        };

        m_leftButtons = new KDecoration2::DecorationButtonGroup(
                KDecoration2::DecorationButtonGroup::Position::Left,
                this,
                buttonCreator);

        m_rightButtons = new KDecoration2::DecorationButtonGroup(
                KDecoration2::DecorationButtonGroup::Position::Right,
                this,
                buttonCreator);

        updateButtonsGeometry();

        // For some reason, the shadow should be installed the last. Otherwise,
        // the Window Decorations KCM crashes.
        updateShadow();
    }

    void Decoration::updateBorders()
    {
        QMargins borders;
        borders.setTop(titleBarHeight());
        setBorders(borders);
    }

    void Decoration::updateResizeBorders()
    {
        QMargins borders;

        const int extender = settings()->largeSpacing();
        borders.setLeft(extender);
        borders.setTop(extender);
        borders.setRight(extender);
        borders.setBottom(extender);

        setResizeOnlyBorders(borders);
    }

    void Decoration::updateTitleBar()
    {
        auto *decoratedClient = client().toStrongRef().data();
        setTitleBar(QRect(0, 0, decoratedClient->width(), titleBarHeight()));
    }

    void Decoration::updateButtonsGeometry()
    {
        if (!m_leftButtons->buttons().isEmpty()) {
            m_leftButtons->setPos(QPointF(0, 0));
            m_leftButtons->setSpacing(0);
        }

        if (!m_rightButtons->buttons().isEmpty()) {
            m_rightButtons->setPos(QPointF(size().width() - m_rightButtons->geometry().width(), 0));
            m_rightButtons->setSpacing(0);
        }

        update();
    }

    void Decoration::updateButtonsGeometryDelayed()
    {
        QTimer::singleShot(0, this, &Decoration::updateButtonsGeometry);
    }

    void Decoration::updateShadow()
    {
        const auto *decoratedClient = client().toStrongRef().data();
        auto isActive = decoratedClient->isActive();

        auto &shadow = isActive ? s_cachedShadow : s_cachedShadowInactive;
        if (shadow.isNull()) {
            shadow = createShadow(s_shadowParams, isActive ? 1.0 : 0.5);
        }

        setShadow(shadow);
    }

    int Decoration::titleBarHeight() const
    {
        const QFontMetrics fontMetrics(settings()->font());
        const int baseUnit = settings()->largeSpacing();
        return qRound(1.5 * baseUnit) + fontMetrics.height();
    }

    void Decoration::paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const
    {
        Q_UNUSED(repaintRegion)

        const auto *decoratedClient = client().toStrongRef().data();

        painter->save();

        painter->fillRect(rect(), Qt::transparent);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(decoratedClient->color(
                decoratedClient->isActive()
                ? KDecoration2::ColorGroup::Active
                : KDecoration2::ColorGroup::Inactive,
                KDecoration2::ColorRole::Frame));
        painter->setClipRect(0, borderTop(), size().width(), size().height() - borderTop(), Qt::IntersectClip);
        painter->drawRect(rect());

        painter->restore();
    }

    QColor Decoration::titleBarBackgroundColor() const
    {
        const auto *decoratedClient = client().toStrongRef().data();
        const auto group = decoratedClient->isActive()
                           ? KDecoration2::ColorGroup::Active
                           : KDecoration2::ColorGroup::Inactive;
        const qreal opacity = decoratedClient->isActive()
                              ? s_titleBarOpacityActive
                              : s_titleBarOpacityInactive;
        QColor color = decoratedClient->color(group, KDecoration2::ColorRole::TitleBar);
        color.setAlphaF(opacity);
        return color;
    }

    QColor Decoration::titleBarForegroundColor() const
    {
        const auto *decoratedClient = client().toStrongRef().data();
        const auto group = decoratedClient->isActive()
                           ? KDecoration2::ColorGroup::Active
                           : KDecoration2::ColorGroup::Inactive;
        return decoratedClient->color(group, KDecoration2::ColorRole::Foreground);
    }

    void Decoration::paintTitleBarBackground(QPainter *painter, const QRect &repaintRegion) const
    {
        Q_UNUSED(repaintRegion)

        const auto *decoratedClient = client().toStrongRef().data();

        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(titleBarBackgroundColor());
        painter->drawRect(QRect(0, 0, decoratedClient->width(), titleBarHeight()));
        painter->restore();
    }

    void Decoration::paintCaption(QPainter *painter, const QRect &repaintRegion) const
    {
        Q_UNUSED(repaintRegion)

        const auto *decoratedClient = client().toStrongRef().data();

        const int textWidth = settings()->fontMetrics().boundingRect(decoratedClient->caption()).width();
        const QRect textRect((size().width() - textWidth) / 2, 0, textWidth, titleBarHeight());

        const QRect titleBarRect(0, 0, size().width(), titleBarHeight());

        const QRect availableRect = titleBarRect.adjusted(
                m_leftButtons->geometry().width() + settings()->smallSpacing(), 0,
                -(m_rightButtons->geometry().width() + settings()->smallSpacing()), 0
        );

        const QString caption = painter->fontMetrics().elidedText(
                decoratedClient->caption(), Qt::ElideRight, availableRect.width());

        painter->save();
        painter->setFont(settings()->font());
        painter->setPen(titleBarForegroundColor());
        painter->drawText(availableRect, Qt::AlignLeft | Qt::AlignVCenter, caption);
        painter->restore();
    }

    void Decoration::paintButtons(QPainter *painter, const QRect &repaintRegion) const
    {
        m_leftButtons->paint(painter, repaintRegion);
        m_rightButtons->paint(painter, repaintRegion);
    }

    QSharedPointer<KDecoration2::DecorationShadow> Decoration::createShadow(const CompositeShadowParams shadowParams, const qreal strength)
    {
        auto withOpacity = [] (const QColor &color, qreal opacity) -> QColor {
            QColor c(color);
            c.setAlphaF(opacity);
            return c;
        };

        const int shadowSize = qMax(shadowParams.shadow1.radius, shadowParams.shadow2.radius);
        const QRect box(shadowSize, shadowSize, 2 * shadowSize + 1, 2 * shadowSize + 1);
        const QRect rect = box.adjusted(-shadowSize, -shadowSize, shadowSize, shadowSize);

        QImage shadow(rect.size(), QImage::Format_ARGB32_Premultiplied);
        shadow.fill(Qt::transparent);

        QPainter painter(&shadow);
        painter.setRenderHint(QPainter::Antialiasing);

        // Draw the "shape" shadow.
        BoxShadowHelper::boxShadow(
                &painter,
                box,
                shadowParams.shadow1.offset,
                shadowParams.shadow1.radius,
                withOpacity(s_shadowColor, shadowParams.shadow1.opacity * strength));

        // Draw the "contrast" shadow.
        BoxShadowHelper::boxShadow(
                &painter,
                box,
                shadowParams.shadow2.offset,
                shadowParams.shadow2.radius,
                withOpacity(s_shadowColor, shadowParams.shadow2.opacity * strength));

        // Mask out inner rect.
        const QMargins padding = QMargins(
                shadowSize - shadowParams.offset.x(),
                shadowSize - shadowParams.offset.y(),
                shadowSize + shadowParams.offset.x(),
                shadowSize + shadowParams.offset.y());
        const QRect innerRect = rect - padding;

        // Draw outline.
        QPen outLine (QColor(100,100,100));
        outLine.setWidth(2);
        outLine.setJoinStyle(Qt::MiterJoin);
        painter.setPen(outLine);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(innerRect);

        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        painter.drawRect(innerRect);

        painter.end();

        auto decorationShadow = QSharedPointer<KDecoration2::DecorationShadow>::create();
        decorationShadow->setPadding(padding);
        decorationShadow->setInnerShadowRect(QRect(shadow.rect().center(), QSize(1, 1)));
        decorationShadow->setShadow(shadow);

        return decorationShadow;
    }
}
