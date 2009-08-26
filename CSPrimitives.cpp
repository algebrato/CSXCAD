#include "CSPrimitives.h"
#include "CSProperties.h"
#include <sstream>
#include <iostream>
#include <limits>
#include "tinyxml.h"
#include "fparser.hh"

/*********************CSPrimitives********************************************************************/
CSPrimitives::CSPrimitives(unsigned int ID, ParameterSet* paraSet, CSProperties* prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=0;
	clProperty=NULL;
	SetProperty(prop);
	uiID=ID;
	clParaSet=paraSet;
	iPriority=0;
}

CSPrimitives::CSPrimitives(CSPrimitives* prim, CSProperties *prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=prim->dBoundBox[i];
	clProperty=NULL;
	if (prop==NULL) SetProperty(prim->clProperty);
	else SetProperty(prop);
	uiID=prim->uiID;
	clParaSet=prim->clParaSet;
	iPriority=prim->iPriority;
}


CSPrimitives::CSPrimitives(ParameterSet* paraSet, CSProperties* prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=0;
	clProperty=NULL;
	SetProperty(prop);
	clParaSet=paraSet;
	uiID=0;
	iPriority=0;
}

void CSPrimitives::SetProperty(CSProperties *prop)
{
	if (clProperty!=NULL) clProperty->RemovePrimitive(this);
	clProperty=prop;
	if (prop!=NULL) prop->AddPrimitive(this);
}

CSPrimitives::~CSPrimitives()
{
	if (clProperty!=NULL) clProperty->RemovePrimitive(this);
}

bool CSPrimitives::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement *elem = root.ToElement();

	elem->SetAttribute("ID",uiID);
	if (clProperty!=NULL) elem->SetAttribute("PropertyID",clProperty->GetID());
	else elem->SetAttribute("PropertyID",-1);
	elem->SetAttribute("Priority",iPriority);

	return true;
}

bool CSPrimitives::ReadFromXML(TiXmlNode &root)
{
	int help;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
    if (elem->QueryIntAttribute("ID",&help)!=TIXML_SUCCESS) return false;
	uiID=(unsigned int)help;
    if (elem->QueryIntAttribute("Priority",&iPriority)!=TIXML_SUCCESS) return false;

	return true;
}


/*********************CSPrimBox********************************************************************/
CSPrimBox::CSPrimBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}

CSPrimBox::CSPrimBox(CSPrimBox* primBox, CSProperties *prop) : CSPrimitives(primBox,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
}

CSPrimBox::CSPrimBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}


CSPrimBox::~CSPrimBox()
{
}

double* CSPrimBox::GetBoundBox(bool &accurate)
{
	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
	for (int i=0;i<3;++i)
		if (dBoundBox[2*i]>dBoundBox[2*i+1])
		{
			double help=dBoundBox[2*i];
			dBoundBox[2*i]=dBoundBox[2*i+1];
			dBoundBox[2*i+1]=help;
		}
	accurate=true;
	return dBoundBox;
}

bool CSPrimBox::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	return true;
}


