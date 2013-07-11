#ifndef __KD_TREE_H__
#define __KD_TREE_H__
#include "c4d.h"
#include "c4d_tools.h"
#include "ge_dynamicarray.h"

class KDNode{
	public:
		~KDNode();
		void initInterior(Real splitPos);
		void initLeaf(const LONG *m_points, LONG m_numPoints);
		LONG getNearestNeighbor(const GeDynamicArray<Vector> &points, const Vector &point, const GeDynamicArray<LONG> &validPoints, Real &dist, INT depth);
		Bool m_interior;
		KDNode *m_rChild;
		union{
			KDNode *m_lChild;
			const LONG *m_points;
		};
		union{
			Real m_splitPos;
			LONG m_numPoints;
		};
};

void buildKDTree(const GeDynamicArray<Vector> &points, KDNode **nodes, Random &rng);

#endif