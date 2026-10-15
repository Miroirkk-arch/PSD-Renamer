#pragma once
#include <QtWidgets>
#include <memory>
#include <vector>
struct Layer {
    QString id, name, originalName, blend;
    bool group=false, hidden=false, opened=true, checked=false, manual=false, clipping=false;
    QRect bounds, crop; QString pixels; QSize size;
    QJsonObject mask; double opacity=1, fillOpacity=1;
    Layer* parent=nullptr;
    std::vector<std::shared_ptr<Layer>> children;
};
using Layers=std::vector<std::shared_ptr<Layer>>;
struct Document {
    int width=0,height=0; QString path,workspace; Layers roots;
};
Layers loadLayers(const QJsonArray&, Layer* parent=nullptr);
QJsonArray serializeLayers(const Layers&);
QVector<Layer*> flatten(const Layers&);
QImage mappedPixels(const QString&, QSize);
QImage renderDocument(const Document&, QRectF region, QSize output);
QImage thumbnail(const Document&, const Layer&, int side=64);
QPainter::CompositionMode blendMode(const QString&);