bool CSPrimBox::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<6;++i)
	{
		EC=psCoords[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Box (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("Box");

	CSPrimitives::Write2XML(elem,parameterised);

	TiXmlElement P1("P1");
	WriteTerm(psCoords[0],P1,"X",parameterised);
	WriteTerm(psCoords[2],P1,"Y",parameterised);
	WriteTerm(psCoords[4],P1,"Z",parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement P2("P2");
	WriteTerm(psCoords[1],P2,"X",parameterised);
	WriteTerm(psCoords[3],P2,"Y",parameterised);
	WriteTerm(psCoords[5],P2,"Z",parameterised);
	elem.InsertEndChild(P2);

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	//P1
	TiXmlElement* P1=root.FirstChildElement("P1");
	if (P1==NULL) return false;
	if (ReadTerm(psCoords[0],*P1,"X")==false) return false;
	if (ReadTerm(psCoords[2],*P1,"Y")==false) return false;
	if (ReadTerm(psCoords[4],*P1,"Z")==false) return false;
	TiXmlElement* P2=root.FirstChildElement("P2");
	if (P1==NULL) return false;
	if (ReadTerm(psCoords[1],*P2,"X")==false) return false;
	if (ReadTerm(psCoords[3],*P2,"Y")==false) return false;
	if (ReadTerm(psCoords[5],*P2,"Z")==false) return false;

	return true;
}

/*********************CSPrimMultiBox********************************************************************/
CSPrimMultiBox::CSPrimMultiBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=MULTIBOX;
}

CSPrimMultiBox::CSPrimMultiBox(CSPrimMultiBox* multiBox, CSProperties *prop) : CSPrimitives(multiBox, prop)
{
	Type=MULTIBOX;
	for (size_t i=0;i<multiBox->vCoords.size();++i) vCoords.push_back(ParameterScalar(multiBox->vCoords.at(i)));
}

CSPrimMultiBox::CSPrimMultiBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=MULTIBOX;
}

CSPrimMultiBox::~CSPrimMultiBox()
{
}

void CSPrimMultiBox::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimMultiBox::SetCoord(int index, const char* val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimMultiBox::AddCoord(double val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddCoord(const char* val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddBox(int initBox)
{
	ClearOverlap();
	if ((initBox<0) || (((initBox+1)*6)>(int)vCoords.size()))
	{
		for (unsigned int i=0;i<6;++i) AddCoord(0.0);
	}
	else for (unsigned int i=0;i<6;++i) vCoords.push_back(ParameterScalar(vCoords.at(6*initBox+i)));
}

void CSPrimMultiBox::DeleteBox(size_t box)
{
	if ((box*6<0) || ((box+1)*6>vCoords.size())) return;
	vector<ParameterScalar>::iterator start=vCoords.begin()+(box*6);
	vector<ParameterScalar>::iterator end=vCoords.begin()+(box*6+6);

	vCoords.erase(start,end);
}


double CSPrimMultiBox::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return vCoords.at(index).GetValue();
	return 0;
}

ParameterScalar* CSPrimMultiBox::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return &vCoords.at(index);
	return NULL;
}

double* CSPrimMultiBox::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i) array[i]=vCoords.at(i).GetValue();
	return array;
}

void CSPrimMultiBox::ClearOverlap()
{
	if (vCoords.size()%6==0) return;  //no work to be done

	vCoords.resize(vCoords.size()-vCoords.size()%6);
}

double* CSPrimMultiBox::GetBoundBox(bool &accurate)
{
	//Update();
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		for (unsigned int n=0;n<3;++n)
		{
			if (vCoords.at(6*i+2*n).GetValue()<=vCoords.at(6*i+2*n+1).GetValue())
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n).GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1).GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n).GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n).GetValue();
					if (vCoords.at(6*i+2*n+1).GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1).GetValue();
				}

			}
			else
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n+1).GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n).GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n+1).GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n+1).GetValue();
					if (vCoords.at(6*i+2*n).GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n).GetValue();
				}
			}
		}
	}
	accurate=false;
	return dBoundBox;
}

bool CSPrimMultiBox::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;
	bool in=false;
	double UpVal,DownVal;
	//fprintf(stderr,"here\n");
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		in=true;
		for (unsigned int n=0;n<3;++n)
		{
			//fprintf(stderr,"%e %e %e \n",vCoords.at(6*i+2*n).GetValue(),vCoords.at(6*i+2*n+1).GetValue());
			UpVal=vCoords.at(6*i+2*n+1).GetValue();
			DownVal=vCoords.at(6*i+2*n).GetValue();
			if (DownVal<UpVal)
			{
				if (DownVal>Coord[n]) {in=false;break;}
				if (UpVal<Coord[n]) {in=false;break;}
			}
			else
			{
				if (DownVal<Coord[n]) {in=false;break;}
				if (UpVal>Coord[n]) {in=false;break;}
			}
		}
		if (in==true) {	return true;}
	}
	return false;
}

