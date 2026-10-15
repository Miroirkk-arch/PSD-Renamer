#include "window.h"

class SearchNameDelegate:public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem& option,const QModelIndex& index)const override {auto size=QStyledItemDelegate::sizeHint(option,index);size.setHeight(qMax(28,option.fontMetrics.height()+10));return size;}
    QWidget* createEditor(QWidget* parent,const QStyleOptionViewItem& option,const QModelIndex& index)const override {
        if(index.column()!=0)return nullptr;
        auto editor=new QLineEdit(parent);editor->setObjectName("layerNameEditor");editor->setFont(option.font);editor->setFrame(false);return editor;
    }
    void updateEditorGeometry(QWidget* editor,const QStyleOptionViewItem& option,const QModelIndex& index)const override {
        QStyledItemDelegate::updateEditorGeometry(editor,option,index);auto rect=editor->geometry();rect.setTop(option.rect.top()+2);rect.setBottom(option.rect.bottom()-2);editor->setGeometry(rect);
    }
};
class SearchTreeWidget:public QTreeWidget {
    Window* owner;
public:
    SearchTreeWidget(Window* window,QWidget* parent):QTreeWidget(parent),owner(window){}
protected:
    void drawRow(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index)const override {
        QTreeWidget::drawRow(painter,option,index);painter->save();painter->setPen(owner->night?QColor("#505050"):QColor("#b8b8b8"));
        painter->drawLine(0,option.rect.bottom(),viewport()->width(),option.rect.bottom());painter->restore();
    }
};
static QList<QTreeWidgetItem*> searchItems(QTreeWidget* tree){QList<QTreeWidgetItem*> out;for(QTreeWidgetItemIterator it(tree);*it;++it)out.append(*it);return out;}

