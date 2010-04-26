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
#include <wendy/UISlider.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace UI
  {

///////////////////////////////////////////////////////////////////////

Slider::Slider(Desktop& desktop, Widget* parent, Orientation initOrientation):
  Widget(desktop, parent),
  minValue(0.f),
  maxValue(1.f),
  stepSize(1.f),
  value(0.f),
  orientation(initOrientation)
{
  const float em = desktop.getRenderer().getDefaultEM();

  if (orientation == HORIZONTAL)
    setSize(Vec2(em * 10.f, em * 1.5f));
  else
    setSize(Vec2(em * 1.5f, em * 10.f));

  getKeyPressedSignal().connect(*this, &Slider::onKeyPressed);
  getButtonClickedSignal().connect(*this, &Slider::onButtonClicked);
  getWheelTurnedSignal().connect(*this, &Slider::onWheelTurned);
  getDragMovedSignal().connect(*this, &Slider::onDragMoved);

  setDraggable(true);
}

float Slider::getMinValue(void) const
{
  return minValue;
}

float Slider::getMaxValue(void) const
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
}

float Slider::getValue(void) const
{
  return value;
}

void Slider::setValue(float newValue)
{
  setValue(newValue, false);
}

float Slider::getStepSize(void) const
{
  return stepSize;
}

void Slider::setStepSize(float newSize)
{
  stepSize = std::max(newSize, 0.f);
}

SignalProxy1<void, Slider&> Slider::getValueChangedSignal(void)
{
  return valueChangedSignal;
}

void Slider::draw(void) const
{
  const Rect& area = getGlobalArea();

  Renderer& renderer = getDesktop().getRenderer();
  if (renderer.pushClipArea(area))
  {
    renderer.drawWell(area, getState());

    const float position = (value - minValue) / (maxValue - minValue);

    const float em = renderer.getDefaultEM();

    Rect handleArea;

    if (orientation == HORIZONTAL)
    {
      handleArea.set(area.position.x + position * (area.size.x - em),
		     area.position.y,
		     em,
		     area.size.y);
    }
    else
    {
      handleArea.set(area.position.x,
		     area.position.y + position * (area.size.y - em),
		     area.size.x,
		     em);
    }

    renderer.drawHandle(handleArea, getState());

    Widget::draw();

    renderer.popClipArea();
  }
}

void Slider::onButtonClicked(Widget& widget,
			     const Vec2& position,
			     input::Button button,
			     bool clicked)
{
  if (clicked)
    setValue(transformToLocal(position));
}

void Slider::onKeyPressed(Widget& widget, input::Key key, bool pressed)
{
  if (pressed)
  {
    switch (key)
    {
      case input::Key::UP:
      case input::Key::RIGHT:
	setValue(value + stepSize, true);
	break;
      case input::Key::DOWN:
      case input::Key::LEFT:
	setValue(value - stepSize, true);
	break;
      case input::Key::HOME:
	setValue(minValue, true);
	break;
      case input::Key::END:
	setValue(maxValue, true);
	break;
    }
  }
}

void Slider::onWheelTurned(Widget& widget, int offset)
{
  setValue(value + offset * stepSize, true);
}

void Slider::onDragMoved(Widget& widget, const Vec2& position)
{
  setValue(transformToLocal(position));
}

void Slider::setValue(const Vec2& position)
{
  const float em = getDesktop().getRenderer().getDefaultEM();

  float scale;

  if (orientation == HORIZONTAL)
    scale = (position.x - em / 2.f) / (getArea().size.x - em);
  else
    scale = (position.y - em / 2.f) / (getArea().size.y - em);

  setValue(minValue + (maxValue - minValue) * scale, true);
}

void Slider::setValue(float newValue, bool notify)
{
  if (newValue < minValue)
    newValue = minValue;
  else if (newValue > maxValue)
    newValue = maxValue;

  if (newValue == value)
    return;

  value = newValue;

  if (notify)
    valueChangedSignal.emit(*this);
}

///////////////////////////////////////////////////////////////////////

  } /*namespace UI*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
