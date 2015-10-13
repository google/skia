

#ifndef COMPILERS_HPP
#define COMPILERS_HPP

#include <QWidget>
#include <ui_Compilers.h>

class Compilers : public QWidget, public Ui::Compilers
{
  Q_OBJECT
public:
  Compilers(QWidget* p=NULL) :
    QWidget(p)
  {
    this->setupUi(this);
  }
};

#endif

