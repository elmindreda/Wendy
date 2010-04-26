#!/usr/bin/env ruby
#
# Wendy core library
# Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any
# damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any
# purpose, including commercial applications, and to alter it and
# redistribute it freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you
#     must not claim that you wrote the original software. If you use
#     this software in a product, an acknowledgment in the product
#     documentation would be appreciated but is not required.
#
#  2. Altered source versions must be plainly marked as such, and
#     must not be misrepresented as being the original software.
#
#  3. This notice may not be removed or altered from any source
#     distribution.
#

def make_declaration(count)
  declaration = ['typename R']
  count.times do |i|
    declaration << "typename A#{i+1}"
  end
  declaration = declaration.join(', ')
end

def output_file(file, count)
  file.print <<DONE
///////////////////////////////////////////////////////////////////////
// Wendy core library
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
#ifndef WENDY_SIGNAL_H
#define WENDY_SIGNAL_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////
// NOTE: This file is generated by #{File.basename($0)}. DO NOT EDIT.
///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

class SignalSlot;

DONE

  count.times do |i|
    file.print <<DONE
template <#{make_declaration(i)}>
class Signal#{i};
DONE
  end

  file.print <<DONE

////////////////////////////////////////////////////////////////////////

/*! @brief Mixin for automatic slot destruction.
 *
 *  @remarks Inherit from this to automatically be disconnected from all
 *  attached signals on destruction.
 */
class Trackable
{
  friend class SignalSlot;
public:
  Trackable(void);
  Trackable(const Trackable& source);
  virtual ~Trackable(void);
  Trackable& operator = (const Trackable& source);
private:
  typedef std::list<SignalSlot*> SlotList;
  SlotList slots;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Abstract base class for signal slots.
 *
 *  @remarks If you wish to keep track of a connection for explicit
 *  disconnection, you usually want to do it by keeping a pointer to a
 *  SignalSlot object.
 */
class SignalSlot
{
public:
  /*! Constructor.
   *  @param[in] object The trackable object this slot connects to, or @c
   *  NULL if it is connected to a function or a non-trackable object.
   */
  SignalSlot(Trackable* object = NULL);
  /*! Destructor.
   *  If this slot is attached to a trackable object, it will sever the
   *  connection to it.
   */
  virtual ~SignalSlot(void) = 0;
private:
  Trackable* object;
};

DONE

  (0...count).each do |i|
    output_number(file, count, i)
  end

  file.print <<DONE
///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_SIGNAL_H*/
///////////////////////////////////////////////////////////////////////
DONE
end

def output_number(file, count, index)

  if index > 0 then
    parameters = []
    index.times do |i|
      parameters << "A#{i+1} a#{i+1}"
    end
    parameters = parameters.join(', ')
  else
    parameters = 'void'
  end

  if parameters == 'void' then
    result_parameters = ''
  else
    result_parameters = ', ' + parameters
  end

  if index > 0 then
    arguments = []
    index.times do |i|
      arguments << "a#{i+1}"
    end
    arguments = arguments.join(', ')
  else
    arguments = ''
  end

  type = ['R']
  index.times do |i|
    type << "A#{i+1}"
  end
  type = type.join(',')

  declaration = make_declaration(index)

