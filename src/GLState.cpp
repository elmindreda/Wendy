///////////////////////////////////////////////////////////////////////
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

#include <wendy/OpenGL.h>
#include <wendy/GLTexture.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLContext.h>
#include <wendy/GLState.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <internal/GLConvert.h>

#include <algorithm>
#include <cstring>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {

///////////////////////////////////////////////////////////////////////

namespace
{

GLenum convertToGL(CullMode mode)
{
  switch (mode)
  {
    case CULL_FRONT:
      return GL_FRONT;
    case CULL_BACK:
      return GL_BACK;
    case CULL_BOTH:
      return GL_FRONT_AND_BACK;
  }

  logError("Invalid cull mode %u", mode);
  return 0;
}

CullMode invertCullMode(CullMode mode)
{
  switch (mode)
  {
    case CULL_NONE:
      return CULL_BOTH;
    case CULL_FRONT:
      return CULL_BACK;
    case CULL_BACK:
      return CULL_FRONT;
    case CULL_BOTH:
      return CULL_NONE;
  }

  logError("Invalid cull mode %u", mode);
  return (CullMode) 0;
}

GLenum convertToGL(BlendFactor factor)
{
  switch (factor)
  {
    case BLEND_ZERO:
      return GL_ZERO;
    case BLEND_ONE:
      return GL_ONE;
    case BLEND_SRC_COLOR:
      return GL_SRC_COLOR;
    case BLEND_DST_COLOR:
      return GL_DST_COLOR;
    case BLEND_SRC_ALPHA:
      return GL_SRC_ALPHA;
    case BLEND_DST_ALPHA:
      return GL_DST_ALPHA;
    case BLEND_ONE_MINUS_SRC_COLOR:
      return GL_ONE_MINUS_SRC_COLOR;
    case BLEND_ONE_MINUS_DST_COLOR:
      return GL_ONE_MINUS_DST_COLOR;
    case BLEND_ONE_MINUS_SRC_ALPHA:
      return GL_ONE_MINUS_SRC_ALPHA;
    case BLEND_ONE_MINUS_DST_ALPHA:
      return GL_ONE_MINUS_DST_ALPHA;
  }

  logError("Invalid blend factor %u", factor);
  return 0;
}

GLenum convertToGL(Function function)
{
  switch (function)
  {
    case ALLOW_NEVER:
      return GL_NEVER;
    case ALLOW_ALWAYS:
      return GL_ALWAYS;
    case ALLOW_EQUAL:
      return GL_EQUAL;
    case ALLOW_NOT_EQUAL:
      return GL_NOTEQUAL;
    case ALLOW_LESSER:
      return GL_LESS;
    case ALLOW_LESSER_EQUAL:
      return GL_LEQUAL;
    case ALLOW_GREATER:
      return GL_GREATER;
    case ALLOW_GREATER_EQUAL:
      return GL_GEQUAL;
  }

  logError("Invalid comparison function %u", function);
  return 0;
}

GLenum convertToGL(Operation operation)
{
  switch (operation)
  {
    case OP_KEEP:
      return GL_KEEP;
    case OP_ZERO:
      return GL_ZERO;
    case OP_REPLACE:
      return GL_REPLACE;
    case OP_INCREASE:
      return GL_INCR;
    case OP_DECREASE:
      return GL_DECR;
    case OP_INVERT:
      return GL_INVERT;
    case OP_INCREASE_WRAP:
      return GL_INCR_WRAP;
    case OP_DECREASE_WRAP:
      return GL_DECR_WRAP;
  }

  logError("Invalid stencil operation %u", operation);
  return 0;
}

bool samplerTypeMatchesTextureType(Sampler::Type samplerType, TextureType textureType)
{
  return (int) samplerType == (int) textureType;
}

} /*namespace*/

///////////////////////////////////////////////////////////////////////

