#ifndef __KD_TREE_H__
#define __KD_TREE_H__
#include "c4d.h"
#include "c4d_tools.h"
#include "ge_dynamicarray.h"

class KDNode{
	public:
		~KDNode();
		void initInterior(Float splitPos);
		void initLeaf(const Int32 *m_points, Int32 m_numPoints);
		Int32 getNearestNeighbor(const GeDynamicArray<Vector> &points, const Vector &point, const GeDynamicArray<Int32> &validPoints, Float &dist, INT depth);
		Bool m_interior;
		KDNode *m_rChild;
		union{
			KDNode *m_lChild;
			const Int32 *m_points;
		};
		union{
			Float m_splitPos;
			Int32 m_numPoints;
		};
};

void buildKDTree(const GeDynamicArray<Vector> &points, KDNode **nodes, Random &rng);

#endif