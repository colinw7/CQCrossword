#include <CQCrossword.h>

#include <QApplication>
#include <QStringList>
#include <QPainter>
#include <QSvgGenerator>
#include <QKeyEvent>

#include <cstdio>
#include <iostream>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  auto font = QFont("Calibri", 20);
  app.setFont(font);

  auto *crossword = new CQCrossword;
  crossword->setFont(font);

  crossword->load("crossword.txt");

  crossword->show();

  return app.exec();
}

CQCrossword::
CQCrossword(QWidget *parent) :
 QFrame(parent)
{
}

void
CQCrossword::
load(const QString &filename)
{
  auto *fp = fopen(filename.toLatin1().constData(), "r");

  QString line;

  lines_ = QStringList();

  while (! feof(fp)) {
    auto c = fgetc(fp);

    if (c == '\n') {
      lines_.push_back(line);

      line = "";
    }
    else
      line += c;
  }

  fclose(fp);

  nr_ = lines_.length();
  nc_ = 0;

  for (auto &line : lines_) {
    //std::cout << line.toStdString() << "\n";
    nc_ = std::max(nc_, line.length());
  }

  grid_.resize(nr_);

  for (int r = 0; r < nr_; ++r)
    grid_[r].resize(nc_);

  int r = 0;

  for (auto &line : lines_) {
    int c = 0;

    for (const auto &lc : line) {
      grid_[r][c] = Cell(lc.toLatin1());

      ++c;
    }

    while (c < nc_)
      grid_[r][c++] = ' ';

    ++r;
  }

  auto isBlankCell = [&](Cell *cell) {
    return (! cell || cell->c == ' ');
  };

  int num = 1;

  for (int r = 0; r < nr_; ++r) {
    for (int c = 0; c < nc_; ++c) {
      auto &cell = grid_[r][c];

      if (cell.c == ' ')
        continue;

      int cl = c - 1;
      int cr = c + 1;
      int ru = r - 1;
      int rd = r + 1;

      auto *cell_l = (cl >= 0  ? &grid_[r][cl] : nullptr);
      auto *cell_r = (cr < nc_ ? &grid_[r][cr] : nullptr);
      auto *cell_u = (ru >= 0  ? &grid_[ru][c] : nullptr);
      auto *cell_d = (rd < nr_ ? &grid_[rd][c] : nullptr);

      cell.is_l = ! isBlankCell(cell_l);
      cell.is_r = ! isBlankCell(cell_r);
      cell.is_u = ! isBlankCell(cell_u);
      cell.is_d = ! isBlankCell(cell_d);

      if (! cell.is_l && cell.is_r) {
        cell.num = num++;
        continue;
      }

      if (! cell.is_u && cell.is_d) {
        cell.num = num++;
        continue;
      }
    }
  }
}

void
CQCrossword::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  draw(&painter);
}

void
CQCrossword::
draw(QPainter *painter)
{
  double border = 8;

  painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  painter->fillRect(rect(), Qt::white);

  double cw = (nc_ > 0 ? double(width () - 2*border)/double(nc_) : 0.0);
  double ch = (nr_ > 0 ? double(height() - 2*border)/double(nr_) : 0.0);

  auto cs = std::min(cw, ch);

  auto charFont = painter->font();
  charFont.setPointSizeF(cs*0.65);

  auto numberFont = painter->font();
  numberFont.setPointSizeF(cs*0.25);

  QFontMetricsF charFm(charFont);

  double charFontAscent  = charFm.ascent();
  double charFontDescent = charFm.descent();

  QFontMetricsF numberFm(numberFont);

  double numberFontAscent  = numberFm.ascent();
//double numberFontDescent = numberFm.descent();

  double y = border;

  for (int r = 0; r < nr_; ++r) {
    double x = border;

    for (int c = 0; c < nc_; ++c) {
      const auto &cell = grid_[r][c];

      QRectF rect(x, y, cs, cs);
      QRectF rect1(x + 2, y + 2, cs - 4, cs - 4);

      if (cell.c == ' ') {
        //painter->fillRect(rect, Qt::white);
      }
      else {
        // draw cell background
        painter->fillRect(rect, QColor(240, 240, 240));

        //---

        // draw character
        auto charWidth = charFm.horizontalAdvance(cell.c);

        QString s = " "; s[0] = cell.c;

        painter->setFont(charFont);
        painter->drawText(x + cs/2 - charWidth/2,
                          y + cs/2 + (charFontAscent - charFontDescent)/2, s);

        //---

        // draw number
        if (cell.num > 0) {
          auto numStr = QString::number(cell.num);

          painter->setFont(numberFont);

          painter->drawText(x + 2, y + 2 + numberFontAscent, numStr);
        }
      }

      x += cs;
    }

    y += cs;
  }

  //---

  y = border;

  for (int r = 0; r < nr_; ++r) {
    double x = border;

    for (int c = 0; c < nc_; ++c) {
      const auto &cell = grid_[r][c];

      QRectF rect(x, y, cs, cs);
      QRectF rect1(x + 2, y + 2, cs - 4, cs - 4);

      if (cell.c != ' ') {
        // draw cell edges
        QPen pen;
        pen.setColor(Qt::black);
        pen.setWidthF(3);
        painter->setPen(pen);

        if (! cell.is_l)
          painter->drawLine(x, y, x, y + cs - 1);
        painter->drawLine(x + cs, y, x + cs, y + cs - 1);

        if (! cell.is_u)
          painter->drawLine(x, y, x + cs - 1, y);
        painter->drawLine(x, y + cs, x + cs - 1, y + cs);
      }

      x += cs;
    }

    y += cs;
  }
}

void
CQCrossword::
keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_P) {
    QSvgGenerator generator;

    generator.setFileName("crossword.svg");
    generator.setSize(QSize(1024, 1024));
    generator.setViewBox(QRect(0, 0, 192, 192));
    generator.setTitle("Crossword");

    QPainter painter;

    bool rc = painter.begin(&generator);

    if (rc) {
      painter.setFont(this->font());

      draw(&painter);

      painter.end();
    }
  }
}