void StencilState::apply(void) const
{
  if (cache.dirty)
  {
    force();
    return;
  }

  if (data.enabled)
  {
    if (!cache.enabled)
    {
      glEnable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }

    if (data.function != cache.function ||
        data.reference != cache.reference ||
        data.writeMask != cache.writeMask)
    {
      glStencilFunc(convertToGL(data.function), data.reference, data.writeMask);

      cache.function = data.function;
      cache.reference = data.reference;
      cache.writeMask = data.writeMask;
    }

    if (data.stencilFailed != cache.stencilFailed ||
        data.depthFailed != cache.depthFailed ||
        data.depthPassed != cache.depthPassed)
    {
      glStencilOp(convertToGL(data.stencilFailed),
                  convertToGL(data.depthFailed),
		  convertToGL(data.depthPassed));

      cache.stencilFailed = data.stencilFailed;
      cache.depthFailed = data.depthFailed;
      cache.depthPassed = data.depthPassed;
    }
  }
  else
  {
    if (cache.enabled)
    {
      glDisable(GL_STENCIL_TEST);
      cache.enabled = data.enabled;
    }
  }

#if WENDY_DEBUG
  checkGL("Error when applying stencil state");
#endif

  data.dirty = false;
}

bool StencilState::isEnabled(void) const
{
  return data.enabled;
}

Function StencilState::getFunction(void) const
{
  return data.function;
}

Operation StencilState::getStencilFailOperation(void) const
{
  return data.stencilFailed;
}

Operation StencilState::getDepthFailOperation(void) const
{
  return data.depthFailed;
}

Operation StencilState::getDepthPassOperation(void) const
{
  return data.depthPassed;
}

unsigned int StencilState::getReference(void) const
{
  return data.reference;
}

unsigned int StencilState::getWriteMask(void) const
{
  return data.writeMask;
}

void StencilState::setEnabled(bool newState)
{
  data.enabled = newState;
  data.dirty = true;
}

void StencilState::setFunction(Function newFunction)
{
  data.function = newFunction;
  data.dirty = true;
}

void StencilState::setReference(unsigned int newReference)
{
  data.reference = newReference;
  data.dirty = true;
}

void StencilState::setWriteMask(unsigned int newMask)
{
  data.writeMask = newMask;
  data.dirty = true;
}

void StencilState::setOperations(Operation stencilFailed,
                                 Operation depthFailed,
                                 Operation depthPassed)
{
  data.stencilFailed = stencilFailed;
  data.depthFailed = depthFailed;
  data.depthPassed = depthPassed;
  data.dirty = true;
}

void StencilState::setDefaults(void)
{
  data.setDefaults();
}

void StencilState::force(void) const
{
  cache = data;

  if (data.enabled)
    glEnable(GL_STENCIL_TEST);
  else
    glDisable(GL_STENCIL_TEST);

  glStencilFunc(data.function, data.reference, data.writeMask);
  glStencilOp(data.stencilFailed, data.depthFailed, data.depthPassed);

#if WENDY_DEBUG
  checkGL("Error when forcing stencil state");
#endif

  cache.dirty = data.dirty = false;
}

StencilState::Data StencilState::cache;

///////////////////////////////////////////////////////////////////////

StencilState::Data::Data(void)
{
  setDefaults();
}

void StencilState::Data::setDefaults(void)
{
  dirty = true;
  enabled = false;
  function = ALLOW_ALWAYS;
  reference = 0;
  writeMask = ~0;
  stencilFailed = OP_KEEP;
  depthFailed = OP_KEEP;
  depthPassed = OP_KEEP;
}

///////////////////////////////////////////////////////////////////////

ProgramState::ProgramState(void)
{
  if (usedIDs.empty())
    ID = nextID++;
  else
  {
    ID = usedIDs.back();
    usedIDs.pop_back();
  }
}

ProgramState::~ProgramState(void)
{
  usedIDs.push_front(ID);
}

void ProgramState::apply(void) const
{
  if (!program)
  {
    logError("Applying program state with no program set");
    return;
  }

  Context& context = program->getContext();
  context.setCurrentProgram(program);

  SharedProgramState* state = context.getCurrentSharedProgramState();

  unsigned int textureUnit = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
    {
      if (state)
        state->updateTo(sampler);
      else
        logError("Applying shared sampler \'%s\' of program \'%s\' without a current shared program state",
                 sampler.getName().c_str(),
                 program->getPath().asString().c_str());
    }
    else
    {
      context.setActiveTextureUnit(textureUnit);
      context.setCurrentTexture(textures[textureUnit]);
      sampler.bind(textureUnit);
      textureUnit++;
    }
  }

  size_t offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
    {
      if (state)
        state->updateTo(uniform);
      else
        logError("Applying shared uniform \'%s\' of program \'%s\' without a current shared program state",
                 uniform.getName().c_str(),
                 program->getPath().asString().c_str());
    }
    else
    {
      uniform.copyFrom(&floats[0] + offset);
      offset += uniform.getElementCount();
    }
  }
}

