#include "i18n.h"
static QString activeUiLanguage="zh";
static QHash<QString,QJsonObject> catalogs;
static const QJsonObject& catalog(QString code){if(!catalogs.contains(code)){QFile file(":/locales/"+code+".json");if(file.open(QIODevice::ReadOnly))catalogs.insert(code,QJsonDocument::fromJson(file.readAll()).object());else catalogs.insert(code,{});}return catalogs[code];}
QString ui(const char* source){QString key=QString::fromUtf8(source);return activeUiLanguage=="zh"?key:catalog(activeUiLanguage).value(key).toString(key);}
QString uiLanguage(){return activeUiLanguage;}
QString uiSource(const QString& translated){if(activeUiLanguage=="zh")return translated;const auto& values=catalog(activeUiLanguage);for(auto it=values.begin();it!=values.end();++it)if(it.value().toString()==translated)return it.key();return translated;}
class StandardUiTranslator:public QTranslator {
public:
    bool isEmpty()const override{return false;}
    QString translate(const char*,const char* source,const char*,int)const override {
        QString key=QString::fromUtf8(source);key.remove('&');static const QHash<QString,const char*> buttons={{"Undo","撤销"},{"Redo","重做"},{"Cut","剪切"},{"Paste","粘贴"},{"Select All","全选"},{"Yes","是"},{"No","否"},{"OK","确定"},{"Cancel","取消"},{"Save","保存"},{"Close","关闭"},{"Open","打开"},{"Apply","确认应用"},{"Delete","删除"},{"Rename","重命名"}};
        if(buttons.contains(key))return ui(buttons[key]);if(key=="Copy")return activeUiLanguage=="ja"?QString::fromUtf8("コピー"):activeUiLanguage=="en"?QString("Copy"):QString::fromUtf8("复制");
        static const QHash<QString,QString> japanese={{"Look in:","場所："},{"File name:","ファイル名："},{"Files of type:","ファイルの種類："},{"Save in:","保存先："},{"Name","名前"},{"Size","サイズ"},{"Type","種類"},{"Date Modified","更新日時"},{"Back","戻る"},{"Parent Directory","上の階層へ"},{"Create New Folder","新しいフォルダーを作成"},{"List View","一覧表示"},{"Detail View","詳細表示"},{"Show hidden files","隠しファイルを表示"},{"All Files (*)","すべてのファイル (*)"}};
        static const QHash<QString,QString> chinese={{"Look in:","位置："},{"File name:","文件名："},{"Files of type:","文件类型："},{"Save in:","保存到："},{"Name","名称"},{"Size","大小"},{"Type","类型"},{"Date Modified","修改时间"},{"Back","返回"},{"Parent Directory","上一级"},{"Create New Folder","新建文件夹"},{"List View","列表视图"},{"Detail View","详细视图"},{"Show hidden files","显示隐藏文件"},{"All Files (*)","所有文件 (*)"}};
        if(activeUiLanguage=="ja")return japanese.value(key);if(activeUiLanguage=="zh")return chinese.value(key);return {};
    }
};
void setUiLanguage(QString code){if(code!="en"&&code!="ja")code="zh";activeUiLanguage=code;static StandardUiTranslator* translator=nullptr;if(!translator){translator=new StandardUiTranslator;qApp->installTranslator(translator);}QLocale::setDefault(QLocale(code=="en"?"en_US":code=="ja"?"ja_JP":"zh_CN"));}
void bindUiText(QAbstractButton* button){button->setProperty("uiTextSource",uiSource(button->text()));}
void fitUiButton(QAbstractButton* button){
    button->ensurePolished();
    if(button->parentWidget()&&button->parentWidget()->objectName()=="toolbar")button->setFixedHeight(32);
    if(auto push=qobject_cast<QPushButton*>(button)){
        push->setMinimumWidth(0);push->setMaximumWidth(QWIDGETSIZE_MAX);
        push->setFixedWidth(qMax(push->height()==24?56:60,push->sizeHint().width()));
    }else button->setFixedWidth(26+QFontMetrics(button->font()).horizontalAdvance(button->text()));
    button->updateGeometry();
}
void retranslateUiButtons(QWidget* root){for(auto button:root->findChildren<QAbstractButton*>()){auto value=button->property("uiTextSource");if(!value.isValid())continue;QString key=value.toString();if(key.isEmpty())continue;button->setText(ui(key.toUtf8().constData()));fitUiButton(button);}}
