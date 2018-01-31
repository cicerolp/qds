const EPSILON=-1e-20;

class Triangulator{

    static insideTriangle(const a, const b, const c, const p)
    {
	var ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	var cCROSSap, bCROSScp, aCROSSbp;

	ax = c.x()-b.x();  ay = c.y()-b.y();
	bx = a.x()-c.x();  by = a.y()-c.y();
	cx = b.x()-a.x();  cy = b.y()-a.y();
	apx= p.x()-a.x();  apy= p.y()-a.y();
	bpx= p.x()-b.x();  bpy= p.y()-b.y();
	cpx= p.x()-c.x();  cpy= p.y()-c.y();

	aCROSSbp = ax*bpy-ay*bpx;
	cCROSSap = cx*apy-cy*apx;
	bCROSScp = bx*cpy-by*cpx;
	
	return ((aCROSSbp > 0.0f) && (bCROSScp > 0.0f) && (cCROSSap > 0.0f));
    }
    
    static intersect(const p, const p1, const a, const b)
    {
	var ABx = a.y()-b.y();
	var ABy = b.x()-a.x();
	var PPx = p.y()-p1.y();
	var PPy = p1.x()-p.x();
	return ((ABx*(p.x()-a.x())+ABy*(p.y()-a.y()))*(ABx*(p1.x()-a.x())+ABy*(p1.y()-a.y()))<0) &&
	    ((PPx*(a.x()-p.x())+PPy*(a.y()-p.y()))*(PPx*(b.x()-p.x())+PPy*(b.y()-p.y()))<0);
    }

    static areaPoly(const contour)
    {
	var n = contour.count();
	var A = 0;
	for(var p=n-1,q=0; q<n; p=q++)
	    A+= contour.at(p).x()*contour.at(q).y() - contour.at(q).x()*contour.at(p).y();
	return A*0.5;
    }
    
    static areaTriangle(const a, const b, const c)
    {
	var A = (a.x()*b.y() - b.x()*a.y()) + (b.x()*c.y() - c.x()*b.y()) + (c.x()*a.y() - a.x()*c.y());
	return A*0.5;
    }

    static cw(const a, const b, const c)
    {
	return (Triangulator.areaTriangle(a,b,c) < EPSILON);
    }

    static earQ(i, j, k, const p)
    {
	if (cw(p.at(i), p.at(j), p.at(k)))
	    return false;

	for (var m=0; m<p.count(); m++) {
	    if ((m!=i) && (m!=j) && (m!=k)) {
		if (Triangulator::insideTriangle(p.at(i), p.at(j), p.at(k), p.at(m)))
		    return false;
	    }
	}

	return true;
    }

    earClip(const poly, result, shift)
    {
	var n = poly.count();
	if ( n < 3 ) return false;
	
	var *V = new int[n*3];
	var *l = V+n;
	var *r = l+n;
	
	if (Triangulator.areaPoly(poly)>0.0)
	    for(int v=0; v<n; v++) V[v] = v;
	else
	    for(int v=0; v<n; v++) V[v] = (n-1)-v;
	
	for (int i=0; i<n; i++) {
	    l[i] = ((i-1) + n) % n;
	    r[i] = ((i+1) + n) % n;
	}
	int cnt = 0;
	int i = n-1;
	int loop = 0;
	while (cnt<n-2 && loop<n-2) {
	    i = r[i];
	    if (i==r[i]) break;
	    if (earQ(V[l[i]],V[i],V[r[i]],poly)) {
		result << (V[l[i]]+shift) << (V[i]+shift) << (V[r[i]]+shift);
		cnt++;
		l[r[i]] = l[i];
		r[l[i]] = r[i];
		loop = 0;
	    }
	    else
		loop++;
	}
	delete V;
	return loop==0;
    }

