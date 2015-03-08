#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_tools.h"
#include "ge_dynamicarray.h"
#include "octpointgenerator.h"

class PointGeneratorData : public ObjectData
{
	private:
		LineObject *PrepareSingleSpline(BaseObject *generator, BaseObject *op, Matrix *ml, HierarchyHelp *hh, Bool *dirty);
		void Transform(PointObject *op, const Matrix &m);
		void DoRecursion(BaseObject *op, BaseObject *child, Matrix ml, GeDynamicArray<PolygonObject *> &objs, GeDynamicArray<Matrix> &matrices);
	public:
		virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
		virtual Bool Init(GeListNode *node);

		static NodeData *Alloc(void) { return NewObjClear(PointGeneratorData); }
};

void PointGeneratorData::Transform(PointObject *op, const Matrix &m)
{
	Vector	*padr=op->GetPointW();
	Int32	pcnt=op->GetPointCount(),i;
	
	for (i=0; i<pcnt; i++)
		padr[i] = m * padr[i];
	
	op->Message(MSG_UPDATE);
}

Bool PointGeneratorData::Init(GeListNode *node)
{	
	BaseObject		*op   = (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetInt32(CTPOINTGENERATOR_NUMPOINTS,1000);

	return TRUE;
}

void PointGeneratorData::DoRecursion(BaseObject *op, BaseObject *child, Matrix ml, GeDynamicArray<PolygonObject *> &objs, GeDynamicArray<Matrix> &matrices)
{
	BaseObject *tp;
	if (child){
		tp = child->GetDeformCache();
		ml = ml * child->GetMl();
		if (tp){
			DoRecursion(op,tp,ml,objs,matrices);
		}
		else{
			tp = child->GetCache(NULL);
			if (tp){
				DoRecursion(op,tp,ml,objs,matrices);
			}
			else{
				if (!child->GetBit(BIT_CONTROLOBJECT)){
					if (child->IsInstanceOf(Opolygon)){
						objs.Push(ToPoly(child));
						matrices.Push(ml);
					}
				}
			}
		}
		for (tp = child->GetDown(); tp; tp=tp->GetNext()){
			DoRecursion(op,tp,ml,objs,matrices);
		}
	}
}

BaseObject *PointGeneratorData::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	Random rng;
	BaseObject *orig = op->GetDown();

	if (!orig) return NULL;

	PolygonObject	*pp=NULL;
	Bool			dirty=FALSE;
	Matrix			ml;
	BaseObject *child = op->GetAndCheckHierarchyClone(hh,orig,HIERARCHYCLONEFLAGS_ASPOLY,&dirty,NULL,TRUE);

	if (!dirty) dirty = op->CheckCache(hh);					
	if (!dirty) dirty = op->IsDirty(DIRTYFLAGS_DATA);		
	if (!dirty) return op->GetCache(hh);

	StatusSetBar(0);
	StatusSetText("Filtering Points");

	BaseContainer *data = op->GetDataInstance();
	Int32 numPoints = data->GetInt32(CTPOINTGENERATOR_NUMPOINTS,1000);

	pp = PolygonObject::Alloc(numPoints,0);
	if (!pp){
		return NULL;
	}
	
	GeDynamicArray<PolygonObject *> objs;
	GeDynamicArray<Matrix> matrices;
	DoRecursion(op,child, ml, objs, matrices);
	GeDynamicArray<GeDynamicArray<Float> > faceAreas(objs.GetCount());
	GeDynamicArray<Float> objAreas;
	Float totalArea = 0.;
	for(Int32 i=0;i<objs.GetCount();i++){
		PolygonObject *obj = objs[i];
		Float area = 0.;
		for(Int32 ii=0;ii<obj->GetPolygonCount();ii++){
			const CPolygon &poly = obj->GetPolygonR()[ii];
			const Vector *objPoints = obj->GetPointR();
			Vector a = objPoints[poly.b] - objPoints[poly.a];
			Vector b = objPoints[poly.c] - objPoints[poly.a];
			area += 0.5*Abs(Cross(a,b).GetLength());
			if(poly.c != poly.d){
				a = objPoints[poly.b] - objPoints[poly.a];
				b = objPoints[poly.d] - objPoints[poly.a];
				area += 0.5*Abs(Cross(a,b).GetLength());
			}
			faceAreas[i].Push(area);
		}
		totalArea += area;
		objAreas.Push(area);
	}

	Vector *ppPoints=pp->GetPointW();
	rng.Init(1244);
	if(objs.GetCount() > 0){
		for(Int32 i=0;i<numPoints;i++){
			Matrix *matrix;
			Float val = rng.Get01() * totalArea;
			PolygonObject *obj = objs[objs.GetCount()-1];
			Int32 objIndex = objs.GetCount()-1;
			matrix = &matrices[objIndex];
			Float accum = 0.;
			for(Int32 ii=0;ii<objs.GetCount();ii++){
				accum += objAreas[ii];
				if(val <= accum){
					obj = objs[ii];
					matrix = &matrices[ii];
					objIndex = ii;
					break;
				}
			}
			val = rng.Get01()*objAreas[objIndex];
			const CPolygon *poly = &obj->GetPolygonR()[obj->GetPolygonCount()-1];
			Int32 faceIndex = obj->GetPolygonCount()-1;
			for(Int32 ii=0;ii<obj->GetPolygonCount()-1;ii++){
				if(val <= faceAreas[objIndex][ii]){
					faceIndex = ii;
					poly = &obj->GetPolygonR()[ii];
					break;
				}
			}
			const Vector *objPoints = obj->GetPointR();
			val = rng.Get01();
			Float otherVal = rng.Get01();
			Int32 a = poly->a;
			Int32 b = poly->b;
			Int32 c = poly->c;
			if(rng.Get11() < 0. && poly->c != poly->d)
				b = poly->d;
			val = Sqrt(val);
			ppPoints[i] = *matrix * ((1. - val)*objPoints[a] + val*(1. - otherVal)*objPoints[b] + (val*otherVal)*objPoints[c]);
		}
	}

	pp->Message(MSG_UPDATE);
	pp->SetName(op->GetName());
	StatusClear();
	
	return pp;
}

#define ID_POINTGENERATOROBJECT 1030799

Bool RegisterPointGenerator(void)
{
	return RegisterObjectPlugin(ID_POINTGENERATOROBJECT,GeLoadString(IDS_POINTGENERATOR),OBJECT_GENERATOR|OBJECT_INPUT,PointGeneratorData::Alloc,"Octpointgenerator",AutoBitmap("pointGenerator.tif"),0);
}
