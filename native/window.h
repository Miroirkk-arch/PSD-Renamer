#pragma once
#include "core.h"
#include "i18n.h"
#include <QtConcurrent>
class Window;
class LayerModel:public QAbstractItemModel {
public:
    Document* doc=nullptr;
    using QAbstractItemModel::QAbstractItemModel;
    QModelIndex index(int row,int col,const QModelIndex& parent={})const override;
    QModelIndex parent(const QModelIndex&)const override;
    int rowCount(const QModelIndex& parent={})const override;
    int columnCount(const QModelIndex& ={})const override{return 1;}
    QVariant data(const QModelIndex&,int)const override;
    Qt::ItemFlags flags(const QModelIndex&)const override;
    bool setData(const QModelIndex&,const QVariant&,int)override;
    QModelIndex forNode(Layer*)const;
    void reset(){beginResetModel();endResetModel();}
    std::function<void()> renamed;
};
class Canvas:public QWidget {
public:
    Window* owner; double zoom=1; QPointF pan; bool fitted=true,panning=false; QPoint last;
    QImage frame; QRectF frameRegion; QTimer delay; QFutureWatcher<QImage> watcher; bool pending=false; quint64 generation=0;
    QPushButton *minus,*plus,*fitButton; QLabel* percent;
    Canvas(Window*);
    QRectF region()const; void request();void fit();void changeZoom(double,QPointF={});
    void paintEvent(QPaintEvent*)override;void resizeEvent(QResizeEvent*)override;
    void wheelEvent(QWheelEvent*)override;void mousePressEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;void mouseReleaseEvent(QMouseEvent*)override;
};
class LayerView:public QTreeView {
public:
    Window* owner; QPoint pressed;Layer* dragNode=nullptr;Layer* dropNode=nullptr;int dropPlace=0;QString deferredSelection;
    LayerView(Window*);
    void mousePressEvent(QMouseEvent*)override;void mouseReleaseEvent(QMouseEvent*)override;void mouseDoubleClickEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;
    void dragEnterEvent(QDragEnterEvent*)override;void dragMoveEvent(QDragMoveEvent*)override;void dropEvent(QDropEvent*)override;void keyPressEvent(QKeyEvent*)override;
};
class LayerDelegate:public QStyledItemDelegate {
public:
    Window* owner;using QStyledItemDelegate::QStyledItemDelegate;
    LayerDelegate(Window*);
    void paint(QPainter*,const QStyleOptionViewItem&,const QModelIndex&)const override;
    QSize sizeHint(const QStyleOptionViewItem&,const QModelIndex&)const override;
    void updateEditorGeometry(QWidget*,const QStyleOptionViewItem&,const QModelIndex&)const override;
};
class Window:public QWidget {
public:
    Document doc; QSettings settings; std::unique_ptr<QTemporaryDir> workspace; QJsonArray baseline;
    LayerModel* model;LayerView* tree;Canvas* canvas;QSplitter* splitter;
    QLabel *fileLabel,*countLabel;QStackedWidget* pages; QPushButton *openButton,*recentButton,*saveButton,*saveAsButton,*numberButton,*createLayerButton,*createButton,*deleteButton,*nightButton,*bulkVisible,*bulkExpand,*searchButton,*thumbnailButton,*canvasButton;
    QAbstractButton *dictSwitch,*directionSwitch;
    QCache<QString,QImage> thumbs{4096};QSet<QString> thumbPending;QTimer thumbTimer;QQueue<QString> thumbQueue;
    bool night=false,busy=false,thumbnailsEnabled=true,canvasEnabled=true;QList<int> savedSplitSizes;Layer* active=nullptr;QString selectionAnchor;QHash<QString,QString> numbering;QHash<QString,bool> numberManual;QStringList numberingSelection;
    QString dictionaryMatchMode="contains";QPointer<QDialog> layerSearchDialog;QLineEdit* layerSearchInput=nullptr;QLineEdit* layerFilterInput=nullptr;QSet<QString> filterExpanded;bool filteringLayers=false;QTreeWidget* layerSearchResults=nullptr;QLabel* layerSearchCount=nullptr;QByteArray layerSearchSnapshot;QTimer layerSearchDelay;
    QJsonObject dictionary;QJsonArray dictionaryProfiles;QString dictionaryProfile;QProcess* codec=nullptr;QTimer codecDeadline;QElapsedTimer openClock;quint64 revision=0;
    QString savePath,lastError;QStringList recent;
    Window();~Window();
    QColor bg()const{return QColor(night?"#252525":"#bfbfbf");} QColor panel()const{return QColor(night?"#383838":"#dedede");}
    QColor text()const{return QColor(night?"#e2e2e2":"#262626");} QColor border()const{return QColor(night?"#242424":"#a4a4a4");}
    QColor muted()const{return QColor(night?"#b0b0b0":"#646464");}
    void theme();void retranslate();QComboBox* languageChoice=nullptr;void refresh(bool structure=false,bool pixels=false);void load(QString path);void startCodec(QString op,QString output,std::function<void(bool,QString)> done);
    void setBusy(bool);void select(Layer*,bool);void selectRow(Layer*,Qt::KeyboardModifiers);QVector<Layer*> visibleLayers()const;void visibility(Layer*,bool);void expandAll(bool);void number();void createLayer();void createGroup();void deleteSelection();void move(Layer*,Layer*,int);
    QString proposedName(Layer*,const QJsonObject&,QString,bool)const;bool previewDictionary(const QJsonObject&,QString,bool confirm=false);void findLayers();void filterLayers();void updateLayerSearch();void locateLayer(QString);
    void setThumbnailsEnabled(bool);void setCanvasEnabled(bool);void checkUpdates();void naming();void directionSettings();void numberingSettings();void dictionarySettings();void preview(Layer*);void save(bool saveAs=false);void writeSave(QString);
    QJsonArray changes()const;QImage* thumb(Layer*);QStringList selectedLeaves()const;bool contains(Layer*,Layer*)const;
    void closeEvent(QCloseEvent*)override;
    void resizeEvent(QResizeEvent*)override;
    bool nativeEvent(const QByteArray&,void*,qintptr*)override;
};
int depthOf(const Layer*);int nameLeft(const Layer*,bool thumbnails=true);
void drawEye(QPainter&,QRect,bool,QColor);void drawFolder(QPainter&,QRect,bool,QColor);
QString dictionaryFile();
QJsonObject englishDictionary();
void initializeEnglishDictionary(Window&);

QString dictionaryProfilesFile();
QString dictionaryConflict(const QJsonObject&);
QString dictionaryName(QString,const QJsonObject&,QString);
QJsonObject dictionaryExport(QString name,const QJsonObject&,QString mode="contains");
bool dictionaryImport(const QJsonObject&,QString&,QJsonObject&,QString&,QString* mode=nullptr);

struct ReleaseUpdate {QString version;QUrl download;bool newer=false;};
bool parseReleaseUpdate(const QJsonObject&,QString,QString,ReleaseUpdate&,QString&);
QIcon glyph(QString,QColor,bool state=false);