    bool Triangulator::triangulate(const QList<QPolygonF> &polys, QVector<float> &coords, QVector<unsigned> &indices, int shift)
    {
	bool noError = false;
	for (int j=0; j<polys.count(); j++) {
	    QList<QPolygonF> polygons;
	    QPolygonF subPath;
	    QPolygonF poly = polys.at(j);
	    int start=-1, lastStart=0;
	    for (int k=0; k<poly.count(); k++) {
		if (start==-1) {
		    start = k;
		}
		else if (poly.at(k)==poly.at(start)) {
		    QPolygonF subPoly = poly.mid(start, k-start+1);
		    if (subPath.boundingRect().contains(subPoly.boundingRect())) { // holes
			// remove empty lines
			int front=0;
			int back = subPoly.count()-1;
			while (front<back && subPoly.at(front)==subPoly.at(back))
			{ front++; back--; }
			if (front>1)
			    subPoly = subPoly.mid(front-1, subPoly.count()-(front-1)*2);
			
			int minIdx=-1, minIdx2=-1;
			float minDis = 1e30f;
			for (int l2=0; l2<subPoly.count(); l2++) {
			    for (int l=0; l<subPath.count(); l++) {
				float dis = QVector2D(subPath.at(l)-subPoly.at(l2)).lengthSquared();
				if (dis<minDis) {
				    minIdx = l;
				    minIdx2 = l2;
				    minDis = dis;
				}
			    }
			}
			subPath = subPath.mid(0, minIdx+1) + subPoly.mid(minIdx2) + subPoly.mid(1, minIdx2) + subPath.mid(minIdx);
		    }
		    else {
			if (subPath.count()>0)
			    polygons << subPath;
			subPath = subPoly;
		    }
		    start = -1;
		}
	    }
	    if (subPath.count()>0)
		polygons << subPath;
	    for (int k=0; k<polygons.count(); k++) {
		poly = polygons.at(k);
		Triangulator::earClip(poly, indices, shift);
		shift += poly.count();
		for (int i=0; i<poly.count(); i++) {
		    coords << poly.at(i).x() << poly.at(i).y();
		}
	    }
	}
	return noError;
    }

    bool Triangulator::triangulate(const QPolygonF &poly, QVector<unsigned> &indices, int shift)
    {
	bool noError = false;
	QList<QPolygonF> polygons;
	QPolygonF subPath;
	int start=-1, lastStart=0;
	for (int k=0; k<poly.count(); k++) {
	    if (start==-1) {
		start = k;
	    }
	    else if (poly.at(k)==poly.at(start)) {
		QPolygonF subPoly = poly.mid(start, k-start+1);
		if (subPath.boundingRect().contains(subPoly.boundingRect())) { // holes
		    // remove empty lines
		    int front=0;
		    int back = subPoly.count()-1;
		    while (front<back && subPoly.at(front)==subPoly.at(back))
		    { front++; back--; }
		    if (front>1)
			subPoly = subPoly.mid(front-1, subPoly.count()-(front-1)*2);
		    
		    int minIdx=-1, minIdx2=-1;
		    float minDis = 1e30f;
		    for (int l2=0; l2<subPoly.count(); l2++) {
			for (int l=0; l<subPath.count(); l++) {
			    float dis = QVector2D(subPath.at(l)-subPoly.at(l2)).lengthSquared();
			    if (dis<minDis) {
				minIdx = l;
				minIdx2 = l2;
				minDis = dis;
			    }
			}
		    }
		    subPath = subPath.mid(0, minIdx+1) + subPoly.mid(minIdx2) + subPoly.mid(1, minIdx2) + subPath.mid(minIdx);
		}
		else {
		    if (subPath.count()>0)
			polygons << subPath;
		    subPath = subPoly;
		}
		start = -1;
	    }
	}
	if (subPath.count()>0)
	    polygons << subPath;
	for (int k=0; k<polygons.count(); k++) {
	    const QPolygonF &poly = polygons.at(k);
	    Triangulator::earClip(poly, indices, shift);
	    shift += poly.count();
	}
	return noError;
    }


}