bool CSPrimMultiBox::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (size_t i=0;i<vCoords.size();++i)
	{
		EC=vCoords.at(i).Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=0)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in MultiBox (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimMultiBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("MultiBox");
	CSPrimitives::Write2XML(elem,parameterised);
	elem.SetAttribute("QtyBox",(int)vCoords.size()/6);

	for (size_t i=0;i<vCoords.size()/6;++i)
	{
		TiXmlElement SP("StartP");
		WriteTerm(vCoords.at(i*6),SP,"X",parameterised);
		WriteTerm(vCoords.at(i*6+2),SP,"Y",parameterised);
		WriteTerm(vCoords.at(i*6+4),SP,"Z",parameterised);
		elem.InsertEndChild(SP);

		TiXmlElement EP("EndP");
		WriteTerm(vCoords.at(i*6+1),EP,"X",parameterised);
		WriteTerm(vCoords.at(i*6+3),EP,"Y",parameterised);
		WriteTerm(vCoords.at(i*6+5),EP,"Z",parameterised);
		elem.InsertEndChild(EP);
	}

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimMultiBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;;

	TiXmlElement *SP=root.FirstChildElement("StartP");
	TiXmlElement *EP=root.FirstChildElement("EndP");
	if (vCoords.size()!=0) return false;
	int i=0;
	while ((SP!=NULL) && (EP!=NULL))
	{
		for (int n=0;n<6;++n) this->AddCoord(0.0);

		if (ReadTerm(vCoords.at(i*6),*SP,"X")==false) return false;
		if (ReadTerm(vCoords.at(i*6+2),*SP,"Y")==false) return false;
		if (ReadTerm(vCoords.at(i*6+4),*SP,"Z")==false) return false;

		if (ReadTerm(vCoords.at(i*6+1),*EP,"X")==false) return false;
		if (ReadTerm(vCoords.at(i*6+3),*EP,"Y")==false) return false;
		if (ReadTerm(vCoords.at(i*6+5),*EP,"Z")==false) return false;

//		for (int n=0;n<6;++n) fprintf(stderr,"%e ",vCoords.at(i*6+n).GetValue());
//		fprintf(stderr,"\n");

		SP=SP->NextSiblingElement("StartP");
		EP=EP->NextSiblingElement("EndP");
		++i;
	};
	return true;
}

/*********************CSPrimSphere********************************************************************/
CSPrimSphere::CSPrimSphere(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
}

CSPrimSphere::CSPrimSphere(CSPrimSphere* sphere, CSProperties *prop) : CSPrimitives(sphere,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i]=ParameterScalar(sphere->psCenter[i]);}
	psRadius=ParameterScalar(sphere->psRadius);
}

CSPrimSphere::CSPrimSphere(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
}


CSPrimSphere::~CSPrimSphere()
{
}

double* CSPrimSphere::GetBoundBox(bool &accurate)
{
	for (unsigned int i=0;i<3;++i)
	{
		dBoundBox[2*i]=psCenter[i].GetValue()-psRadius.GetValue();
		dBoundBox[2*i+1]=psCenter[i].GetValue()+psRadius.GetValue();
	}
	accurate=true;
	return dBoundBox;
}

bool CSPrimSphere::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;
	double dist=sqrt(pow(Coord[0]-psCenter[0].GetValue(),2)+pow(Coord[1]-psCenter[1].GetValue(),2)+pow(Coord[2]-psCenter[2].GetValue(),2));
	if (dist<psRadius.GetValue()) return true;
	return false;
}

