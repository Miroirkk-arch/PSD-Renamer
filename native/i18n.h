#pragma once
#include <QtWidgets>
QString ui(const char* source);
QString uiLanguage();
QString uiSource(const QString& translated);
void setUiLanguage(QString language);
void bindUiText(QAbstractButton* button);
void retranslateUiButtons(QWidget* root);

void fitUiButton(QAbstractButton* button);
