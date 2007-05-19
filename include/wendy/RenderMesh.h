///////////////////////////////////////////////////////////////////////
// Wendy default renderer
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
#ifndef WENDY_RENDERMESH_H
#define WENDY_RENDERMESH_H
///////////////////////////////////////////////////////////////////////

#include <list>

///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace render
  {
  
///////////////////////////////////////////////////////////////////////

/*! @brief Renderable mesh object.
 *  @ingroup renderer
 *
 *  This class represents a single static mesh, consisting of one or more
 *  Mesh::Geometry objects. Each geometry is a part of the mesh using a single
 *  render style and primitive mode.
 */
class Mesh : public Renderable,
             public DerivedResource<Mesh, moira::Mesh>,
	     public RefObject<Mesh>
{
public:
  class Geometry;
  typedef std::vector<Geometry*> GeometryList;
  ~Mesh(void);
  void enqueue(Queue& queue, const Transform3& transform) const;
  /*! @return The bounding sphere of this mesh.
   */
  const Sphere& getBounds(void) const;
  const GeometryList& getGeometries(void);
  /*! Creates a renderable mesh from the specified mesh data.
   *  @param[in] mesh The mesh data to use.
   *  @param[in] name The desired name of the created renderable mesh.
   *  @return The newly created renderable mesh, or @c NULL if an error occurred.
   */
  static Mesh* createInstance(const moira::Mesh& mesh, const String& name = "");
private:
  Mesh(const String& name);
  Mesh(const Mesh& source);
  Mesh& operator = (const Mesh& source);
  bool init(const moira::Mesh& mesh);
  GeometryList geometries;
  Ptr<GL::VertexBuffer> vertexBuffer;
  Ptr<GL::IndexBuffer> indexBuffer;
  Sphere bounds;
};

///////////////////////////////////////////////////////////////////////

/*! @brief %Renderable mesh subset.
 *  @ingroup renderer
 *
 *  This class represents a subset of a mesh, using a single render style
 *  and a single primitive mode.
 */
class Mesh::Geometry
{
  friend class Mesh;
public:
  /*! Constructor.
   */
  //Geometry(const GL::IndexRange& range, GLenum renderMode, Style* style);
  /*! @return The range of indices used by this geometry.
   */
  const GL::IndexRange& getIndexRange(void) const;
  /*! @return The primitive mode used by this geometry.
   */
  GLenum getRenderMode(void) const;
  /*! @return The render style used by this geometry.
   */
  Style* getStyle(void) const;
  void setStyle(Style* newStyle);
private:
  GL::IndexRange range;
  GLenum renderMode;
  Ref<Style> style;
};

///////////////////////////////////////////////////////////////////////

/*! @brief Shadow volume generator.
 *  @ingroup renderer
 *
 *  This class is used to generate shadow volumes from a given source triangle
 *  mesh. It is intended for use with static geometry.
 */
class ShadowMesh : public Renderable
{
public:
  void update(const Vector3 origin);
  void enqueue(Queue& queue, const Transform3& transform) const;
  float getExtrudeDistance(void) const;
  void setExtrudeDistance(float newDistance);
  static ShadowMesh* createInstance(const moira::Mesh& mesh);
private:
  class Edge;
  class Triangle;
  ShadowMesh(void);
  ShadowMesh(const ShadowMesh& source);
  ShadowMesh& operator = (const ShadowMesh& source);
  bool init(const moira::Mesh& mesh);
  typedef std::vector<Vector3> VertexList;
  typedef std::vector<Edge> EdgeList;
  typedef std::vector<Triangle> TriangleList;
  VertexList vertices;
  mutable VertexList extrudedVertices;
  TriangleList triangles;
  EdgeList edges;
  float distance;
  unsigned int vertexCount;
  Ptr<GL::VertexBuffer> vertexBuffer;
  Ref<Style> style;
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class ShadowMesh::Edge
{
public:
  unsigned int vertices[2];
  unsigned int triangles[2];
};

///////////////////////////////////////////////////////////////////////

/*! @internal
 */
class ShadowMesh::Triangle
{
public:
  enum Status
  {
    UNREFERENCED,
    FRONT_FACE,
    BACK_FACE,
  };
  unsigned int vertices[3];
  Vector3 normal;
  mutable Status status;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace render*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_RENDERMESH_H*/
///////////////////////////////////////////////////////////////////////
