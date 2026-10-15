#pragma once
#include <QtWidgets>
#include <windows.h>
#include <dwmapi.h>
class CaptionTheme:public QObject {
public:using QObject::QObject;
    bool eventFilter(QObject* object,QEvent* event)override{
        if(event->type()==QEvent::Show){auto widget=qobject_cast<QWidget*>(object);if(widget&&widget->isWindow()&&!widget->windowFlags().testFlag(Qt::FramelessWindowHint)&&widget->windowType()!=Qt::Popup&&widget->windowType()!=Qt::ToolTip){BOOL dark=qApp->palette().color(QPalette::Window).lightness()<128;auto handle=reinterpret_cast<HWND>(widget->winId());if(FAILED(DwmSetWindowAttribute(handle,20,&dark,sizeof(dark))))DwmSetWindowAttribute(handle,19,&dark,sizeof(dark));}}
        return QObject::eventFilter(object,event);
    }
};
