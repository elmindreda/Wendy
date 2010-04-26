///////////////////////////////////////////////////////////////////////
// Wendy core library
// Copyright (c) 2005 Camilla Berglund <elmindreda@elmindreda.org>
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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

///////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

float log2f(float x)
{
  return (float) std::log(x) / (float) std::log(2.f);
}

float fminf(float x, float y)
{
  if (x < y)
    return x;
  else
    return y;
}

float fmaxf(float x, float y)
{
  if (x > y)
    return x;
  else
    return y;
}

#endif /*_MSC_VER*/

///////////////////////////////////////////////////////////////////////

#if !WENDY_HAVE_VASPRINTF

int vasprintf(char** result, const char* format, va_list vl)
{
  char buffer[65536];

  if (vsnprintf(buffer, sizeof(buffer), format, vl) < 0)
    buffer[sizeof(buffer) - 1] = '\0';

  size_t length = std::strlen(buffer);
  *result = (char*) std::malloc(length + 1);
  std::strcpy(*result, buffer);

  return (int) length;
}

#endif /*WENDY_HAVE_VASPRINTF*/

#if !WENDY_HAVE_STRTOF

float strtof(const char* nptr, char** endptr)
{
  return (float) std::strtod(nptr, endptr);
}

#endif /*WENDY_HAVE_STRTOF*/

#if !WENDY_HAVE_STRLCAT

size_t strlcat(char* target, const char* source, size_t size)
{
  size_t target_length, source_length, append_length;

  target_length = std::strlen(target);
  source_length = std::strlen(source);

  if (target_length >= size)
    return size;

  if (source_length == 0)
    return target_length;

  append_length = size - (target_length + 1);
  if (append_length > source_length)
    append_length = source_length;

  if (append_length > 0)
    std::strncat(target, source, append_length);

  return target_length + source_length;
}

#endif /*WENDY_HAVE_STRLCAT*/

#if !WENDY_HAVE_STRLCPY

size_t strlcpy(char* target, const char* source, size_t size)
{
  size_t source_length, copy_length;

  source_length = std::strlen(source);

  if (size == 0)
    return source_length;

  if (source_length < size - 1)
    copy_length = source_length;
  else
    copy_length = size - 1;

  std::strncpy(target, source, copy_length);

  if (copy_length >= size)
    target[size - 1] = '\0';

  return source_length;
}

#endif /*WENDY_HAVE_STRLCPY*/

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

Exception::Exception(const char* initMessage):
  message(initMessage)
{
}

const char* Exception::what(void) const throw()
{
  return message;
}

///////////////////////////////////////////////////////////////////////

bool RefBase::unreferenced(RefObject* object)
{
  return object->count == 0;
}

void RefBase::increment(RefObject* object)
{
  object->count++;
}

void RefBase::decrement(RefObject* object)
{
  object->count--;
}

///////////////////////////////////////////////////////////////////////

RefObject::RefObject(void):
  count(0)
{
}

RefObject::RefObject(const RefObject& source):
  count(0)
{
}

RefObject::~RefObject(void)
{
}

RefObject& RefObject::operator = (const RefObject& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

Log::~Log(void)
{
}

void Log::writeError(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(ERROR, "Error: %s", message);
  else
    fprintf(stderr, "Error: %s\n", message);

  std::free(message);
}

void Log::writeWarning(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(WARNING, "Warning: %s", message);
  else
    fprintf(stderr, "Warning: %s\n", message);

  std::free(message);
}

void Log::write(const char* format, ...)
{
  va_list vl;
  char* message;
  int result;

  va_start(vl, format);
  result = vasprintf(&message, format, vl);
  va_end(vl);

  if (result < 0)
    return;

  if (Log* log = Log::get())
    log->write(INFORMATION, "%s", message);
  else
    fprintf(stderr, "%s\n", message);

  std::free(message);
}

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
