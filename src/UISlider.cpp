//////////////////////////////////////////////////////////////////////
// Wendy user interface library
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

#include <wendy/Config.hpp>

#include <wendy/UIDrawer.hpp>
#include <wendy/UILayer.hpp>
#include <wendy/UIWidget.hpp>
#include <wendy/UISlider.hpp>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Slider::Slider(Layer& layer, Orientation initOrientation):
  Widget(layer),
  minValue(0.f),
  maxValue(1.f),
  stepSize(1.f),
  value(0.f),
  orientation(initOrientation)
{
  const float em = layer.getDrawer().getCurrentEM();

  if (orientation == HORIZONTAL)
    setSize(vec2(em * 10.f, em * 1.5f));
  else
    setSize(vec2(em * 1.5f, em * 10.f));

  getKeyPressedSignal().connect(*this, &Slider::onKey);
  getButtonClickedSignal().connect(*this, &Slider::onMouseButton);
  getScrolledSignal().connect(*this, &Slider::onScroll);
  getDragMovedSignal().connect(*this, &Slider::onDragMoved);

  setDraggable(true);
}

float Slider::getMinValue() const
{
  return minValue;
}

float Slider::getMaxValue() const
{
  return maxValue;
}

void Slider::setValueRange(float newMinValue, float newMaxValue)
{
  minValue = newMinValue;
  maxValue = newMaxValue;

  if (value < minValue)
    setValue(minValue, true);
  else if (value > maxValue)
    setValue(maxValue, true);
  else
    invalidate();
}

float Slider::getValue() const
{
  return value;
}

void Slider::setValue(float newValue)
{
  setValue(newValue, false);
}

float Slider::getStepSize() const
{
  return stepSize;
}

void Slider::setStepSize(float newSize)
{
  stepSize = max(newSize, 0.f);
}

SignalProxy1<void, Slider&> Slider::getValueChangedSignal()
{
  return valueChangedSignal;
}

void Slider::draw() const
{
  const Rect& area = getGlobalArea();

  Drawer& drawer = getLayer().getDrawer();
  if (drawer.pushClipArea(area))
  {
    drawer.drawWell(area, getState());

    const float fraction = (value - minValue) / (maxValue - minValue);
    const float width = getWidth();
    const float height = getHeight();

    Rect handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + fraction * (width - height),
                     area.position.y,
                     height,
                     height);
    }
    else
    {
      handleArea.set(area.position.x,
                     area.position.y + fraction * (height - width),
                     width,
                     width);
    }

    drawer.drawHandle(handleArea, getState());

    Widget::draw();

    drawer.popClipArea();
  }
}

void Slider::onMouseButton(Widget& widget,
                           vec2 position,
                           MouseButton button,
                           Action action)
{
  if (action == PRESSED)
    setValue(transformToLocal(position));
}

void Slider::onKey(Widget& widget, Key key, Action action)
{
  if (action != PRESSED)
    return;

  switch (key)
  {
    case KEY_UP:
    case KEY_RIGHT:
      setValue(value + stepSize, true);
      break;
    case KEY_DOWN:
    case KEY_LEFT:
      setValue(value - stepSize, true);
      break;
    case KEY_HOME:
      setValue(minValue, true);
      break;
    case KEY_END:
      setValue(maxValue, true);
      break;
    default:
      break;
  }
}

void Slider::onScroll(Widget& widget, vec2 offset)
{
  if (orientation == HORIZONTAL)
    setValue(value + float(offset.x) * stepSize, true);
  else
    setValue(value + float(offset.y) * stepSize, true);
}

void Slider::onDragMoved(Widget& widget, vec2 position)
{
  setValue(transformToLocal(position));
}

void Slider::setValue(const vec2& position)
{
  const float width = getWidth();
  const float height = getHeight();

  float fraction;

  if (orientation == HORIZONTAL)
    fraction = clamp((position.x - height / 2.f) / (width - height), 0.f, 1.f);
  else
    fraction = clamp((position.y - width / 2.f) / (height - width), 0.f, 1.f);

  setValue(minValue + (maxValue - minValue) * fraction, true);
}

void Slider::setValue(float newValue, bool notify)
{
  newValue = clamp(newValue, minValue, maxValue);
  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal(*this);

  invalidate();
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
