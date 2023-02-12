#include <QFrame>
#include <QStringList>

#include <vector>

class CQCrossword : public QFrame {
  Q_OBJECT

 public:
  CQCrossword(QWidget *parent=nullptr);

  void load(const QString &filename);

  void paintEvent(QPaintEvent *e) override;

  void keyPressEvent(QKeyEvent *e) override;

  void draw(QPainter *painter);

  QSize sizeHint() const override { return QSize(1800, 1800); }

 private:
  struct Cell {
    Cell() { }
    Cell(char c1) : c(c1) { }

    char c    { 0 };
    int  num  { 0 };
    bool is_l { false };
    bool is_r { false };
    bool is_u { false };
    bool is_d { false };
  };

  using CellRow = std::vector<Cell>;
  using Grid    = std::vector<CellRow>;

  QStringList lines_;
  int         nr_ { 0 };
  int         nc_ { 0 };
  Grid        grid_;
};
