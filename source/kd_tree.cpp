#include "kd_tree.h"
#include "ge_sort.h"

class Sorter : public  GeSortAndSearchEx
{
	public:
		virtual Int32 Compare(void* a, void* b)
		{
			Float aa = *((Float *)a);
			Float bb = *((Float *)b);
			if (aa<bb) return -1;
			if (aa>bb) return 1;
			return 0;
		}
};

KDNode::~KDNode()
{
	if(m_interior){
		DeleteMem(m_lChild);
		DeleteMem(m_rChild);
	}
	else{
		DeleteMem(m_points);
	}
}

void
KDNode::initInterior(Float splitPos)
{
	m_interior = TRUE;
	m_splitPos = splitPos;
	m_lChild = NULL;
	m_rChild = NULL;
}

void
KDNode::initLeaf(const Int32 *points, Int32 numPoints)
{
	m_interior = FALSE;
	m_points = points;
	m_numPoints = numPoints;
}

static
Float
getMedian(const GeDynamicArray<Vector> &points, const Int32 *pnts, Int32 numPoints, Random &rng, INT axis, Sorter &sorter)
{
	Int32 maxSamples = 20;
	Int32 numSamples = numPoints < maxSamples ? numPoints : maxSamples;
	Float* samples = NewMemClear(Float,numSamples);
	for(Int32 i=0;i<numSamples;i++){
		Float val = rng.Get01();
		Int32 a = val * (numPoints-1);
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
	sorter.Sort(samples,numSamples,sizeof(Float));
	Float ret = samples[numSamples/2];
	DeleteMem(samples);
	return ret;
}

static
KDNode *
recursiveBuild(const GeDynamicArray<Vector> &points, const Int32 *pnts, Int32 numPoints, Random &rng, INT depth, Sorter &sorter)
{
	KDNode *node =NewMemClear(KDNode,1);
	Int32 minNodeEntries = 4;
	Int32 maxDepth = 10;
	if(numPoints > minNodeEntries && depth < maxDepth){
		INT axis = depth%3;
		Float splitPos = getMedian(points,pnts,numPoints,rng,axis,sorter);
		Int32 numLeft = 0;
		for(Int32 i=0;i<numPoints;i++){
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
		Int32 *lPoints = NewMemClear(Int32,numLeft);
		Int32 *rPoints = NewMemClear(Int32,numPoints - numLeft);
		Int32 lPos=0;
		Int32 rPos=0;
		for(Int32 i=0;i<numPoints;i++){
			Float val = 0;
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
		DeleteMem(pnts);
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
	Int32 *pnts = NewMemClear(Int32,points.GetCount());
	Int32 numPoints = points.GetCount();
	for(Int32 i=0;i<numPoints;i++){
		pnts[i] = i;
	}
	*nodes = recursiveBuild(points,pnts,numPoints,rng,0,sorter);
}

Int32
KDNode::getNearestNeighbor(const GeDynamicArray<Vector> &points, const Vector &point, const GeDynamicArray<Int32> &validPoints, Float &minDist, INT depth)
{
	Int32 closestPoint = -1;
	if(m_interior){
		INT axis = depth%3;
		KDNode *child = m_rChild;
		Bool visitLeft = FALSE;
		Float val = 0.;
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
			Int32 newPoint = child->getNearestNeighbor(points,point,validPoints,minDist,depth+1);
			closestPoint = newPoint != -1? newPoint : closestPoint;
		}
	}
	else{
		for(Int32 i=0;i<m_numPoints;i++){
			Float dist = (point - points[m_points[i]]).GetLength();
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