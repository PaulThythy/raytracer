#ifndef HITABLEOBJECT_H
#define HITABLEOBJECT_H

#include "Ray.h"

namespace Hitable {
	
	struct HitableObject {
		virtual ~HitableObject() = default;
		virtual bool intersect(const Ray::Ray& ray, float& t) const = 0;
		virtual glm::vec3 getNormal(const glm::vec3& point) const = 0;
	};
}

#endif