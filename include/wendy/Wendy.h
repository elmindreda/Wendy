///////////////////////////////////////////////////////////////////////
// Wendy library
// Copyright (c) 2004 Camilla Berglund <elmindreda@elmindreda.org>
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
#ifndef WENDY_WENDY_H
#define WENDY_WENDY_H
///////////////////////////////////////////////////////////////////////

#include <moira/Moira.h>

#include <wendy/Config.h>

#include <wendy/Core.h>

#include <wendy/OpenGL.h>
#include <wendy/GLContext.h>
#include <wendy/GLTexture.h>
#include <wendy/GLCanvas.h>
#include <wendy/GLCamera.h>
#include <wendy/GLVertex.h>
#include <wendy/GLDisplayList.h>
#include <wendy/GLBuffer.h>
#include <wendy/GLProgram.h>
#include <wendy/GLLight.h>
#include <wendy/GLFont.h>
#include <wendy/GLRender.h>
#include <wendy/GLParticle.h>
#include <wendy/GLMesh.h>
#include <wendy/GLNode.h>
#include <wendy/GLSprite.h>
#include <wendy/GLDemo.h>

#include <wendy/UIWidget.h>

///////////////////////////////////////////////////////////////////////

namespace wendy
{

///////////////////////////////////////////////////////////////////////

/*! @brief Initializes libraries and creates the default objects.
 *
 *  Initializes lower-level libraries and sets up the default objects,
 *  such as instances of all default resource codecs.  You must call
 *  this before creating an OpenGL context.
 *
 *  @remarks If you do call it, it's considered polite to call
 *  wendy::shutdown as well.
 */
bool initialize(void);

/*! @brief Destroys the default objects.
 *
 *  Destroys all objects created by wendy::initialize.
 */
void shutdown(void);

///////////////////////////////////////////////////////////////////////

} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_WENDY_H*/
///////////////////////////////////////////////////////////////////////
