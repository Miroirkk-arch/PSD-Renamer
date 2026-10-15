#include "window.h"
#include <QtTest>
bool localeSelfTest(Window& w,QString directory,QString& error){
    auto snapshot=serializeLayers(w.doc.roots);auto profiles=w.dictionaryProfiles;auto rules=w.dictionary;auto size=w.size();bool night=w.night;
    bool thumbnails=w.thumbnailsEnabled,canvas=w.canvasEnabled;
    auto toolbar=w.openButton->parentWidget();
    for(QString code:{QString("zh"),QString("en"),QString("ja")})for(bool previews:{false,true}){
        w.languageChoice->setCurrentIndex(w.languageChoice->findData(code));
        w.setThumbnailsEnabled(previews);w.setCanvasEnabled(previews);
        if(w.thumbnailButton->isChecked()!=previews||w.canvasButton->isChecked()!=previews){error="preview button state mismatch";return false;}
        for(int width:{1400,1100}){
            w.resize(width,850);QTest::qWait(70);
            auto controls=toolbar->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly);
            for(int i=0;i<controls.size();++i){
                auto control=controls[i];
                if(control->height()!=32||!toolbar->rect().contains(control->geometry())){w.grab().save(directory+"/toolbar-failure.png");error=QString("toolbar height or bounds: %1 %2,%3 %4x%5 parent %6x%7").arg(control->objectName()).arg(control->x()).arg(control->y()).arg(control->width()).arg(control->height()).arg(toolbar->width()).arg(toolbar->height());return false;}
                if(auto push=qobject_cast<QPushButton*>(control)){if(!push->text().isEmpty()&&push->width()<push->sizeHint().width()){error="styled toolbar text clipped: "+push->text();return false;}}
                else if(auto toggle=qobject_cast<QAbstractButton*>(control)){if(toggle->width()<26+QFontMetrics(toggle->font()).horizontalAdvance(toggle->text())){error="toolbar toggle text clipped";return false;}}
                for(int j=0;j<i;++j){auto other=controls[j];if(control->geometry().intersects(other->geometry())){error="toolbar controls overlap";return false;}if(qAbs(control->y()-other->y())<32&&control->y()!=other->y()){error="toolbar row misaligned";return false;}}
            }
            w.grab().save(directory+"/toolbar-"+code+"-"+QString::number(width)+(previews?"-shown.png":"-hidden.png"));
        }
    }
    w.setThumbnailsEnabled(thumbnails);w.setCanvasEnabled(canvas);
    for(QString code:{QString("en"),QString("ja")}){
        w.languageChoice->setCurrentIndex(w.languageChoice->findData(code));QTest::qWait(70);
        if(uiLanguage()!=code||w.settings.value("ui-language").toString()!=code||w.openButton->text()!=(code=="en"?QString("Open PSD"):QString::fromUtf8("PSD を開く"))){error="language switching or preference failed";return false;}
        if(QCoreApplication::translate("QPlatformTheme","Cancel")!=(code=="en"?QString("Cancel"):QString::fromUtf8("キャンセル"))){error="standard dialog translation failed";return false;}
        if(QCoreApplication::translate("QWidgetTextControl","&Copy")!=(code=="en"?QString("Copy"):QString::fromUtf8("コピー"))){error="text context menu translation failed";return false;}auto toolbar=w.openButton->parentWidget();for(auto width:{1400,1100}){w.resize(width,850);QTest::qWait(60);auto controls=toolbar->findChildren<QAbstractButton*>(QString(),Qt::FindDirectChildrenOnly);for(int i=0;i<controls.size();++i){auto button=controls[i];if(!toolbar->rect().contains(button->geometry())){error="toolbar control clipped: "+button->text();return false;}if(!button->text().isEmpty()&&QFontMetrics(button->font()).horizontalAdvance(button->text())>button->width()-10){error="toolbar text clipped: "+button->text();return false;}for(int j=0;j<i;++j)if(button->geometry().intersects(controls[j]->geometry())){error="toolbar controls overlap";return false;}}w.grab().save(directory+"/"+code+"-"+QString::number(width)+".png");}
        w.resize(size);w.night=true;w.theme();QTest::qWait(50);w.grab().save(directory+"/"+code+"-dark.png");w.night=false;w.theme();QTest::qWait(30);w.grab().save(directory+"/"+code+"-light.png");
        bool numbering=false;QTimer::singleShot(100,&w,[&]{if(auto d=qobject_cast<QDialog*>(QApplication::activeModalWidget())){auto style=d->findChild<QComboBox*>("numberingStyle");numbering=d->windowTitle()==ui("编号设置")&&style&&style->itemText(1)==ui("字母 A、B、C");d->grab().save(directory+"/"+code+"-numbering.png");d->reject();}});w.numberingSettings();if(!numbering){error="numbering dialog translation failed";return false;}
        bool direction=false;QTimer::singleShot(100,&w,[&]{if(auto d=qobject_cast<QDialog*>(QApplication::activeModalWidget())){direction=d->windowTitle()==ui("方向后缀设置");bool left=false;for(auto label:d->findChildren<QLabel*>())left|=label->text()==ui("屏幕左侧");direction&=left;d->grab().save(directory+"/"+code+"-direction.png");d->reject();}});w.directionSettings();if(!direction){error="direction dialog translation failed";return false;}
        bool dictionary=false;QTimer::singleShot(100,&w,[&]{if(auto d=qobject_cast<QDialog*>(QApplication::activeModalWidget())){auto table=d->findChild<QTableWidget*>("dictionaryTable");dictionary=d->windowTitle()==ui("词库")&&table&&table->horizontalHeaderItem(0)->text()==ui("标准名称");d->grab().save(directory+"/"+code+"-dictionary.png");d->reject();}});w.dictionarySettings();if(!dictionary){error="dictionary translation failed";return false;}
        bool save=false;QTimer::singleShot(100,&w,[&]{if(auto d=qobject_cast<QDialog*>(QApplication::activeModalWidget())){auto table=d->findChild<QTableWidget*>("changesTable");save=d->windowTitle()==ui("所有改动")&&table&&table->horizontalHeaderItem(0)->text()==ui("改动");d->grab().save(directory+"/"+code+"-save.png");d->reject();}});w.save(false);if(!save){error="save review translation failed";return false;}
        w.findLayers();QTest::qWait(40);if(w.layerSearchDialog->windowTitle()!=ui("图层列表")||w.layerSearchResults->headerItem()->text(0)!=ui("名称")){error="layer list translation failed";return false;}w.layerSearchDialog->findChild<QLineEdit*>("replaceBefore")->setText("pending");w.layerSearchDialog->findChild<QLineEdit*>("replaceAfter")->setText("replacement");w.layerSearchDialog->grab().save(directory+"/"+code+"-layers.png");
        w.languageChoice->setCurrentIndex(0);QTest::qWait(40);if(!w.layerSearchDialog||w.layerSearchDialog->findChild<QLineEdit*>("replaceBefore")->text()!="pending"||w.layerSearchDialog->findChild<QLineEdit*>("replaceAfter")->text()!="replacement"){error="switching language lost pending replacement";return false;}w.layerSearchDialog->reject();
        if(serializeLayers(w.doc.roots)!=snapshot||w.dictionaryProfiles!=profiles||w.dictionary!=rules){error="language switching changed document or user dictionary";return false;}
    }
    w.languageChoice->setCurrentIndex(0);w.night=night;w.resize(size);w.theme();return true;
}
