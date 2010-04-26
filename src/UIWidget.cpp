//////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.h>

#include <wendy/Core.h>
#include <wendy/Block.h>
#include <wendy/Color.h>
#include <wendy/Vector.h>
#include <wendy/Matrix.h>
#include <wendy/Rectangle.h>
#include <wendy/Plane.h>
#include <wendy/Segment.h>
#include <wendy/Triangle.h>
#include <wendy/Bezier.h>
#include <wendy/Quaternion.h>
#include <wendy/Transform.h>
#include <wendy/Sphere.h>
#include <wendy/Frustum.h>
#include <wendy/Random.h>
#include <wendy/Pixel.h>
#include <wendy/Vertex.h>
#include <wendy/Timer.h>
#include <wendy/Signal.h>
#include <wendy/Path.h>
#include <wendy/Stream.h>
#include <wendy/Managed.h>
#include <wendy/Resource.h>
#include <wendy/Image.h>
#include <wendy/Font.h>

#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLVertex.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLRender.h>
#include <wendy/GLState.h>

#include <wendy/RenderMaterial.h>
#include <wendy/RenderFont.h>

#include <wendy/Input.h>

#include <wendy/UIRender.h>
#include <wendy/UIDesktop.h>
#include <wendy/UIWidget.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Widget::Widget(Desktop& initDesktop, Widget* initParent):
  desktop(initDesktop),
  parent(initParent),
  enabled(true),
  visible(true),
  draggable(false),
  area(0.f, 0.f, 0.f, 0.f)
{
  if (parent)
  {
    if (&desktop != &(parent->getDesktop()))
      throw Exception("Parent widget has different desktop");

    parent->children.push_back(this);
    parent->addedChild(*this);
  }
  else
    desktop.roots.push_back(this);
}

Widget::~Widget(void)
{
  destroyChildren();

  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(desktop.roots);

  WidgetList::iterator i = std::find(siblings->begin(), siblings->end(), this);
  if (i != siblings->end())
  {
    siblings->erase(i);
    desktop.removedWidget(*this);

    if (parent)
      parent->removedChild(*this);
  }

  destroyedSignal.emit(*this);
}

void Widget::destroyChildren(void)
{
  while (!children.empty())
    delete children.back();
}

Widget* Widget::findByPoint(const Vec2& point)
{
  if (!area.contains(point))
    return NULL;

  const Vec2 localPoint = point - area.position;

  for (WidgetList::const_iterator c = children.begin();  c != children.end();  c++)
  {
    if ((*c)->isVisible())
      if (Widget* result = (*c)->findByPoint(localPoint))
	return result;
  }

  return this;
}

Vec2 Widget::transformToLocal(const Vec2& globalPoint) const
{
  return globalPoint - getGlobalArea().position;
}

Vec2 Widget::transformToGlobal(const Vec2& localPoint) const
{
  return localPoint + getGlobalArea().position;
}

void Widget::enable(void)
{
  enabled = true;
}

void Widget::disable(void)
{
  enabled = false;
}

void Widget::activate(void)
{
  desktop.setActiveWidget(this);
}

void Widget::bringToFront(void)
{
  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(desktop.roots);

  WidgetList::iterator i = std::find(siblings->begin(), siblings->end(), this);
  siblings->erase(i);
  siblings->push_back(this);
}

void Widget::sendToBack(void)
{
  WidgetList* siblings;

  if (parent)
    siblings = &(parent->children);
  else
    siblings = &(desktop.roots);

  WidgetList::iterator i = std::find(siblings->begin(), siblings->end(), this);
  siblings->erase(i);
  siblings->insert(siblings->begin(), this);
}

void Widget::cancelDragging(void)
{
  if (isBeingDragged())
    desktop.cancelDragging();
}

bool Widget::isEnabled(void) const
{
  if (!enabled)
    return false;

  if (parent)
    return parent->isEnabled();

  return true;
}