bool CSPrimSphere::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<3;++i)
	{
		EC=psCenter[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Sphere Center Point (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=psRadius.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Sphere Radius (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPrimSphere::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("Sphere");

	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Center("Center");
	WriteTerm(psCenter[0],Center,"X",parameterised);
	WriteTerm(psCenter[1],Center,"Y",parameterised);
	WriteTerm(psCenter[2],Center,"Z",parameterised);
	elem.InsertEndChild(Center);

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimSphere::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	TiXmlElement* Center=root.FirstChildElement("Center");
	if (Center==NULL) return false;
	if (ReadTerm(psCenter[0],*Center,"X")==false) return false;
	if (ReadTerm(psCenter[1],*Center,"Y")==false) return false;
	if (ReadTerm(psCenter[2],*Center,"Z")==false) return false;

	return true;
}

/*********************CSPrimCylinder********************************************************************/
CSPrimCylinder::CSPrimCylinder(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
}

CSPrimCylinder::CSPrimCylinder(CSPrimCylinder* cylinder, CSProperties *prop) : CSPrimitives(cylinder,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(cylinder->psCoords[i]);}
	psRadius=ParameterScalar(cylinder->psRadius);
}

CSPrimCylinder::CSPrimCylinder(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
}


CSPrimCylinder::~CSPrimCylinder()
{
}

double* CSPrimCylinder::GetBoundBox(bool &accurate)
{
	accurate=false;
	int Direction=0;
	double dCoords[6];
	for (unsigned int i=0;i<6;++i)
		dCoords[i]=psCoords[i].GetValue();
	double rad=psRadius.GetValue();
	for (unsigned int i=0;i<3;++i)
	{
		//vorerst ganz einfach... muss ueberarbeitet werden!!! //todo
		double min=dCoords[2*i];
		double max=dCoords[2*i+1];
		if (min<max)
		{
			dBoundBox[2*i]=min-rad;
			dBoundBox[2*i+1]=max+rad;
		}
		else
		{
			dBoundBox[2*i+1]=min+rad;
			dBoundBox[2*i]=max-rad;
		}
		if (min==max) Direction+=pow(2,i);
	}
	switch (Direction)
	{
	case 3: //orientaion in z-direction
		dBoundBox[4]=dBoundBox[4]+rad;
		dBoundBox[5]=dBoundBox[5]-rad;
		accurate=true;
		break;
	case 5: //orientaion in y-direction
		dBoundBox[2]=dBoundBox[2]+rad;
		dBoundBox[3]=dBoundBox[3]-rad;
		accurate=true;
		break;
	case 6: //orientaion in x-direction
		dBoundBox[1]=dBoundBox[1]+rad;
		dBoundBox[2]=dBoundBox[2]-rad;
		accurate=true;
		break;
	}
	return dBoundBox;
}

bool CSPrimCylinder::IsInside(double* Coord, double tol)
{
	//Lot-Fuss-Punkt
	if (Coord==NULL) return false;
	double* p=Coord; //punkt
	double r0[3]={psCoords[0].GetValue(),psCoords[2].GetValue(),psCoords[4].GetValue()}; //aufpunkt
	double r1[3]={psCoords[1].GetValue(),psCoords[3].GetValue(),psCoords[5].GetValue()}; //aufpunkt
	double a[3]={r1[0]-r0[0],r1[1]-r0[1],r1[2]-r0[2]}; //richtungsvektor
	double a2=(a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2]);
	double FP[3];
	double e=0;
	double t=(p[0]-r0[0])*a[0]+(p[1]-r0[1])*a[1]+(p[2]-r0[2])*a[2];
	t/=a2;
	for (int i=0;i<3;++i)
	{
		FP[i]=r0[i]+t*a[i];
		if ((FP[i]<r0[i] || FP[i]>r1[i]) && (a[i]>0)) return false;
		if ((FP[i]>r0[i] || FP[i]<r1[i]) && (a[i]<0)) return false;
		e+=(FP[i]-p[i])*(FP[i]-p[i]);
	}
	double r=psRadius.GetValue();
	if (e<r*r) return true;
	return false;
}

bool CSPrimCylinder::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<6;++i)
	{
		EC=psCoords[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Cylinder Coord (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=psRadius.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Cylinder Radius (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPrimCylinder::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("Cylinder");

	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Start("P0");
	WriteTerm(psCoords[0],Start,"X",parameterised);
	WriteTerm(psCoords[2],Start,"Y",parameterised);
	WriteTerm(psCoords[4],Start,"Z",parameterised);
	elem.InsertEndChild(Start);

	TiXmlElement Stop("P1");
	WriteTerm(psCoords[1],Stop,"X",parameterised);
	WriteTerm(psCoords[3],Stop,"Y",parameterised);
	WriteTerm(psCoords[5],Stop,"Z",parameterised);
	elem.InsertEndChild(Stop);

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimCylinder::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	TiXmlElement* Point=root.FirstChildElement("P0");
	if (Point==NULL) return false;
	if (ReadTerm(psCoords[0],*Point,"X")==false) return false;
	if (ReadTerm(psCoords[2],*Point,"Y")==false) return false;
	if (ReadTerm(psCoords[4],*Point,"Z")==false) return false;

	Point=root.FirstChildElement("P1");
	if (Point==NULL) return false;
	if (ReadTerm(psCoords[1],*Point,"X")==false) return false;
	if (ReadTerm(psCoords[3],*Point,"Y")==false) return false;
	if (ReadTerm(psCoords[5],*Point,"Z")==false) return false;

	return true;
}

/*********************CSPrimPolygon********************************************************************/
CSPrimPolygon::CSPrimPolygon(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=POLYGON;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}

CSPrimPolygon::CSPrimPolygon(CSPrimPolygon* primPolygon, CSProperties *prop) : CSPrimitives(primPolygon,prop)
{
	Type=POLYGON;
//	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
}

CSPrimPolygon::CSPrimPolygon(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=POLYGON;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}


CSPrimPolygon::~CSPrimPolygon()
{
}

void CSPrimPolygon::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::SetCoord(int index, const string val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::AddCoord(double val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::AddCoord(const string val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::RemoveCoords(int index)
{
	//not yet implemented
}

double CSPrimPolygon::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return vCoords.at(index).GetValue();
	return 0;
}

ParameterScalar* CSPrimPolygon::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return &vCoords.at(index);
	return NULL;
}

double* CSPrimPolygon::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i) array[i]=vCoords.at(i).GetValue();
	return array;
}


double* CSPrimPolygon::GetBoundBox(bool &accurate)
{
//	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
//	for (int i=0;i<3;++i)
//		if (dBoundBox[2*i]>dBoundBox[2*i+1])
//		{
//			double help=dBoundBox[2*i];
//			dBoundBox[2*i]=dBoundBox[2*i+1];
//			dBoundBox[2*i+1]=help;
//		}
//	accurate=true;
	return dBoundBox;
}

bool CSPrimPolygon::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	//more checking needed!!
	return true;
}


bool CSPrimPolygon::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
//	for (int i=0;i<6;++i)
//	{
//		EC=psCoords[i].Evaluate();
//		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
//		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
//		{
//			bOK=false;
//			stringstream stream;
//			stream << endl << "Error in Box (ID: " << uiID << "): ";
//			ErrStr->append(stream.str());
//			PSErrorCode2Msg(EC,ErrStr);
//		}
//	}
	return bOK;
}