void ProgramState::getUniformState(const String& name, float& result) const
{
  std::memcpy(&result, getData(name, Uniform::FLOAT), sizeof(result));
}

void ProgramState::getUniformState(const String& name, vec2& result) const
{
  std::memcpy(&result, getData(name, Uniform::VEC2), sizeof(result));
}

void ProgramState::getUniformState(const String& name, vec3& result) const
{
  std::memcpy(&result, getData(name, Uniform::VEC3), sizeof(result));
}

void ProgramState::getUniformState(const String& name, vec4& result) const
{
  std::memcpy(&result, getData(name, Uniform::VEC4), sizeof(result));
}

void ProgramState::getUniformState(const String& name, mat2& result) const
{
  std::memcpy(&result, getData(name, Uniform::MAT2), sizeof(result));
}

void ProgramState::getUniformState(const String& name, mat3& result) const
{
  std::memcpy(&result, getData(name, Uniform::MAT3), sizeof(result));
}

void ProgramState::getUniformState(const String& name, mat4& result) const
{
  std::memcpy(&result, getData(name, Uniform::MAT4), sizeof(result));
}

void ProgramState::setUniformState(const String& name, float newValue)
{
  std::memcpy(getData(name, Uniform::FLOAT), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const vec2& newValue)
{
  std::memcpy(getData(name, Uniform::VEC2), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const vec3& newValue)
{
  std::memcpy(getData(name, Uniform::VEC3), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const vec4& newValue)
{
  std::memcpy(getData(name, Uniform::VEC4), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const mat2& newValue)
{
  std::memcpy(getData(name, Uniform::MAT2), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const mat3& newValue)
{
  std::memcpy(getData(name, Uniform::MAT3), &newValue, sizeof(newValue));
}

void ProgramState::setUniformState(const String& name, const mat4& newValue)
{
  std::memcpy(getData(name, Uniform::MAT4), &newValue, sizeof(newValue));
}

Texture* ProgramState::getSamplerState(const String& name) const
{
  if (!program)
  {
    logError("Cannot retrieve sampler state on program state with no program");
    return NULL;
  }

  unsigned int textureUnit = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    const Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.getName() == name)
      return textures[textureUnit];

    textureUnit++;
  }

  logError("Program \'%s\' has no sampler named \'%s\'",
           program->getPath().asString().c_str(),
           name.c_str());
  return NULL;
}

void ProgramState::setSamplerState(const String& name, Texture* newTexture)
{
  if (!program)
  {
    logError("Cannot set sampler state on program state with no program");
    return;
  }

  unsigned int textureUnit = 0;

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    Sampler& sampler = program->getSampler(i);
    if (sampler.isShared())
      continue;

    if (sampler.getName() == name)
    {
      if (newTexture)
      {
        if (samplerTypeMatchesTextureType(sampler.getType(), newTexture->getType()))
          textures[textureUnit] = newTexture;
        else
          logError("Type mismatch between sampler \'%s\' and texture \'%s\'",
                   sampler.getName().c_str(),
                   newTexture->getPath().asString().c_str());
      }
      else
        textures[textureUnit] = NULL;

      return;
    }

    textureUnit++;
  }
}

Program* ProgramState::getProgram(void) const
{
  return program;
}

void ProgramState::setProgram(Program* newProgram)
{
  floats.clear();
  textures.clear();

  program = newProgram;
  if (!program)
    return;

  unsigned int floatCount = 0;
  unsigned int textureCount = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    Uniform& uniform = program->getUniform(i);
    if (!uniform.isShared())
      floatCount += uniform.getElementCount();
  }

  for (unsigned int i = 0;  i < program->getSamplerCount();  i++)
  {
    Sampler& sampler = program->getSampler(i);
    if (!sampler.isShared())
      textureCount++;
  }

  floats.insert(floats.end(), floatCount, 0.f);
  textures.resize(textureCount);
}

StateID ProgramState::getID(void) const
{
  return ID;
}

void ProgramState::setDefaults(void)
{
  setProgram(NULL);
}

void* ProgramState::getData(const String& name, Uniform::Type type)
{
  if (!program)
  {
    logError("Cannot set uniform state on program state with no program");
    return NULL;
  }

  unsigned int offset = 0;

  for (unsigned int i = 0;  i < program->getUniformCount();  i++)
  {
    Uniform& uniform = program->getUniform(i);
    if (uniform.isShared())
      continue;

    if (uniform.getName() == name)
    {
      if (uniform.getType() == type)
        return &floats[0] + offset;

      logError("Uniform \'%s\' of program \'%s\' is not of type \'%s\'",
               uniform.getName().c_str(),
               program->getPath().asString().c_str(),
               asString(type));
      return NULL;
    }

    offset += uniform.getElementCount();
  }

  logError("Program \'%s\' has no uniform named \'%s\'",
           program->getPath().asString().c_str(),
           name.c_str());
  return NULL;
}

const void* ProgramState::getData(const String& name, Uniform::Type type) const
{
  // TODO: The code.
  return NULL;
}

ProgramState::IDQueue ProgramState::usedIDs;

StateID ProgramState::nextID = 0;

///////////////////////////////////////////////////////////////////////

void RenderState::apply(void) const
{
  if (Context* context = Context::getSingleton())
  {
    if (Stats* stats = context->getStats())
      stats->addPasses(1);
  }

  if (cache.dirty)
  {
    force();
    return;
  }

  CullMode cullMode = data.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  if (cullMode != cache.cullMode)
  {
    if ((cullMode == CULL_NONE) != (cache.cullMode == CULL_NONE))
      setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);

    if (cullMode != CULL_NONE)
      glCullFace(convertToGL(cullMode));

    cache.cullMode = cullMode;
  }

  if (data.srcFactor != cache.srcFactor || data.dstFactor != cache.dstFactor)
  {
    setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);

    if (data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO)
      glBlendFunc(convertToGL(data.srcFactor), convertToGL(data.dstFactor));

    cache.srcFactor = data.srcFactor;
    cache.dstFactor = data.dstFactor;
  }

  if (data.depthTesting || data.depthWriting)
  {
    // Set depth buffer writing.
    if (data.depthWriting != cache.depthWriting)
      glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);

    if (data.depthTesting)
    {
      // Set depth buffer function.
      if (data.depthFunction != cache.depthFunction)
      {
        glDepthFunc(convertToGL(data.depthFunction));
        cache.depthFunction = data.depthFunction;
      }
    }
    else if (data.depthWriting)
    {
      // NOTE: Special case; depth buffer filling.
      //       Set specific depth buffer function.
      const Function depthFunction = ALLOW_ALWAYS;

      if (cache.depthFunction != depthFunction)
      {
        glDepthFunc(convertToGL(depthFunction));
        cache.depthFunction = depthFunction;
      }
    }

    if (!(cache.depthTesting || cache.depthWriting))
      glEnable(GL_DEPTH_TEST);
  }
  else
  {
    if (cache.depthTesting || cache.depthWriting)
      glDisable(GL_DEPTH_TEST);
  }

  cache.depthTesting = data.depthTesting;
  cache.depthWriting = data.depthWriting;

  if (data.colorWriting != cache.colorWriting)
  {
    const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
    glColorMask(state, state, state, state);
    cache.colorWriting = data.colorWriting;
  }

  if (data.wireframe != cache.wireframe)
  {
    const GLenum state = data.wireframe ? GL_LINE : GL_FILL;
    glPolygonMode(GL_FRONT_AND_BACK, state);
    cache.wireframe = data.wireframe;
  }

  if (data.lineSmoothing != cache.lineSmoothing)
  {
    setBooleanState(GL_LINE_SMOOTH, data.lineSmoothing);
    cache.lineSmoothing = data.lineSmoothing;
  }

  if (data.lineWidth != cache.lineWidth)
  {
    glLineWidth(data.lineWidth);
    cache.lineWidth = data.lineWidth;
  }