bool Widget::isVisible(void) const
{
  if (!visible)
    return false;

  if (parent)
    return parent->isVisible();

  return true;
}

bool Widget::isActive(void) const
{
  return desktop.getActiveWidget() == this;
}

bool Widget::isUnderCursor(void) const
{
  return desktop.getHoveredWidget() == this;
}

bool Widget::isDraggable(void) const
{
  return draggable;
}

bool Widget::isBeingDragged(void) const
{
  return desktop.getDraggedWidget() == this;
}

bool Widget::isChildOf(const Widget& widget) const
{
  for (Widget* ancestor = parent;  ancestor;  ancestor = ancestor->parent)
  {
    if (ancestor == &widget)
      return true;
  }

  return false;
}

Desktop& Widget::getDesktop(void) const
{
  return desktop;
}

Widget* Widget::getParent(void) const
{
  return parent;
}

const WidgetList& Widget::getChildren(void) const
{
  return children;
}

WidgetState Widget::getState(void) const
{
  if (isEnabled())
  {
    if (isActive())
      return STATE_ACTIVE;
    else
      return STATE_NORMAL;
  }
  else
    return STATE_DISABLED;
}

const Rect& Widget::getArea(void) const
{
  return area;
}

const Rect& Widget::getGlobalArea(void) const
{
  globalArea = area;

  if (parent)
    globalArea.position += parent->getGlobalArea().position;

  return globalArea;
}

void Widget::setArea(const Rect& newArea)
{
  area = newArea;
  areaChangedSignal.emit(*this);
}

void Widget::setSize(const Vec2& newSize)
{
  setArea(Rect(area.position, newSize));
}

void Widget::setPosition(const Vec2& newPosition)
{
  setArea(Rect(newPosition, area.size));
}

void Widget::setVisible(bool newState)
{
  visible = newState;
}

void Widget::setDraggable(bool newState)
{
  draggable = newState;

  if (!draggable)
    cancelDragging();
}

SignalProxy1<void, Widget&> Widget::getDestroyedSignal(void)
{
  return destroyedSignal;
}

SignalProxy1<void, Widget&> Widget::getAreaChangedSignal(void)
{
  return areaChangedSignal;
}

SignalProxy2<void, Widget&, bool> Widget::getFocusChangedSignal(void)
{
  return focusChangedSignal;
}

SignalProxy3<void, Widget&, input::Key, bool> Widget::getKeyPressedSignal(void)
{
  return keyPressedSignal;
}

SignalProxy2<void, Widget&, wchar_t> Widget::getCharInputSignal(void)
{
  return charInputSignal;
}

SignalProxy2<void, Widget&, const Vec2&> Widget::getCursorMovedSignal(void)
{
  return cursorMovedSignal;
}

SignalProxy4<void, Widget&, const Vec2&, input::Button, bool> Widget::getButtonClickedSignal(void)
{
  return buttonClickedSignal;
}

SignalProxy2<void, Widget&, int> Widget::getWheelTurnedSignal(void)
{
  return wheelTurnedSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorEnteredSignal(void)
{
  return cursorEnteredSignal;
}

SignalProxy1<void, Widget&> Widget::getCursorLeftSignal(void)
{
  return cursorLeftSignal;
}

SignalProxy2<void, Widget&, const Vec2&> Widget::getDragBegunSignal(void)
{
  return dragBegunSignal;
}

SignalProxy2<void, Widget&, const Vec2&> Widget::getDragMovedSignal(void)
{
  return dragMovedSignal;
}

SignalProxy2<void, Widget&, const Vec2&> Widget::getDragEndedSignal(void)
{
  return dragEndedSignal;
}

void Widget::draw(void) const
{
  for (WidgetList::const_iterator i = children.begin();  i != children.end();  i++)
  {
    if ((*i)->isVisible())
      (*i)->draw();
  }
}

void Widget::addedChild(Widget& child)
{
}

void Widget::removedChild(Widget& child)
{
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