bool CSPrimPolygon::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("Polygon");

	CSPrimitives::Write2XML(elem,parameterised);
	
	WriteTerm(Elevation,elem,"Elevation",parameterised);

	elem.SetAttribute("QtyVertices",(int)vCoords.size()/2);
	TiXmlElement NV("NormDir");
	WriteTerm(NormDir[0],NV,"X",parameterised);	
	WriteTerm(NormDir[1],NV,"Y",parameterised);	
	WriteTerm(NormDir[2],NV,"Z",parameterised);	
	elem.InsertEndChild(NV);

	for (size_t i=0;i<vCoords.size()/2;++i)
	{
		TiXmlElement VT("Vertex");
		WriteTerm(vCoords.at(i*2),VT,"X1",parameterised);
		WriteTerm(vCoords.at(i*2+1),VT,"X2",parameterised);
		elem.InsertEndChild(VT);
	}

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimPolygon::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(Elevation,*elem,"Elevation")==false) return false;

	TiXmlElement* NV=root.FirstChildElement("NormDir");
	if (NV==NULL) return false;
	if (ReadTerm(NormDir[0],*NV,"X")==false) return false;
	if (ReadTerm(NormDir[1],*NV,"Y")==false) return false;
	if (ReadTerm(NormDir[2],*NV,"Z")==false) return false;
	
	TiXmlElement *VT=root.FirstChildElement("Vertex");
	if (vCoords.size()!=0) return false;
	int i=0;
	while (VT)
	{
		for (int n=0;n<2;++n) this->AddCoord(0.0);

		if (ReadTerm(vCoords.at(i*2),*VT,"X1")==false) return false;
		if (ReadTerm(vCoords.at(i*2+1),*VT,"X2")==false) return false;

		VT=VT->NextSiblingElement("Vertex");
		++i;
	};
	
	return true;
}

