#ifndef ___BANSHEE_GEOMETRY_HPP___
#define ___BANSHEE_GEOMETRY_HPP___

#include "general.hpp"

class Geometry : public Object
{
private:
	ptr<VertexBuffer> vertexBuffer;
	ptr<IndexBuffer> indexBuffer;

public:
	Geometry(ptr<VertexBuffer> vertexBuffer, ptr<IndexBuffer> indexBuffer);

	ptr<VertexBuffer> GetVertexBuffer() const;
	ptr<IndexBuffer> GetIndexBuffer() const;

	META_DECLARE_CLASS(Geometry);
};

#endif
