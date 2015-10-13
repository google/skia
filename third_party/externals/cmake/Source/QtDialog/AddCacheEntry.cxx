/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "AddCacheEntry.h"
#include <QMetaProperty>
#include <QCompleter>

static const int NumTypes = 4;
static const int DefaultTypeIndex = 0;
static const QByteArray TypeStrings[NumTypes] =
  { "BOOL", "PATH", "FILEPATH", "STRING" };
static const QCMakeProperty::PropertyType Types[NumTypes] =
  { QCMakeProperty::BOOL, QCMakeProperty::PATH,
    QCMakeProperty::FILEPATH, QCMakeProperty::STRING};

AddCacheEntry::AddCacheEntry(QWidget* p, const QStringList& varNames,
                                         const QStringList& varTypes)
  : QWidget(p), VarNames(varNames), VarTypes(varTypes)
{
  this->setupUi(this);
  for(int i=0; i<NumTypes; i++)
    {
    this->Type->addItem(TypeStrings[i]);
    }
  QWidget* cb = new QCheckBox();
  QWidget* path = new QCMakePathEditor();
  QWidget* filepath = new QCMakeFilePathEditor();
  QWidget* string = new QLineEdit();
  this->StackedWidget->addWidget(cb);
  this->StackedWidget->addWidget(path);
  this->StackedWidget->addWidget(filepath);
  this->StackedWidget->addWidget(string);
  this->setTabOrder(this->Name, this->Type);
  this->setTabOrder(this->Type, cb);
  this->setTabOrder(cb, path);
  this->setTabOrder(path, filepath);
  this->setTabOrder(filepath, string);
  this->setTabOrder(string, this->Description);
  QCompleter *completer = new QCompleter(this->VarNames, this);
  this->Name->setCompleter(completer);
  connect(completer, SIGNAL(activated(const QString&)),
          this, SLOT(onCompletionActivated(const QString&)));
}

QString AddCacheEntry::name() const
{
  return this->Name->text();
}

QVariant AddCacheEntry::value() const
{
  QWidget* w = this->StackedWidget->currentWidget();
  if(qobject_cast<QLineEdit*>(w))
    {
    return static_cast<QLineEdit*>(w)->text();
    }
  else if(qobject_cast<QCheckBox*>(w))
    {
    return static_cast<QCheckBox*>(w)->isChecked();
    }
  return QVariant();
}

QString AddCacheEntry::description() const
{
  return this->Description->text();
}

QCMakeProperty::PropertyType AddCacheEntry::type() const
{
  int idx = this->Type->currentIndex();
  if(idx >= 0 && idx < NumTypes)
    {
    return Types[idx];
    }
  return Types[DefaultTypeIndex];
}

QString AddCacheEntry::typeString() const
{
  int idx = this->Type->currentIndex();
  if(idx >= 0 && idx < NumTypes)
    {
    return TypeStrings[idx];
    }
  return TypeStrings[DefaultTypeIndex];
}

void AddCacheEntry::onCompletionActivated(const QString &text)
{
  int idx = this->VarNames.indexOf(text);
  if (idx != -1)
    {
    QString vartype = this->VarTypes[idx];
    for (int i = 0; i < NumTypes; i++)
      {
        if (TypeStrings[i] == vartype)
          {
          this->Type->setCurrentIndex(i);
          break;
          }
      }
    }
}