/*********************CSPrimLinPoly********************************************************************/
CSPrimLinPoly::CSPrimLinPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}

CSPrimLinPoly::CSPrimLinPoly(CSPrimLinPoly* primLinPoly, CSProperties *prop) : CSPrimPolygon(primLinPoly,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
}

CSPrimLinPoly::CSPrimLinPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}


CSPrimLinPoly::~CSPrimLinPoly()
{
}

double* CSPrimLinPoly::GetBoundBox(bool &accurate)
{
//	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
//	for (int i=0;i<3;++i)
//		if (dBoundBox[2*i]>dBoundBox[2*i+1])
//		{
//			double help=dBoundBox[2*i];
//			dBoundBox[2*i]=dBoundBox[2*i+1];
//			dBoundBox[2*i+1]=help;
//		}
//	accurate=true;
	return dBoundBox;
}

bool CSPrimLinPoly::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	//more checking needed!!
	return true;
}


bool CSPrimLinPoly::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;

	return bOK;
}

bool CSPrimLinPoly::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("LinPoly");

	CSPrimPolygon::Write2XML(elem,parameterised);

	WriteTerm(extrudeLength,elem,"Length",parameterised);


	root.InsertEndChild(elem);
	return true;
}

bool CSPrimLinPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(extrudeLength,*elem,"Length")==false) return false;

	return true;
}

/*********************CSPrimLinPoly********************************************************************/
CSPrimRotPoly::CSPrimRotPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}

CSPrimRotPoly::CSPrimRotPoly(CSPrimRotPoly* primRotPoly, CSProperties *prop) : CSPrimPolygon(primRotPoly,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
}

CSPrimRotPoly::CSPrimRotPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
}


CSPrimRotPoly::~CSPrimRotPoly()
{
}

double* CSPrimRotPoly::GetBoundBox(bool &accurate)
{
//	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
//	for (int i=0;i<3;++i)
//		if (dBoundBox[2*i]>dBoundBox[2*i+1])
//		{
//			double help=dBoundBox[2*i];
//			dBoundBox[2*i]=dBoundBox[2*i+1];
//			dBoundBox[2*i+1]=help;
//		}
//	accurate=true;
	return dBoundBox;
}

bool CSPrimRotPoly::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	//more checking needed!!
	return true;
}


bool CSPrimRotPoly::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
//	for (int i=0;i<6;++i)
//	{
//		EC=psCoords[i].Evaluate();
//		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
//		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
//		{
//			bOK=false;
//			stringstream stream;
//			stream << endl << "Error in Box (ID: " << uiID << "): ";
//			ErrStr->append(stream.str());
//			PSErrorCode2Msg(EC,ErrStr);
//		}
//	}
	return bOK;
}

