
#include "libwidget.h"

LibWidget::LibWidget(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::LibWidget)
{
  ui->setupUi(this);
}
