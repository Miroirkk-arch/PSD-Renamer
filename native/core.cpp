#include "core.h"
Layers loadLayers(const QJsonArray& array, Layer* parent) {
    Layers nodes;
    for (const auto& value: array) {
        auto j=value.toObject(); auto n=std::make_shared<Layer>();
        n->id=j["id"].toString(); n->name=n->originalName=j["name"].toString(); n->parent=parent;
        n->group=j["group"].toBool(); n->hidden=j["hidden"].toBool(); n->opened=j["opened"].toBool(true);
        n->bounds=QRect(j["left"].toInt(),j["top"].toInt(),j["right"].toInt()-j["left"].toInt(),j["bottom"].toInt()-j["top"].toInt());
        n->opacity=j["opacity"].toDouble(1); n->fillOpacity=j["fillOpacity"].toDouble(1); n->blend=j["blend"].toString(); n->clipping=j["clipping"].toBool();
        auto image=j["image"].toObject(); n->pixels=image["file"].toString(); n->size=QSize(image["width"].toInt(),image["height"].toInt());
        auto crop=image["crop"].toArray(); if(crop.size()==4)n->crop=QRect(crop[0].toInt(),crop[1].toInt(),crop[2].toInt(),crop[3].toInt());
        n->mask=j["mask"].toObject(); n->children=loadLayers(j["children"].toArray(),n.get()); nodes.push_back(n);
    }
    return nodes;
}
QJsonArray serializeLayers(const Layers& nodes) {
    QJsonArray out; for(auto& n:nodes)out.append(QJsonObject{{"id",n->id},{"name",n->name},{"group",n->group},{"hidden",n->hidden},{"opened",n->opened},{"children",serializeLayers(n->children)}}); return out;
}
QVector<Layer*> flatten(const Layers& nodes) { QVector<Layer*> out; for(auto& n:nodes){out.append(n.get());out+=flatten(n->children);}return out; }
struct Mapping { QFile file; uchar* data=nullptr; Mapping(QString path):file(path){} ~Mapping(){if(data)file.unmap(data);} };
QImage mappedPixels(const QString& path,QSize size) {
    if(size.isEmpty())return {}; auto mapping=new Mapping(path);
    if(!mapping->file.open(QIODevice::ReadOnly)||mapping->file.size()!=qint64(size.width())*size.height()*4){delete mapping;return {};}
    mapping->data=mapping->file.map(0,mapping->file.size());if(!mapping->data){delete mapping;return {};}
    return QImage(mapping->data,size.width(),size.height(),size.width()*4,QImage::Format_RGBA8888,[](void* p){delete static_cast<Mapping*>(p);},mapping);
}
QPainter::CompositionMode blendMode(const QString& s) {
    static const QHash<QString,QPainter::CompositionMode> modes={
        {"multiply",QPainter::CompositionMode_Multiply},{"screen",QPainter::CompositionMode_Screen},{"overlay",QPainter::CompositionMode_Overlay},
        {"darken",QPainter::CompositionMode_Darken},{"lighten",QPainter::CompositionMode_Lighten},{"color dodge",QPainter::CompositionMode_ColorDodge},
        {"color burn",QPainter::CompositionMode_ColorBurn},{"hard light",QPainter::CompositionMode_HardLight},{"soft light",QPainter::CompositionMode_SoftLight},
        {"difference",QPainter::CompositionMode_Difference},{"exclusion",QPainter::CompositionMode_Exclusion},{"linear dodge",QPainter::CompositionMode_Plus}};
    return modes.value(s,QPainter::CompositionMode_SourceOver);
}
static QImage blank(QSize size){QImage image(size,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);return image;}
struct RGB {double r,g,b;};
static double lum(RGB c){return .3*c.r+.59*c.g+.11*c.b;}
static double sat(RGB c){return qMax(c.r,qMax(c.g,c.b))-qMin(c.r,qMin(c.g,c.b));}
static RGB setLum(RGB c,double l){double delta=l-lum(c);c.r+=delta;c.g+=delta;c.b+=delta;double n=qMin(c.r,qMin(c.g,c.b)),x=qMax(c.r,qMax(c.g,c.b));l=lum(c);if(n<0){c.r=l+(c.r-l)*l/(l-n);c.g=l+(c.g-l)*l/(l-n);c.b=l+(c.b-l)*l/(l-n);}if(x>1){c.r=l+(c.r-l)*(1-l)/(x-l);c.g=l+(c.g-l)*(1-l)/(x-l);c.b=l+(c.b-l)*(1-l)/(x-l);}return c;}
static RGB setSat(RGB c,double s){double* values[]={&c.r,&c.g,&c.b};std::sort(values,values+3,[](auto a,auto b){return *a<*b;});if(*values[2]>*values[0]){*values[1]=(*values[1]-*values[0])*s/(*values[2]-*values[0]);*values[2]=s;}else{*values[1]=*values[2]=0;}*values[0]=0;return c;}
static bool specialBlend(QString mode){return mode=="hue"||mode=="saturation"||mode=="color"||mode=="luminosity";}
static void compose(QImage& dest,const QImage& source,QString mode,double opacity){if(!specialBlend(mode)){QPainter p(&dest);p.setOpacity(opacity);p.setCompositionMode(blendMode(mode));p.drawImage(0,0,source);return;}
    for(int y=0;y<dest.height();++y){auto d=reinterpret_cast<QRgb*>(dest.scanLine(y));auto s=reinterpret_cast<const QRgb*>(source.constScanLine(y));for(int x=0;x<dest.width();++x){double sa=qAlpha(s[x])/255.*opacity,da=qAlpha(d[x])/255.;if(sa<=0)continue;RGB sc{qAlpha(s[x])?double(qRed(s[x]))/qAlpha(s[x]):0,qAlpha(s[x])?double(qGreen(s[x]))/qAlpha(s[x]):0,qAlpha(s[x])?double(qBlue(s[x]))/qAlpha(s[x]):0};RGB dc{da?qRed(d[x])/255./da:0,da?qGreen(d[x])/255./da:0,da?qBlue(d[x])/255./da:0};RGB b=mode=="hue"?setLum(setSat(sc,sat(dc)),lum(dc)):mode=="saturation"?setLum(setSat(dc,sat(sc)),lum(dc)):mode=="color"?setLum(sc,lum(dc)):setLum(dc,lum(sc));double alpha=sa+da*(1-sa);auto channel=[&](double a,double c,double mix){return qBound(0,qRound((sa*(1-da)*a+sa*da*mix+(1-sa)*da*c)*255),255);};d[x]=qRgba(channel(sc.r,dc.r,b.r),channel(sc.g,dc.g,b.g),channel(sc.b,dc.b,b.b),qRound(alpha*255));}}
}
struct Renderer {
    const Document& d; QRectF region; QSize output;
    void applyMask(QImage& image,const Layer& n) {
        auto m=n.mask;if(m.isEmpty()||m["disabled"].toBool())return;
        auto mi=m["image"].toObject();auto mask=mappedPixels(d.workspace+"/"+mi["file"].toString(),{mi["width"].toInt(),mi["height"].toInt()});
        if(mask.isNull())return; double left=m["left"].toDouble(),top=m["top"].toDouble();
        if(m["positionRelativeToLayer"].toBool()){left+=n.bounds.left();top+=n.bounds.top();}
        for(int y=0;y<output.height();++y){auto line=reinterpret_cast<QRgb*>(image.scanLine(y));
            int my=qFloor(region.top()+(y+.5)*region.height()/output.height()-top);
            for(int x=0;x<output.width();++x){int mx=qFloor(region.left()+(x+.5)*region.width()/output.width()-left);
                int value=m["defaultColor"].toInt(255);if(mx>=0&&my>=0&&mx<mask.width()&&my<mask.height())value=mask.constScanLine(my)[mx*4];
                QRgb c=line[x];line[x]=qRgba(qRed(c)*value/255,qGreen(c)*value/255,qBlue(c)*value/255,qAlpha(c)*value/255);
            }
        }
    }
    QImage frame(const Layer& n) {
        auto image=blank(output);if(n.hidden)return image;
        if(n.group)draw(n.children,image);
        else {auto src=mappedPixels(d.workspace+"/"+n.pixels,n.size);if(!src.isNull()){
            QPainter p(&image);p.setRenderHint(QPainter::SmoothPixmapTransform,region.width()/output.width()>1);
            p.setOpacity(n.fillOpacity);p.scale(output.width()/region.width(),output.height()/region.height());p.translate(-region.topLeft());
            p.drawImage(n.bounds.topLeft(),src);
        }}
        applyMask(image,n);return image;
    }
    void draw(const Layers& nodes,QImage& dest) {
        for(int i=int(nodes.size())-1;i>=0;--i){const auto& n=*nodes[i];
            if(n.clipping)continue;
            QVector<Layer*> clipped;while(i>0&&nodes[i-1]->clipping)clipped.append(nodes[--i].get());
            if(n.hidden)continue;
            if(n.group&&clipped.isEmpty()&&n.blend=="pass through"&&n.opacity>=1&&n.mask.isEmpty()){draw(n.children,dest);continue;}
            if(!n.group&&clipped.isEmpty()&&n.mask.isEmpty()&&!specialBlend(n.blend)){
                auto source=mappedPixels(d.workspace+"/"+n.pixels,n.size);if(source.isNull())continue;
                QPainter p(&dest);p.setRenderHint(QPainter::SmoothPixmapTransform,region.width()/output.width()>1);p.setOpacity(n.opacity*n.fillOpacity);p.setCompositionMode(blendMode(n.blend));p.scale(output.width()/region.width(),output.height()/region.height());p.translate(-region.topLeft());p.drawImage(n.bounds.topLeft(),source);continue;
            }
            auto base=frame(n);auto bundle=base;
            for(auto c:clipped){if(c->hidden)continue;auto layer=frame(*c);{QPainter p(&layer);p.setCompositionMode(QPainter::CompositionMode_DestinationIn);p.drawImage(0,0,base);}compose(bundle,layer,c->blend,c->opacity);}
            compose(dest,bundle,n.blend,n.opacity);
        }
    }
};
QImage renderDocument(const Document& d,QRectF region,QSize output) {
    if(region.isEmpty()||output.isEmpty())return {};auto image=blank(output);Renderer{d,region,output}.draw(d.roots,image);return image;
}
QImage thumbnail(const Document& d,const Layer& n,int side) {
    if(n.crop.isEmpty())return {};auto src=mappedPixels(d.workspace+"/"+n.pixels,n.size);if(src.isNull())return {};
    QSize target=n.crop.size().scaled(side,side,Qt::KeepAspectRatio);QImage out(target,QImage::Format_ARGB32_Premultiplied);out.fill(Qt::transparent);
    QPainter p(&out);p.setRenderHint(QPainter::SmoothPixmapTransform);p.drawImage(QRect(QPoint(),target),src,n.crop);return out;
}