bool CSPrimRotPoly::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("RotPoly");

	CSPrimPolygon::Write2XML(elem,parameterised);

	TiXmlElement RT("RotAxis");
	WriteTerm(RotAxis[0],RT,"X",parameterised);	
	WriteTerm(RotAxis[1],RT,"Y",parameterised);	
	WriteTerm(RotAxis[2],RT,"Z",parameterised);	
	elem.InsertEndChild(RT);
	
	TiXmlElement Ang("Angles");
	WriteTerm(StartStopAngle[0],Ang,"Start",parameterised);	
	WriteTerm(StartStopAngle[1],Ang,"Stop",parameterised);	
	elem.InsertEndChild(Ang);
	
	root.InsertEndChild(elem);
	return true;
}

bool CSPrimRotPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	TiXmlElement* NV=root.FirstChildElement("RotAxis");
	if (NV==NULL) return false;
	if (ReadTerm(NormDir[0],*NV,"X")==false) return false;
	if (ReadTerm(NormDir[1],*NV,"Y")==false) return false;
	if (ReadTerm(NormDir[2],*NV,"Z")==false) return false;
	
	NV=root.FirstChildElement("Angles");
	if (NV==NULL) return false;
	if (ReadTerm(StartStopAngle[0],*NV,"Start")==false) return false;
	if (ReadTerm(StartStopAngle[1],*NV,"Stop")==false) return false;

	return true;
}

/*********************CSPrimUserDefined********************************************************************/
CSPrimUserDefined::CSPrimUserDefined(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new FunctionParser();
	fParse->AddConstant("pi", 3.1415926535897932);
	stFunction = string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i) {dPosShift[i].SetParameterSet(paraSet);}
}

CSPrimUserDefined::CSPrimUserDefined(CSPrimUserDefined* primUDef, CSProperties *prop) : CSPrimitives(primUDef,prop)
{
	Type=USERDEFINED;
	fParse = new FunctionParser(*primUDef->fParse);
//	fParse = primUDef->fParse;
	stFunction = string(primUDef->stFunction);
	CoordSystem = primUDef->CoordSystem;
	for (int i=0;i<3;++i) {dPosShift[i]=ParameterScalar(primUDef->dPosShift[i]);}
}

CSPrimUserDefined::CSPrimUserDefined(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new FunctionParser();
	fParse->AddConstant("pi", 3.1415926535897932);
	stFunction = string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i) {dPosShift[i].SetParameterSet(paraSet);}
}


CSPrimUserDefined::~CSPrimUserDefined()
{
	delete fParse;fParse=NULL;
}

void CSPrimUserDefined::SetCoordSystem(UserDefinedCoordSystem newSystem)
{
	CoordSystem=newSystem;
}

void CSPrimUserDefined::SetFunction(const char* func)
{
	if (func==NULL) return;
	stFunction = string(func);
}

double* CSPrimUserDefined::GetBoundBox(bool &accurate)
{
	//this type has no simple bound box
	double max=std::numeric_limits<double>::max();
	dBoundBox[0]=dBoundBox[2]=dBoundBox[4]=-max;
	dBoundBox[1]=dBoundBox[3]=dBoundBox[5]=max;
	accurate=false;
	return dBoundBox;
}