#if WENDY_DEBUG
  checkGL("Error when applying render state");
#endif

  ProgramState::apply();

  data.dirty = false;
}

bool RenderState::isCulling(void) const
{
  return data.cullMode != CULL_NONE;
}

bool RenderState::isBlending(void) const
{
  return data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO;
}

bool RenderState::isDepthTesting(void) const
{
  return data.depthTesting;
}

bool RenderState::isDepthWriting(void) const
{
  return data.depthWriting;
}

bool RenderState::isColorWriting(void) const
{
  return data.colorWriting;
}

bool RenderState::isWireframe(void) const
{
  return data.wireframe;
}

bool RenderState::isLineSmoothing(void) const
{
  return data.lineSmoothing;
}

float RenderState::getLineWidth(void) const
{
  return data.lineWidth;
}

CullMode RenderState::getCullMode(void) const
{
  return data.cullMode;
}

BlendFactor RenderState::getSrcFactor(void) const
{
  return data.srcFactor;
}

BlendFactor RenderState::getDstFactor(void) const
{
  return data.dstFactor;
}

Function RenderState::getDepthFunction(void) const
{
  return data.depthFunction;
}

void RenderState::setDepthTesting(bool enable)
{
  data.depthTesting = enable;
  data.dirty = true;
}

void RenderState::setDepthWriting(bool enable)
{
  data.depthWriting = enable;
  data.dirty = true;
}

