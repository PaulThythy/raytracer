#ifndef SCENE_H
#define SCENE_H

#include "math/HitableObject.h"

#include <memory>
#include <vector>
#include <iostream>

namespace Scene {
	struct Scene {
		std::vector<std::shared_ptr<Hitable::HitableObject>> m_objects;

		Scene() {}

		void addObject(const std::shared_ptr<Hitable::HitableObject>& object) {
			m_objects.push_back(object);
		}

		bool intersect(const Ray::Ray& ray, float& t, std::shared_ptr<Hitable::HitableObject>& hitObject) const {
			bool hitAnything = false;
			double closestSoFar = t;

			for (const auto& object : m_objects) {
				float tempT;
				if (object->intersect(ray, tempT) && tempT < closestSoFar) {
					hitAnything = true;
					closestSoFar = tempT;
					hitObject = object;
				}
			}

			t = closestSoFar;
			return hitAnything;
		}
	};
}

#endif