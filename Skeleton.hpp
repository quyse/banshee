#ifndef ___BANSHEE_SKELETON_HPP___
#define ___BANSHEE_SKELETON_HPP___

#include "general.hpp"

/// Класс скелета.
/** Содержит иерархию костей. */
class Skeleton : public Object
{
public:
	/// Структура кости.
	struct Bone
	{
		/// Оригинальная мировая ориентация.
		quat originalWorldOrientation;
		/// Оригинальная мировая позицця.
		vec3 originalWorldPosition;
		/// Относительная мировая позиция.
		vec3 originalRelativePosition;
		/// Номер родительской кости.
		int parent;
	};

public:
	/// Кости.
	std::vector<Bone> bones;
	/// Порядок топологической сортировки для костей.
	std::vector<int> sortedBones;

public:
	Skeleton(const std::vector<Bone>& bones);

	const std::vector<Bone>& GetBones() const;
	const std::vector<int>& GetSortedBones() const;

	static ptr<Skeleton> Deserialize(ptr<InputStream> inputStream);

	META_DECLARE_CLASS(Skeleton);
};

#endif