bool CSPrimUserDefined::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;

	int NrPara=clParaSet->GetQtyParameter();
	if (NrPara!=iQtyParameter) return false;
	double *vars = new double[NrPara+6];

	vars=clParaSet->GetValueArray(vars);

	double x=Coord[0]-dPosShift[0].GetValue();
	double y=Coord[1]-dPosShift[1].GetValue();
	double z=Coord[2]-dPosShift[2].GetValue();
	double rxy=sqrt(x*x+y*y);
	vars[NrPara]=x;
	vars[NrPara+1]=y;
	vars[NrPara+2]=z;

	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:  //uses x,y,z
		vars[NrPara+3]=0;
		vars[NrPara+4]=0;
		vars[NrPara+5]=0;		break;
	case CYLINDER_SYSTEM: //uses x,y,z,r,a,0
		vars[NrPara+3]=rxy;
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=0;
		break;
	case SPHERE_SYSTEM:   //uses x,y,z,r,a,t
		vars[NrPara+3]=sqrt(x*x+y*y+z*z);
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=asin(1)-atan(z/rxy);
		//cout << "x::" << x << "y::" << y << "z::" << z << "r::" << vars[NrPara] << "a::" << vars[NrPara+1] << "t::" << vars[NrPara+2] << endl;
		break;
	default:
		//unknown System
		return false;
		break;
	}
	double dValue=0;

	if (fParse->GetParseErrorType()==FunctionParser::FP_NO_ERROR) dValue=fParse->Eval(vars);
	else dValue=0;
	delete[] vars;vars=NULL;

	return dValue==1;
}


bool CSPrimUserDefined::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	string vars;
	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:
		vars=string("x,y,z");
		break;
	case CYLINDER_SYSTEM:
		vars=string("x,y,z,r,a");
		break;
	case SPHERE_SYSTEM:
		vars=string("x,y,z,r,a,t");
		break;
	default:
		//unknown System
		return false;
		break;
	}
	iQtyParameter=clParaSet->GetQtyParameter();
	if (iQtyParameter>0)
	{
		fParameter=string(clParaSet->GetParameterString());
		vars = fParameter + "," + vars;
	}

	fParse->Parse(stFunction,vars);

	EC=fParse->GetParseErrorType();
	//cout << fParse.ErrorMsg();

	if (EC!=FunctionParser::FP_NO_ERROR) bOK=false;

	if ((EC!=FunctionParser::FP_NO_ERROR)  && (ErrStr!=NULL))
	{
		ostringstream oss;
		oss << "\nError in User Defined Primitive Function (ID: " << uiID << "): " << fParse->ErrorMsg();
		ErrStr->append(oss.str());
		bOK=false;
	}

	for (int i=0;i<3;++i)
	{
		EC=dPosShift[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			ostringstream oss;
			oss << "\nError in User Defined Primitive Coord (ID: " << uiID << "): ";
			ErrStr->append(oss.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimUserDefined::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement elem("UserDefined");

	CSPrimitives::Write2XML(elem,parameterised);

	elem.SetAttribute("CoordSystem",CoordSystem);

	TiXmlElement P1("CoordShift");
	WriteTerm(dPosShift[0],P1,"X",parameterised);
	WriteTerm(dPosShift[1],P1,"Y",parameterised);
	WriteTerm(dPosShift[2],P1,"Z",parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement FuncElem("Function");
	TiXmlText FuncText(GetFunction());
	FuncElem.InsertEndChild(FuncText);

	elem.InsertEndChild(FuncElem);

	root.InsertEndChild(elem);
	return true;
}

bool CSPrimUserDefined::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	int value;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
    if (elem->QueryIntAttribute("CoordSystem",&value)!=TIXML_SUCCESS) return false;
	SetCoordSystem((UserDefinedCoordSystem)value);

	//P1
	TiXmlElement* P1=root.FirstChildElement("CoordShift");
	if (P1==NULL) return false;
	if (ReadTerm(dPosShift[0],*P1,"X")==false) return false;
	if (ReadTerm(dPosShift[1],*P1,"Y")==false) return false;
	if (ReadTerm(dPosShift[2],*P1,"Z")==false) return false;

	TiXmlElement* FuncElem=root.FirstChildElement("Function");
	if (FuncElem==NULL) return false;
	SetFunction(FuncElem->GetText());

	return true;
}

