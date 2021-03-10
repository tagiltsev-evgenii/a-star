#include "objectlist.h"

ObjectList::ObjectList(QObject *parent)
    : QAbstractListModel(parent)
{

}

int ObjectList::distance(const QPoint &aAgent, const QPoint &aTarget)
{
    int xDistance = qAbs(aTarget.x() - aAgent.x()) / 32;
    int yDistance = qAbs(aTarget.y() - aAgent.y()) / 32;

    return xDistance + yDistance;
}

int ObjectList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_map.count();
}

QVariant ObjectList::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    if(role == PositionRole)
        return m_map[index.row()]->rectangle;

    return QVariant();
}

QHash<int, QByteArray> ObjectList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PositionRole] = "position";
    return roles;
}

Cell* ObjectList::findNode(const QPoint &aPoint) const
{
    for(int i = 0; i < m_map.count(); i++)
    {
        if(m_map[i]->rectangle.contains(aPoint))
            return m_map[i];
    }

    return nullptr;
}

QPointF ObjectList::findPosNode(const QPoint &aPoint) const
{
    Cell* cell = findNode(aPoint);

    if(cell != nullptr)
        return cell->rectangle.topLeft();

    return QPointF(0, 0);
}

Cell *ObjectList::takeNode(const QPoint &aPoint)
{
    for(int i = 0; i < m_map.count(); i++)
    {
        if(m_map[i]->rectangle.contains(aPoint)) {
            return m_map.takeAt(i);
        }
    }

    return nullptr;
}

void ObjectList::setMapSize(const QSize &aMapSize)
{
    beginResetModel();
    //  Вычисляем количество нод по вертикали и горизонтали
    int width = aMapSize.width() / 48;
    int height = aMapSize.height() / 48;

    //  Чистим старую карту
    for(int i = 0; i < m_map.count(); i++)
        delete m_map[i];

    m_map.clear();

    //  Заполняем карту нодами
    for(int row = 0; row < height; row++)
    {
        for(int column = 0; column < width; column++)
        {
            Cell* node = new Cell(QRect(column * 48, row * 48, 48, 48));
            if(column) {
                Cell* leftNode = m_map[m_map.count() - 1];
                node->leftNode = leftNode;
                leftNode->rightNode = node;

                if(row) {
                    Cell* leftTopNode = m_map[m_map.count() - width - 1];
                    node->leftTopNode = leftTopNode;
                    leftTopNode->rightBottomNode = node;
                }
            }

            if(row) {
                Cell* anotherNode = m_map[m_map.count() - width];
                node->topNode = anotherNode;
                anotherNode->bottomNode = node;

                if(column != width - 1) {
                    Cell* rightTopNode = m_map[m_map.count() - width + 1];
                    node->rightTopNode = rightTopNode;
                    rightTopNode->leftBottomNode = node;
                }
            }

            m_map.append(node);
        }
    }

    qDebug() << "map was built. width:" << width << "height:" << height;
    endResetModel();
}

void ObjectList::setMapSize(const int aWidth, const int aHeight)
{
    setMapSize(QSize(aWidth, aHeight));
}

void ObjectList::addObject(const QPoint &aPoint)
{
    Cell* node = takeNode(aPoint);

    if(node != nullptr) {
        if(node->leftTopNode != nullptr)        node->leftTopNode->rightBottomNode = nullptr;
        if(node->topNode != nullptr)            node->topNode->bottomNode = nullptr;
        if(node->rightTopNode != nullptr)       node->rightTopNode->leftBottomNode = nullptr;
        if(node->leftNode != nullptr)           node->leftNode->rightNode = nullptr;
        if(node->rightNode != nullptr)          node->rightNode->leftNode = nullptr;
        if(node->leftBottomNode != nullptr)     node->leftBottomNode->rightTopNode = nullptr;
        if(node->bottomNode != nullptr)         node->bottomNode->topNode = nullptr;
        if(node->rightBottomNode != nullptr)    node->rightBottomNode->leftTopNode = nullptr;

        emit removeNode(node->rectangle.topLeft());
        delete node;
    }
}

