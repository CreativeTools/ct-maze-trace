#include "kd_tree.h"
#include "ge_sort.h"

class Sorter : public  GeSortAndSearch
{
	public:
		virtual LONG Compare(void* a, void* b)
		{
			Real aa = *((Real *)a);
			Real bb = *((Real *)b);
			if (aa<bb) return -1;
			if (aa>bb) return 1;
			return 0;
		}
};

KDNode::~KDNode()
{
	if(m_interior){
		GeFree(m_lChild);
		GeFree(m_rChild);
	}
	else{
		GeFree(m_points);
	}
}

void
KDNode::initInterior(Real splitPos)
{
	m_interior = TRUE;
	m_splitPos = splitPos;
	m_lChild = NULL;
	m_rChild = NULL;
}

void
KDNode::initLeaf(const LONG *points, LONG numPoints)
{
	m_interior = FALSE;
	m_points = points;
	m_numPoints = numPoints;
}

static
Real
getMedian(const GeDynamicArray<Vector> &points, const LONG *pnts, LONG numPoints, Random &rng, INT axis, Sorter &sorter)
{
	LONG maxSamples = 20;
	LONG numSamples = numPoints < maxSamples ? numPoints : maxSamples;
	Real* samples = GeAllocType(Real,numSamples);
	for(LONG i=0;i<numSamples;i++){
		Real val = rng.Get01();
		LONG a = val * (numPoints-1);
		switch(axis){
			case 0:
				samples[i] = points[pnts[a]].x;
				break;
			case 1:
				samples[i] = points[pnts[a]].y;
				break;
			case 2:
				samples[i] = points[pnts[a]].z;
				break;
		}
		
	}
	sorter.Sort(samples,numSamples,sizeof(Real));
	Real ret = samples[numSamples/2];
	GeFree(samples);
	return ret;
}

static
KDNode *
recursiveBuild(const GeDynamicArray<Vector> &points, const LONG *pnts, LONG numPoints, Random &rng, INT depth, Sorter &sorter)
{
	KDNode *node =GeAllocType(KDNode,1);
	LONG minNodeEntries = 4;
	LONG maxDepth = 10;
	if(numPoints > minNodeEntries && depth < maxDepth){
		INT axis = depth%3;
		Real splitPos = getMedian(points,pnts,numPoints,rng,axis,sorter);
		LONG numLeft = 0;
		for(LONG i=0;i<numPoints;i++){
			switch(axis){
				case 0:
					if(points[pnts[i]].x < splitPos){
						numLeft++;
					}
					break;
				case 1:
					if(points[pnts[i]].y < splitPos){
						numLeft++;
					}
					break;
				case 2:
					if(points[pnts[i]].z < splitPos){
						numLeft++;
					}
					break;
			}
			
		}
		LONG *lPoints = GeAllocType(LONG,numLeft);
		LONG *rPoints = GeAllocType(LONG,numPoints - numLeft);
		LONG lPos=0;
		LONG rPos=0;
		for(LONG i=0;i<numPoints;i++){
			Real val = 0;
			switch(axis){
				case 0:
					val = points[pnts[i]].x;
					break;
				case 1:
					val = points[pnts[i]].y;
					break;
				case 2:
					val = points[pnts[i]].z;
					break;
			}
			if(val < splitPos){
				lPoints[lPos] = pnts[i];
				lPos++;
			}
			else{
				rPoints[rPos] = pnts[i];
				rPos++;
			}
		}
		node->initInterior(splitPos);
		node->m_lChild = recursiveBuild(points,lPoints,numLeft,rng,depth+1,sorter);
		node->m_rChild = recursiveBuild(points,rPoints,numPoints-numLeft,rng,depth+1,sorter);
		GeFree(pnts);
	}
	else{
		node->initLeaf(pnts,numPoints);
	}
	return node;
}

void
buildKDTree(const GeDynamicArray<Vector> &points, KDNode **nodes, Random &rng)
{
	Sorter sorter;
	LONG *pnts = GeAllocType(LONG,points.GetCount());
	LONG numPoints = points.GetCount();
	for(LONG i=0;i<numPoints;i++){
		pnts[i] = i;
	}
	*nodes = recursiveBuild(points,pnts,numPoints,rng,0,sorter);
}

LONG
KDNode::getNearestNeighbor(const GeDynamicArray<Vector> &points, const Vector &point, const GeDynamicArray<LONG> &validPoints, Real &minDist, INT depth)
{
	LONG closestPoint = -1;
	if(m_interior){
		INT axis = depth%3;
		KDNode *child = m_rChild;
		Bool visitLeft = FALSE;
		Real val = 0.;
		switch(axis){
			case 0:
				val = point.x;
				break;
			case 1:
				val = point.y;
				break;
			case 2:
				val = point.z;
				break;
		}
		if(val < m_splitPos){
			child = m_lChild;
			visitLeft = TRUE;
		}
		closestPoint = child->getNearestNeighbor(points,point,validPoints,minDist,depth+1);
		if(Abs(val-m_splitPos) < minDist || minDist < 0){
			child = m_lChild;
			if(visitLeft){
				child = m_rChild;
			}
			LONG newPoint = child->getNearestNeighbor(points,point,validPoints,minDist,depth+1);
			closestPoint = newPoint != -1? newPoint : closestPoint;
		}
	}
	else{
		for(LONG i=0;i<m_numPoints;i++){
			Real dist = (point - points[m_points[i]]).GetLength();
			if(validPoints[m_points[i]] && (dist < minDist || minDist < 0)){
				minDist = dist;
				closestPoint = m_points[i];
			}
		}
	}
	String indent = "";
	for(INT i=0;i<depth;i++){
		indent += "  ";
	}
	return closestPoint;
}