#pragma once
#include <QtWidgets>
class ToolbarFlow:public QLayout {
    QList<QLayoutItem*> items;
    int arrange(QRect rect,bool measure)const {auto margins=contentsMargins();QRect area=rect.adjusted(margins.left(),margins.top(),-margins.right(),-margins.bottom());int x=area.left(),y=area.top(),height=0;for(auto item:items){auto size=item->sizeHint();if(x>area.left()&&x+size.width()>area.right()+1){x=area.left();y+=height+8;height=0;}if(!measure)item->setGeometry(QRect(QPoint(x,y),size));x+=size.width()+8;height=qMax(height,size.height());}return y+height-rect.top()+margins.bottom();}
public:
    explicit ToolbarFlow(QWidget* parent):QLayout(parent){}
    ~ToolbarFlow(){while(auto item=takeAt(0))delete item;}
    void addItem(QLayoutItem* item)override{items.append(item);}
    void addStretch(){addItem(new QSpacerItem(4,1,QSizePolicy::Fixed,QSizePolicy::Fixed));}
    int count()const override{return items.size();}
    QLayoutItem* itemAt(int index)const override{return items.value(index);}
    QLayoutItem* takeAt(int index)override{return index>=0&&index<items.size()?items.takeAt(index):nullptr;}
    Qt::Orientations expandingDirections()const override{return {};}
    bool hasHeightForWidth()const override{return true;}
    int heightForWidth(int width)const override{return arrange(QRect(0,0,width,0),true);}
    QSize minimumSize()const override{QSize result;for(auto item:items)result=result.expandedTo(item->minimumSize());auto m=contentsMargins();return result+QSize(m.left()+m.right(),m.top()+m.bottom());}
    QSize sizeHint()const override{return minimumSize();}
    void setGeometry(const QRect& rect)override{QLayout::setGeometry(rect);arrange(rect,false);}
};