QVariantList ObjectList::findRoute(const QPoint &aUnit, const QPoint &aTarget)
{
    QElapsedTimer timer;
    timer.start();

    QList<QVariant> path;
    const Cell* targetNode = findNode(aTarget);
    const Cell* agentNode = findNode(aUnit);

    if(targetNode != nullptr && agentNode != nullptr)
    {
//        emit highlightBlue(targetNode->rectangle.topLeft());

        QHash<const Cell*, Param> openList;
        QHash<const Cell*, Param> closeList;

        Param startParam(nullptr);
        startParam.hCost = distance(agentNode->rectangle.topLeft(), targetNode->rectangle.topLeft()) * 10;
        startParam.fCost = startParam.hCost;
        openList[agentNode] = startParam;

        while (!openList.isEmpty())
        {
            auto activeNode = std::min_element(openList.begin(), openList.end(), [](Param& a, Param& b){ return a.fCost < b.fCost; });

            closeList[activeNode.key()] = activeNode.value();
//            emit highlightBlue(activeNode.key()->rectangle.topLeft());

            if(activeNode.key() == targetNode) break;

            QHash<const Cell*, int> directions = activeNode.key()->gCostDirections();

            for(auto iterator = directions.begin(); iterator != directions.end(); iterator++)
            {
                if(closeList.contains(iterator.key())) continue;

                int gCost = iterator.value() + activeNode.value().gCost;
                int hCost = distance(iterator.key()->rectangle.topLeft(), targetNode->rectangle.topLeft()) * 10;
                int fCost = gCost + hCost;

                auto openCell = openList.find(iterator.key());

                if(openCell != openList.end())
                {
                    if(gCost < openCell.value().gCost)
                    {
                        openCell.value().gCost = gCost;
                        openCell.value().hCost = hCost;
                        openCell.value().fCost = gCost + hCost;
                        openCell.value().parent = activeNode.key();
                    }

                    continue;
                }

                Param param(activeNode.key());
                param.gCost = gCost;
                param.hCost = hCost;
                param.fCost = fCost;
                openList[iterator.key()] = param;

//                emit highlightGreen(iterator.key()->rectangle.topLeft());
            }

            openList.erase(activeNode);
        }

        const Cell* currentPos = targetNode;

        while (currentPos != agentNode)
        {
//            emit highlightRed(currentPos->rectangle.topLeft());
            path << currentPos->rectangle.topLeft();
            currentPos = closeList[currentPos].parent;
        }

        std::reverse(path.begin(), path.end());
    }

    qDebug() << "speed:" << double(timer.nsecsElapsed()) / 1000000 << "ms";

    return path;
}

QList<const Cell *> Cell::directions() const
{
    QList<const Cell *> directions;

    if(leftTopNode != nullptr)      directions << leftTopNode;
    if(topNode != nullptr)          directions << topNode;
    if(rightTopNode != nullptr)     directions << rightTopNode;
    if(leftNode != nullptr)         directions << leftNode;
    if(rightNode != nullptr)        directions << rightNode;
    if(leftBottomNode != nullptr)   directions << leftBottomNode;
    if(bottomNode != nullptr)       directions << bottomNode;
    if(rightBottomNode != nullptr)  directions << rightBottomNode;

    return directions;
}

QHash<const Cell *, int> Cell::gCostDirections() const
{
   QHash<const Cell *, int> directions;

   if(leftTopNode != nullptr)      { directions[leftTopNode]      = 14; }
   if(topNode != nullptr)          { directions[topNode]          = 10; }
   if(rightTopNode != nullptr)     { directions[rightTopNode]     = 14; }
   if(leftNode != nullptr)         { directions[leftNode]         = 10; }
   if(rightNode != nullptr)        { directions[rightNode]        = 10; }
   if(leftBottomNode != nullptr)   { directions[leftBottomNode]   = 14; }
   if(bottomNode != nullptr)       { directions[bottomNode]       = 10; }
   if(rightBottomNode != nullptr)  { directions[rightBottomNode]  = 14; }

   return directions;
}