  file.print <<DONE
////////////////////////////////////////////////////////////////////////

/*! @brief Base class for typed signal slots.
 */
template <#{declaration}>
class SignalSlot#{index} : public SignalSlot
{
  friend class Signal#{index}<#{type}>;
public:
  /*! Destructor.
   */
  ~SignalSlot#{index}(void);
  /*! Calls the target for this slot.
   */
  virtual R emit(#{parameters}) = 0;
protected:
  /*! Constructor.
   */
  SignalSlot#{index}(Trackable* object = NULL);
private:
  Signal#{index}<#{type}>* signal;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal slot for functions and static methods.
 */
template <#{declaration}>
class FunctionSlot#{index} : public SignalSlot#{index}<#{type}>
{
public:
  /*! Function pointer type for this slot.
   */
  typedef R (*Function)(#{parameters});
  /*! Constructor.
   */
  inline FunctionSlot#{index}(Function function);
  /*! Calls the target for this slot.
   */
  inline R emit(#{parameters});
private:
  Function function;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal slot for member functions.
 */
template <typename T, #{declaration}>
class MethodSlot#{index} : public SignalSlot#{index}<#{type}>
{
public:
  /*! Method pointer type for this slot.
   */
  typedef R (T::*Function)(#{parameters});
  /*! Constructor.
   */
  inline MethodSlot#{index}(T& object, Function function);
  /*! Calls the target for this slot.
   */
  inline R emit(#{parameters});
private:
  T& object;
  Function function;
};

////////////////////////////////////////////////////////////////////////

/*! @brief Typed signal object.
 */
template <#{declaration}>
class Signal#{index}
{
  friend class SignalSlot#{index}<#{type}>;
public:
  /*! Slot base class for this signal.
   */
  typedef SignalSlot#{index}<#{type}> Slot;
  /*! Constructor.
   */
  inline Signal#{index}(void);
  /*! Copy contructor.
   *  @note This does not copy any existing connections to the source object.
   */
  inline Signal#{index}(const Signal#{index}<#{type}>& source);
  /*! Destructor.
   */
  inline ~Signal#{index}(void);
  /*! Adds the specified generic slot to this signal.
   */
  inline Slot* connect(Slot* slot);
  /*! Connects the specified instance and member function to this signal.
   */
  template <typename T>
  inline Slot* connect(T& object, typename MethodSlot#{index}<T,#{type}>::Function function)
  { return connect(new MethodSlot#{index}<T,#{type}>(object, function)); }
  /*! Connects the specified function to this signal.
   */
  inline Slot* connect(typename FunctionSlot#{index}<#{type}>::Function function);
  /*! Calls the targets for all slots in this signal.
   */
  inline void emit(#{parameters}) const;
  /*! Calls the targets for all slots in this signal and stores the return values.
   */
  inline void emit(std::list<R>& results#{result_parameters}) const;
  /*! Assignment operator.
   *  @note This does not copy any existing connections to the source object.
   */
  inline Signal#{index}<#{type}>& operator = (const Signal#{index}<#{type}>& source);
private:
  typedef std::list<Slot*> SlotList;
  SlotList slots;
};

///////////////////////////////////////////////////////////////////////

/*! Proxy object for signal objects.
 *  Allows public connections to private signal objects.
 *  Concept taken from gtkmm.
 */
template <#{declaration}>
class SignalProxy#{index}
{
public:
  /*! Signal type.
   */
  typedef Signal#{index}<#{type}> Signal;
  /*! Slot type.
   */
  typedef SignalSlot#{index}<#{type}> Slot;
  /*! Constructor.
   */
  inline SignalProxy#{index}(Signal& signal);
  /*! Connects the specified slot to the proxied signal.
   */
  inline Slot* connect(Slot* slot);
  /*! Connects the specified function to the proxied signal.
   */
  inline Slot* connect(typename FunctionSlot#{index}<#{type}>::Function function);
  /*! Connects the specified instance and member function to the proxied signal.
   */
  template <typename T>
  inline Slot* connect(T& object, typename MethodSlot#{index}<T,#{type}>::Function function)
  { return signal.connect(new MethodSlot#{index}<T,#{type}>(object, function)); }
private:
  Signal& signal;
};

////////////////////////////////////////////////////////////////////////

template <#{declaration}>
SignalSlot#{index}<#{type}>::~SignalSlot#{index}(void)
{
  if (signal)
    signal->slots.remove(this);
}

template <#{declaration}>
SignalSlot#{index}<#{type}>::SignalSlot#{index}(Trackable* object):
  SignalSlot(object)
{
}

////////////////////////////////////////////////////////////////////////

template <#{declaration}>
inline FunctionSlot#{index}<#{type}>::FunctionSlot#{index}(Function initFunction):
  function(initFunction)
{
}

template <#{declaration}>
inline R FunctionSlot#{index}<#{type}>::emit(#{parameters})
{
  return (*function)(#{arguments});
}

////////////////////////////////////////////////////////////////////////

template <typename T, #{declaration}>
inline MethodSlot#{index}<T,#{type}>::MethodSlot#{index}(T& initObject, Function initFunction):
  SignalSlot#{index}<#{type}>(dynamic_cast<Trackable*>(&initObject)),
  object(initObject),
  function(initFunction)
{
}

template <typename T, #{declaration}>
inline R MethodSlot#{index}<T,#{type}>::emit(#{parameters})
{
  return (object.*function)(#{arguments});
}

////////////////////////////////////////////////////////////////////////

template <#{declaration}>
inline Signal#{index}<#{type}>::Signal#{index}(void)
{
}

template <#{declaration}>
inline Signal#{index}<#{type}>::Signal#{index}(const Signal#{index}<#{type}>& source)
{
}

template <#{declaration}>
inline Signal#{index}<#{type}>::~Signal#{index}(void)
{
  while (!slots.empty())
    delete slots.front();
}

template <#{declaration}>
inline SignalSlot#{index}<#{type}>* Signal#{index}<#{type}>::connect(Slot* slot)
{
  slots.push_front(slot);
  slot->signal = this;
  return slot;
}

template <#{declaration}>
inline SignalSlot#{index}<#{type}>* Signal#{index}<#{type}>::connect(typename FunctionSlot#{index}<#{type}>::Function function)
{
  return connect(new FunctionSlot#{index}<#{type}>(function));
}

template <#{declaration}>
inline void Signal#{index}<#{type}>::emit(#{parameters}) const
{
  for (typename SlotList::const_iterator i = slots.begin();  i != slots.end();  i++)
    (*i)->emit(#{arguments});
}

template <#{declaration}>
inline void Signal#{index}<#{type}>::emit(std::list<R>& results#{result_parameters}) const
{
  results.clear();
  for (typename SlotList::const_iterator i = slots.begin();  i != slots.end();  i++)
    results.push_back((*i)->emit(#{arguments}));
}

template <#{declaration}>
inline Signal#{index}<#{type}>& Signal#{index}<#{type}>::operator = (const Signal#{index}<#{type}>& source)
{
  return *this;
}

///////////////////////////////////////////////////////////////////////

template <#{declaration}>
inline SignalProxy#{index}<#{type}>::SignalProxy#{index}(Signal& initSignal):
  signal(initSignal)
{
}

template <#{declaration}>
inline SignalSlot#{index}<#{type}>*
SignalProxy#{index}<#{type}>::connect(Slot* slot)
{
  return signal.connect(slot);
}

template <#{declaration}>
inline SignalSlot#{index}<#{type}>*
SignalProxy#{index}<#{type}>::connect(typename FunctionSlot#{index}<#{type}>::Function function)
{
  return signal.connect(function);
}

DONE

end

if ARGV.length != 2 then
  raise 'Invalid user'
end

path = ARGV[0]

begin
  file = File.new(path, 'w')
rescue
  raise "Unable to open output file #{path}"
end

output_file file, ARGV[1].to_i + 1

