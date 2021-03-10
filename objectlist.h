#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include <QAbstractListModel>
#include <QtDebug>
#include <QRect>
#include <QThread>
#include <QElapsedTimer>

struct Cell;
struct Param;

typedef QPair<const Cell*, Param> Node;

struct Cell {
    QRect rectangle;
    Cell* rightNode = nullptr;
    Cell* rightTopNode = nullptr;
    Cell* topNode = nullptr;
    Cell* leftNode = nullptr;
    Cell* leftTopNode = nullptr;
    Cell* bottomNode = nullptr;
    Cell* leftBottomNode = nullptr;
    Cell* rightBottomNode = nullptr;

    Cell(const QRect& aPosition) { rectangle = aPosition; }

    QList<const Cell *> directions() const;
    QHash<const Cell*, int> gCostDirections() const;
};

struct Param {
    const Cell* parent;
    int fCost = 0;
    int gCost = 0;
    int hCost = 0;

    Param(){}
    Param(const Cell* aPrevCell)
    {
        parent = aPrevCell;
    }

    Param& operator = (const Param& aNode) {
        fCost = aNode.fCost;
        gCost = aNode.gCost;
        hCost = aNode.hCost;
        parent = aNode.parent;
        return *this;
    }

    bool operator == (const Param& aNode) const
    {
        return fCost == aNode.fCost;
    }

    bool operator < (const Param& aNode) const
    {
        return fCost < aNode.fCost;
    }
};

class ObjectList : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Position {
        PositionRole = Qt::UserRole
    };

private:
    QList<Cell*> m_map;

    //  Возвращает количество клеток до цели, игнорируя препятсвия
    inline int distance(const QPoint& aAgent, const QPoint& aTarget);

public:
    explicit ObjectList(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    //  Находим ноду по координате
    Cell* findNode(const QPoint& aPoint) const;
    Q_INVOKABLE QPointF findPosNode(const QPoint& aPoint) const;
    Cell* takeNode(const QPoint& aPoint);

public slots:
    //  Заполнение карты квадратами
    void setMapSize(const QSize& aMapSize);
    void setMapSize(const int aWidth, const int aHeight);

    //  Добавить объект на карту
    void addObject(const QPoint& aPoint);

    //  Найти маршрут
    QVariantList findRoute(const QPoint& aUnit, const QPoint& aTarget);

signals:
    void highlightGreen(QPointF position);
    void highlightRed(QPointF position);
    void highlightBlue(QPointF position);
    void removeNode(QPointF position);
};

#endif // OBJECTLIST_H
