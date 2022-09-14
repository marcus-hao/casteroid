#include <cstdint>
#include <cstdlib>
#include <vector>

#include "game.h"
#include "ObjectField.h"

SpaceObject::SpaceObject(int x, int y)
{
    pos.x = x;
    pos.y = y;
}

void SpaceObject::update()
{
    pos.y++;
}

vec2i SpaceObject::getPos() const
{
    return pos;
}

void ObjectField::setBounds(rect a)
{
    field_bounds = a;
}

std::vector<SpaceObject> ObjectField::getData() const
{
    return object_set;
}

void ObjectField::erase(size_t i)
{
    object_set.erase(object_set.begin() + i);
}

void ObjectField::update()
{
    /* Update objects on screen */
    for (size_t i = 0; i < object_set.size(); i++) {
        if (object_set.at(i).getPos().y > field_bounds.bot()) {
            object_set.erase(object_set.begin() + i);
        }
        object_set.at(i).update();
    }
    /* Spawn objects */
    SpaceObject s(rand() % field_bounds.width(), 0);
    object_set.push_back(s);
}