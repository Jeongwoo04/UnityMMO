#include "pch.h"
#include "Map.h"
#include "GameObject.h"

GameObjectRef Map::Find(Vector2Int cellPos)
{
    if (cellPos._x < _minX || cellPos._x > _maxX)
        return nullptr;
    if (cellPos._y < _minY || cellPos._y > _maxY)
        return nullptr;

    int x = cellPos._x - _minX;
    int y = _maxY - cellPos._y;
    return _objects[y][x];
}

bool Map::ApplyLeave(GameObjectRef gameObject)
{
    if (gameObject->GetRoom() == nullptr)
        return false;
    if (gameObject->GetRoom()->GetMap() != shared_from_this())
        return false;

    PositionInfo* posInfo = gameObject->_posInfo();
    if (posInfo->posx() < _minX || posInfo->posx() > _maxX)
        return false;
    if (posInfo->posy() < _minY || posInfo->posy() > _maxY)
        return false;

    {
        int x = posInfo->posx() - _minX;
        int y = _maxY - posInfo->posy();
        if (_objects[y][x] == gameObject)
            _objects[y][x] = nullptr;
    }

    return true;
}

bool Map::ApplyMove(GameObjectRef gameObject, Vector2Int dest)
{
    ApplyLeave(gameObject);

    if (gameObject->GetRoom() == nullptr)
        return false;
    if (gameObject->GetRoom()->GetMap() != shared_from_this())
        return false;

    PositionInfo* posInfo = gameObject->_posInfo();
    if (CanGo(dest, true) == false)
        return false;

    {
        int32 x = dest._x - _minX;
        int32 y = _maxY - dest._y;
        _objects[y][x] = gameObject;
    }

    // 실제 좌표 이동
    posInfo->set_posx(dest._x);
    posInfo->set_posy(dest._y);

    return true;
}

void Map::LoadMap(int32 mapId, string pathPrefix)
{
    stringstream ss;
    ss << pathPrefix << "/Map_" << setfill('0') << setw(3) << mapId << ".txt";
    // Collision file data load
    ifstream inFile(ss.str());
    if (!inFile.is_open())
        return;

    std::wcout << "Stream good? " << inFile.good() << std::endl;
    std::wcout << "Stream fail? " << inFile.fail() << std::endl;
    std::wcout << "Stream eof? " << inFile.eof() << std::endl;

    std::wcout << "this = " << this << std::endl;
    std::wcout << "&_minX = " << &_minX << std::endl;

    inFile >> _minX >> _maxX >> _minY >> _maxY;

    int xCount = _maxX - _minX + 1;
    int yCount = _maxY - _minY + 1;

    _collision.resize(yCount, std::vector<bool>(xCount, false));
    _objects.resize(yCount, std::vector<GameObjectRef>(xCount, nullptr));

    string line;
    getline(inFile, line);
    for (int y = 0; y < yCount; y++)
    {
        getline(inFile, line);
        for (int x = 0; x < xCount; x++)
        {
            _collision[y][x] = (line[x] == '1' ? true : false);
        }
    }

    _sizeX = xCount;
    _sizeY = yCount;
}

vector<Vector2Int> Map::FindPath(Vector2Int startCellPos, Vector2Int destCellPos, bool checkObjects)
{
    vector<vector<bool>> closed(_sizeY, vector<bool>(_sizeX, false));
    vector<vector<int>> open(_sizeY, vector<int>(_sizeX, numeric_limits<int>::max()));
    vector<vector<Pos>> parent(_sizeY, vector<Pos>(_sizeX));

    priority_queue<PQNode> pq;

    Pos pos = Cell2Pos(startCellPos);
    Pos dest = Cell2Pos(destCellPos);

    int h = 10 * (abs(dest._y - pos._y) + abs(dest._x - pos._x));
    open[pos._y][pos._x] = h;
    pq.push({ h, 0, pos._y, pos._x });
    parent[pos._y][pos._x] = pos;

    const int deltaY[4] = { 1, -1, 0, 0 };
    const int deltaX[4] = { 0, 0, -1, 1 };

    while (!pq.empty()) {
        PQNode node = pq.top(); pq.pop();

        if (closed[node.Y][node.X])
            continue;
        closed[node.Y][node.X] = true;
        if (node.Y == dest._y && node.X == dest._x)
            break;

        for (int i = 0; i < 4; ++i) {
            Pos next(node.Y + deltaY[i], node.X + deltaX[i]);
            if (!InRange(next))
                continue;
            if ((next._y != dest._y || next._x != dest._x) && !CanGo(Pos2Cell(next), checkObjects))
                continue;
            if (closed[next._y][next._x])
                continue;

            int g = node.G + 10;
            int h = 10 * ((dest._y - next._y) * (dest._y - next._y) + (dest._x - next._x) * (dest._x - next._x));
            if (open[next._y][next._x] <= g + h)
                continue;

            open[next._y][next._x] = g + h;
            pq.push({ g + h, g, next._y, next._x });
            parent[next._y][next._x] = { node.Y, node.X };
        }
    }

    return CalcCellPathFromParent(parent, dest);
}

vector<Vector2Int> Map::CalcCellPathFromParent(const vector<vector<Pos>>& parent, const Pos& dest)
{
    vector<Vector2Int> cells;
    int32 y = dest._y;
    int32 x = dest._x;
    while (!(parent[y][x] == Pos(y, x))) {
        cells.push_back(Pos2Cell({ y, x }));
        Pos p = parent[y][x];
        y = p._y;
        x = p._x;
    }
    cells.push_back(Pos2Cell({ y, x }));
    reverse(cells.begin(), cells.end());
    return cells;
}

Pos Map::Cell2Pos(const Vector2Int& cell)
{
    return { MaxY - cell._y, cell._x - MinX };
}

Vector2Int Map::Pos2Cell(const Pos& pos)
{
    return { pos._x + MinX, MaxY - pos._y };
}

bool Map::InRange(const Pos& pos)
{
    return pos._y >= 0 && pos._y < _sizeY && pos._x >= 0 && pos._x < _sizeX;
}

bool Map::CanGo(Vector2Int cell, bool checkObjects)
{
    Pos pos = Cell2Pos(cell);

    if (!InRange(pos))
        return false;
    if (_collision[pos._y][pos._x])
        return false;
    if (checkObjects && _objects[pos._y][pos._x] != nullptr)
        return false;

    return true;
}