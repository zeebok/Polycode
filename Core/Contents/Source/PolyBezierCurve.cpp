/*
 Copyright (C) 2011 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "PolyBezierCurve.h"

using namespace Polycode;

bool BezierCurve::cacheHeightValues = false;
unsigned int BezierCurve::defaultHeightCacheResolution = 512;

BezierPoint::BezierPoint(Number p1x, Number p1y, Number p1z, Number p2x, Number p2y, Number p2z, Number p3x, Number p3y, Number p3z) {
	p1.x = p1x;
	p1.y = p1y;
	p1.z = p1z;		
	p2.x = p2x;
	p2.y = p2y;
	p2.z = p2z;		
	p3.x = p3x;
	p3.y = p3y;
	p3.z = p3z;	
}

BezierCurve::BezierCurve(){
	insertPoint = NULL;
	evaluationAccuracy = 0.01;
    distancesDirty =  false;
    heightCacheResolution = defaultHeightCacheResolution;
}

void BezierCurve::clearControlPoints() {
	insertPoint = NULL;
    distancesDirty =  true;
	for(int i=0; i < controlPoints.size(); i++) {
		delete controlPoints[i];
	}
	controlPoints.clear();
}

BezierCurve::~BezierCurve() {
    for(int i=0; i < controlPoints.size(); i++) {
        delete controlPoints[i];
    }
}

void BezierCurve::addControlPoint(Number p1x, Number p1y, Number p1z, Number p2x, Number p2y, Number p2z, Number p3x, Number p3y, Number p3z) {
	BezierPoint* newPoint = new BezierPoint(p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z);
	
	bool inserted = false;
	for(int i=0; i < controlPoints.size(); i++) {
		if(controlPoints[i] == insertPoint) {
			controlPoints.insert(controlPoints.begin()+i, newPoint);
			inserted = true;
			break;
		}
	}
	
	if(!inserted) {
		controlPoints.push_back(newPoint);
	}
	
	distances.push_back(0);
    distancesDirty =  true;
}

void BezierCurve::addControlPoint3dWithHandles(Number p1x, Number p1y, Number p1z, Number p2x, Number p2y, Number p2z, Number p3x, Number p3y, Number p3z) {
	addControlPoint(p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z);
}

void BezierCurve::addControlPoint3d(Number x, Number y, Number z) {
	addControlPoint(x, y, z, x, y, z, x, y, z);
}

void BezierCurve::addControlPoint2dWithHandles(Number p1x, Number p1y, Number p2x, Number p2y, Number p3x, Number p3y) {
	addControlPoint(p1x, p1y, 0, p2x, p2y, 0, p3x, p3y, 0);
}

void BezierCurve::addControlPoint2d(Number x, Number y) {
	addControlPoint(x, y, 0, x, y, 0, x, y, 0);
}


void BezierCurve::recalculateDistances() {
	if(controlPoints.size() < 2)
		return;
    
    distancesDirty = false;
		
	Number dist, lastDist = 0;
	distances[0] = 0;
	Number totalDistance = 0;
		
	Vector3 point, lastPoint;
	for(int i=0; i < controlPoints.size()-1; i++) {
		lastPoint = getPointBetween(0, controlPoints[i], controlPoints[i+1]);
		dist = 0;
		for(Number a=0.0f; a < 1.0f; a += 0.01) {
			point = getPointBetween(a, controlPoints[i], controlPoints[i+1]);
			dist += point.distance(lastPoint);
			lastPoint = point;
		}
		lastDist += dist;
		distances[i+1] = lastDist;
		totalDistance += dist;
	}
	
	// normalize distances to total distance
	for(int i=0; i < controlPoints.size(); i++) {
		distances[i] = distances[i]/totalDistance;
	}
    
    minX = getPointAt(0.0).x;
    maxX = getPointAt(1.0).x;
    midX = getPointAt(0.5).x;

    if(cacheHeightValues) {
        rebuildHeightCache();
    }
}

void BezierCurve::rebuildHeightCache() {
    heightCache.clear();
    
    Number xSize = maxX - minX;
    
    for(int i=0; i < heightCacheResolution; i++) {
        Number xVal = minX + (xSize * ((Number)i)/((Number)heightCacheResolution));
        Number heightValue = getPointAt(getTValueAtX(xVal)).y;
        heightCache.push_back(heightValue);
    }
}

Vector3 BezierCurve::getPointBetween(Number a, BezierPoint *bp1, BezierPoint *bp2) {
	Vector3 retVector;
	Number b = 1.0f - a;
	
	retVector.x = bp1->p2.x*a*a*a + bp1->p3.x*3*a*a*b + bp2->p1.x*3*a*b*b + bp2->p2.x*b*b*b;
	retVector.y = bp1->p2.y*a*a*a + bp1->p3.y*3*a*a*b + bp2->p1.y*3*a*b*b + bp2->p2.y*b*b*b;
	retVector.z = bp1->p2.z*a*a*a + bp1->p3.z*3*a*a*b + bp2->p1.z*3*a*b*b + bp2->p2.z*b*b*b;

	return retVector;
}

Number BezierCurve::getYValueAtX(Number x) {
    if(cacheHeightValues) {
        if(distancesDirty) {
            recalculateDistances();
        }
        unsigned int cacheIndex = (x-minX/(maxX-minX)) * ((Number) heightCacheResolution);
        if(cacheIndex > heightCacheResolution-1) {
            cacheIndex = heightCacheResolution-1;
        }
        return heightCache[cacheIndex];
    } else {
        return getPointAt(getTValueAtX(x)).y;
    }
}

void BezierCurve::setHeightCacheResolution(Number resolution) {
    heightCacheResolution = resolution;
    distancesDirty = true;
}

Number BezierCurve::getTValueAtX(Number x) {
    if(controlPoints.size() < 2) {
		return 0;
    }
    
	if (distancesDirty)  {
        recalculateDistances();
    }    
    
    if(x <= minX) {
        return 0.0;
    }
    if(x >= maxX) {
        return 1.0;
    }
    
    Number _x = midX;
    
    Number lower = 0.0;
    Number upper = 1.0;
    Number percent = (upper + lower) / 2.0;
    
    while(fabs(x - _x) > evaluationAccuracy) {
        if(x  > _x) {
            lower = percent;
        } else {
            upper = percent;
        }
        
        percent = (upper + lower) / 2.0;
        _x = getPointAt(percent).x;
    }
    
    return percent;
}

BezierPoint *BezierCurve::getControlPoint(unsigned int index) {
	return controlPoints[index];
} 

unsigned int BezierCurve::getNumControlPoints() {
	return controlPoints.size();
}

void BezierCurve::removePoint(BezierPoint *point) {
	for(int i=0; i < controlPoints.size(); i++) {
		if(controlPoints[i] == point) {
			delete point;
			controlPoints.erase(controlPoints.begin() + i);
			break;
		}
	}
    distancesDirty =  true;
}

Vector3 BezierCurve::getPointAt(Number a) {
    
	if (distancesDirty)  {
        recalculateDistances();
    }
    
	if(controlPoints.size() == 0) {
		return Vector3();
	}
	
	if(controlPoints.size() == 1) {
		return controlPoints[0]->p2;
	}
	
	if(a < 0)
		a = 0;
	if(a > 1)
		a = 1;		
		
	if(controlPoints.size() < 2)
		return Vector3(0,0,0);
	
	Vector3 retVector;
	for(int i=0; i < controlPoints.size()-1; i++) {
		if(a >= distances[i] && a <= distances[i+1]) {
			retVector = getPointBetween(1.0f-((a-distances[i])/(distances[i+1]-distances[i])), controlPoints[i], controlPoints[i+1]);
		}
	}
	return retVector;
}