void Window::findLayers(){
    if(busy||doc.path.isEmpty()||QApplication::activeModalWidget())return;
    if(!layerSearchDialog){
        layerSearchDialog=new QDialog(this,Qt::Tool);layerSearchDialog->setWindowTitle(QStringLiteral("图层列表"));layerSearchDialog->setObjectName("layerSearchDialog");layerSearchDialog->resize(500,580);
        auto layout=new QVBoxLayout(layerSearchDialog);auto top=new QHBoxLayout;
        layerSearchInput=new QLineEdit(layerSearchDialog);layerSearchInput->setObjectName("layerSearchInput");layerSearchInput->setPlaceholderText(QStringLiteral("图层或文件夹名称"));layerSearchInput->setClearButtonEnabled(true);
        layerSearchCount=new QLabel(layerSearchDialog);layerSearchCount->setObjectName("layerSearchCount");top->addWidget(layerSearchInput,1);top->addWidget(layerSearchCount);layout->addLayout(top);
        layerSearchResults=new SearchTreeWidget(this,layerSearchDialog);layerSearchResults->setObjectName("layerSearchResults");layerSearchResults->setColumnCount(1);layerSearchResults->setHeaderLabels({QStringLiteral("名称")});layerSearchResults->setSelectionMode(QAbstractItemView::ExtendedSelection);layerSearchResults->setSelectionBehavior(QAbstractItemView::SelectRows);layerSearchResults->setEditTriggers(QAbstractItemView::EditKeyPressed);layerSearchResults->setItemDelegate(new SearchNameDelegate(layerSearchResults));layerSearchResults->setUniformRowHeights(true);layerSearchResults->setIndentation(20);layerSearchResults->header()->setSectionResizeMode(QHeaderView::Stretch);layout->addWidget(layerSearchResults);
        auto selectionBar=new QHBoxLayout;selectionBar->addStretch();auto all=new QPushButton(QStringLiteral("全选"),layerSearchDialog);all->setObjectName("searchSelectAll");selectionBar->addWidget(all);layout->addLayout(selectionBar);
        auto form=new QFormLayout;auto before=new QLineEdit(layerSearchDialog),after=new QLineEdit(layerSearchDialog);before->setObjectName("replaceBefore");after->setObjectName("replaceAfter");form->addRow(QStringLiteral("替换前"),before);form->addRow(QStringLiteral("替换后"),after);layout->addLayout(form);
        auto bottom=new QHBoxLayout;auto sensitive=new QCheckBox(QStringLiteral("区分大小写"),layerSearchDialog);sensitive->setObjectName("replaceCaseSensitive");bottom->addWidget(sensitive);bottom->addStretch();auto replace=new QPushButton(QStringLiteral("替换"),layerSearchDialog);replace->setObjectName("replaceNames");replace->setEnabled(false);bottom->addWidget(replace);layout->addLayout(bottom);auto status=new QLabel(layerSearchDialog);status->setObjectName("replaceStatus");layout->addWidget(status);
        connect(layerSearchInput,&QLineEdit::textChanged,this,[this]{layerSearchDelay.start();});
        auto syncSelection=[this,replace,before,all]{bool matched=false,allSelected=true;for(auto item:searchItems(layerSearchResults))if(item->data(0,Qt::UserRole+1).toBool()){matched=true;allSelected&=item->isSelected();}all->setText(matched&&allSelected?QStringLiteral("取消全选"):QStringLiteral("全选"));auto selected=layerSearchResults->selectedItems();replace->setEnabled(!selected.isEmpty()&&!before->text().isEmpty());QSet<QString> ids;for(auto item:selected)ids.insert(item->data(0,Qt::UserRole).toString());for(auto n:flatten(doc.roots))n->checked=ids.contains(n->id);active=nullptr;refresh();};
        connect(layerSearchResults,&QTreeWidget::itemSelectionChanged,this,syncSelection);connect(before,&QLineEdit::textChanged,this,[replace,before,this]{replace->setEnabled(!before->text().isEmpty()&&!layerSearchResults->selectedItems().isEmpty());});
        auto selectAll=[this](bool toggle){bool matched=false,allSelected=true;auto items=searchItems(layerSearchResults);for(auto item:items)if(item->data(0,Qt::UserRole+1).toBool()){matched=true;allSelected&=item->isSelected();}bool clear=toggle&&matched&&allSelected;QSignalBlocker blocker(layerSearchResults);for(auto item:items)item->setSelected(!clear&&item->data(0,Qt::UserRole+1).toBool());};
        connect(all,&QPushButton::clicked,this,[selectAll,syncSelection]{selectAll(true);syncSelection();});auto allShortcut=new QShortcut(QKeySequence::SelectAll,layerSearchResults);allShortcut->setContext(Qt::WidgetWithChildrenShortcut);connect(allShortcut,&QShortcut::activated,this,[selectAll,syncSelection]{selectAll(false);syncSelection();});
        connect(layerSearchResults,&QTreeWidget::itemClicked,this,[this](QTreeWidgetItem* item,int column){locateLayer(item->data(0,Qt::UserRole).toString());if(column==0&&QApplication::keyboardModifiers()==Qt::NoModifier)layerSearchResults->editItem(item,0);});
        connect(layerSearchResults,&QTreeWidget::itemChanged,this,[this,status](QTreeWidgetItem* item,int column){if(column!=0||busy)return;QString name=item->text(0).trimmed();if(name.isEmpty()){QSignalBlocker blocker(layerSearchResults);item->setText(0,item->data(0,Qt::UserRole+2).toString());return;}for(auto n:flatten(doc.roots))if(n->id==item->data(0,Qt::UserRole).toString()){if(n->name==name)return;n->name=name;n->manual=true;status->clear();QTimer::singleShot(0,this,[this]{refresh();});break;}});
        connect(replace,&QPushButton::clicked,this,[this,before,after,sensitive,status]{if(busy||before->text().isEmpty())return;QSet<QString> ids;for(auto item:layerSearchResults->selectedItems())ids.insert(item->data(0,Qt::UserRole).toString());int changed=0,skipped=0;for(auto n:flatten(doc.roots)){if(!ids.contains(n->id))continue;QString next=n->name;next.replace(before->text(),after->text(),sensitive->isChecked()?Qt::CaseSensitive:Qt::CaseInsensitive);next=next.trimmed();if(next.isEmpty()){++skipped;continue;}if(next!=n->name){n->name=next;n->manual=true;++changed;}}refresh();status->setText(QStringLiteral("已替换 %1 项").arg(changed)+(skipped?QStringLiteral(" · 跳过 %1 个空名称").arg(skipped):QString()));});
        auto find=new QShortcut(QKeySequence::Find,layerSearchDialog);connect(find,&QShortcut::activated,this,[this]{layerSearchInput->setFocus();layerSearchInput->selectAll();});
    }
    layerSearchDialog->show();layerSearchDialog->raise();layerSearchDialog->activateWindow();updateLayerSearch();layerSearchInput->setFocus();layerSearchInput->selectAll();
}
void Window::updateLayerSearch(){
    if(!layerSearchDialog||!layerSearchDialog->isVisible())return;QString query=layerSearchInput->text().trimmed();auto flat=flatten(doc.roots);QSet<QString> matches,included;QJsonArray rows;
    for(auto n:flat)if(query.isEmpty()||n->name.contains(query,Qt::CaseInsensitive)||n->originalName.contains(query,Qt::CaseInsensitive)){matches.insert(n->id);included.insert(n->id);for(auto p=n->parent;p;p=p->parent)included.insert(p->id);}
    for(auto n:flat)if(included.contains(n->id))rows.append(QJsonObject{{"id",n->id},{"name",n->name},{"parent",n->parent?n->parent->id:QString()},{"group",n->group},{"match",matches.contains(n->id)}});
    auto snapshot=QJsonDocument(rows).toJson(QJsonDocument::Compact);if(snapshot==layerSearchSnapshot)return;layerSearchSnapshot=snapshot;QSet<QString> selected,collapsed;for(auto item:searchItems(layerSearchResults)){auto id=item->data(0,Qt::UserRole).toString();if(item->isSelected())selected.insert(id);if(item->childCount()&&!item->isExpanded())collapsed.insert(id);}
    QSignalBlocker blocker(layerSearchResults);layerSearchResults->clear();
    std::function<void(const Layers&,QTreeWidgetItem*)> build=[&](const Layers& nodes,QTreeWidgetItem* parent){QVector<Layer*> ordered;for(auto& n:nodes)if(included.contains(n->id))ordered.append(n.get());std::stable_sort(ordered.begin(),ordered.end(),[](auto a,auto b){return a->group&&!b->group;});for(auto n:ordered){auto item=parent?new QTreeWidgetItem(parent):new QTreeWidgetItem(layerSearchResults);item->setText(0,n->name);item->setData(0,Qt::UserRole,n->id);item->setData(0,Qt::UserRole+1,matches.contains(n->id));item->setData(0,Qt::UserRole+2,n->name);item->setFlags(item->flags()|Qt::ItemIsEditable);item->setIcon(0,glyph(n->group?"folder":"layer",text()));if(!matches.contains(n->id))item->setForeground(0,muted());build(n->children,item);item->setExpanded(!query.isEmpty()||!collapsed.contains(n->id));item->setSelected(selected.contains(n->id));}};build(doc.roots,nullptr);auto replace=layerSearchDialog->findChild<QPushButton*>("replaceNames");auto before=layerSearchDialog->findChild<QLineEdit*>("replaceBefore");replace->setEnabled(!layerSearchResults->selectedItems().isEmpty()&&!before->text().isEmpty());bool allSelected=!matches.isEmpty();for(auto item:searchItems(layerSearchResults))if(item->data(0,Qt::UserRole+1).toBool()&&!item->isSelected())allSelected=false;layerSearchDialog->findChild<QPushButton*>("searchSelectAll")->setText(allSelected?QStringLiteral("取消全选"):QStringLiteral("全选"));layerSearchCount->setText(QStringLiteral("%1 项").arg(matches.size()));
}
void Window::locateLayer(QString id){if(busy)return;for(auto n:flatten(doc.roots))if(n->id==id){auto target=model->forNode(n);if(tree->isRowHidden(target.row(),target.parent()))layerFilterInput->clear();for(auto parent=n->parent;parent;parent=parent->parent){parent->opened=true;tree->setExpanded(model->forNode(parent),true);}tree->setCurrentIndex(model->forNode(n));tree->scrollTo(model->forNode(n),QAbstractItemView::PositionAtCenter);return;}}

void Window::filterLayers(){
    if(!layerFilterInput)return;
    QString query=layerFilterInput->text().trimmed();
    if(!query.isEmpty()&&!filteringLayers){filterExpanded.clear();for(auto n:flatten(doc.roots))if(n->group&&tree->isExpanded(model->forNode(n)))filterExpanded.insert(n->id);filteringLayers=true;}
    std::function<bool(const Layers&,bool)> walk=[&](const Layers& nodes,bool ancestorMatch){bool any=false;for(auto& ptr:nodes){auto n=ptr.get();bool match=query.isEmpty()||ancestorMatch||n->name.contains(query,Qt::CaseInsensitive)||n->originalName.contains(query,Qt::CaseInsensitive);bool child=walk(n->children,match);auto index=model->forNode(n);bool show=match||child;tree->setRowHidden(index.row(),index.parent(),!show);if(n->group){if(!query.isEmpty()&&show)tree->setExpanded(index,true);else if(query.isEmpty()&&filteringLayers)tree->setExpanded(index,filterExpanded.contains(n->id));}any|=show;}return any;};walk(doc.roots,false);
    if(query.isEmpty()&&filteringLayers){filteringLayers=false;filterExpanded.clear();}
}
