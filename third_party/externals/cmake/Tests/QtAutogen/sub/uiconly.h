
#ifndef UIC_ONLY_H
#define UIC_ONLY_H

#include <QWidget>
#include <memory>

#include "ui_uiconly.h"

class UicOnly : public QWidget
{
  Q_OBJECT
public:
  explicit UicOnly(QWidget *parent = 0);

private:
  const std::auto_ptr<Ui::UicOnly> ui;
};

#endif