void RenderState::setCullMode(CullMode mode)
{
  data.cullMode = mode;
  data.dirty = true;
}

void RenderState::setBlendFactors(BlendFactor src, BlendFactor dst)
{
  data.srcFactor = src;
  data.dstFactor = dst;
  data.dirty = true;
}

void RenderState::setDepthFunction(Function function)
{
  data.depthFunction = function;
  data.dirty = true;
}

void RenderState::setColorWriting(bool enabled)
{
  data.colorWriting = enabled;
  data.dirty = true;
}

void RenderState::setWireframe(bool enabled)
{
  data.wireframe = enabled;
  data.dirty = true;
}

void RenderState::setLineSmoothing(bool enabled)
{
  data.lineSmoothing = enabled;
  data.dirty = true;
}

void RenderState::setLineWidth(float newWidth)
{
  data.lineWidth = newWidth;
  data.dirty = true;
}

void RenderState::setDefaults(void)
{
  data.setDefaults();
}

bool RenderState::isCullingInverted(void)
{
  return cullingInverted;
}

void RenderState::setCullingInversion(bool newState)
{
  cullingInverted = newState;
}

void RenderState::force(void) const
{
  cache = data;

  CullMode cullMode = data.cullMode;
  if (cullingInverted)
    cullMode = invertCullMode(cullMode);

  setBooleanState(GL_CULL_FACE, cullMode != CULL_NONE);
  if (cullMode != CULL_NONE)
    glCullFace(convertToGL(cullMode));

  setBooleanState(GL_BLEND, data.srcFactor != BLEND_ONE || data.dstFactor != BLEND_ZERO);
  glBlendFunc(convertToGL(data.srcFactor), convertToGL(data.dstFactor));

  glDepthMask(data.depthWriting ? GL_TRUE : GL_FALSE);
  setBooleanState(GL_DEPTH_TEST, data.depthTesting || data.depthWriting);

  if (data.depthWriting && !data.depthTesting)
  {
    const Function depthFunction = ALLOW_ALWAYS;
    glDepthFunc(convertToGL(depthFunction));
    cache.depthFunction = depthFunction;
  }
  else
    glDepthFunc(convertToGL(data.depthFunction));

  const GLboolean state = data.colorWriting ? GL_TRUE : GL_FALSE;
  glColorMask(state, state, state, state);

  const GLenum polygonMode = data.wireframe ? GL_LINE : GL_FILL;
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

  setBooleanState(GL_LINE_SMOOTH, data.lineSmoothing);
  glLineWidth(data.lineWidth);

#if WENDY_DEBUG
  checkGL("Error when forcing render state");
#endif

  ProgramState::apply();

  cache.dirty = data.dirty = false;
}

void RenderState::setBooleanState(unsigned int state, bool value) const
{
  if (value)
    glEnable(state);
  else
    glDisable(state);
}

RenderState::Data RenderState::cache;

bool RenderState::cullingInverted = false;

///////////////////////////////////////////////////////////////////////

RenderState::Data::Data(void)
{
  setDefaults();
}

void RenderState::Data::setDefaults(void)
{
  dirty = true;
  depthTesting = true;
  depthWriting = true;
  colorWriting = true;
  wireframe = false;
  lineSmoothing = false;
  lineWidth = 1.f;
  cullMode = CULL_BACK;
  srcFactor = BLEND_ONE;
  dstFactor = BLEND_ZERO;
  depthFunction = ALLOW_LESSER;
}

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
